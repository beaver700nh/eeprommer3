#include <Arduino.h>
#include "constants.hpp"

#include "dialog.hpp"
#include "error.hpp"
#include "gui.hpp"
#include "new_delete.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "util.hpp"

#include "file.hpp"

Gui::MenuSdFileSel::MenuSdFileSel(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols) :
  MenuChoice(pad_v, pad_h, marg_v, marg_h, calc_num_cols(tft, cols), calc_btn_height(tft, rows, marg_v, pad_v), true) {
  m_num_rows = rows;  // NOLINT(cppcoreguidelines-prefer-member-initializer): init list taken by delegated ctor

  for (uint8_t j = 0; j < m_num_rows; ++j) {
    for (uint8_t i = 0; i < m_num_cols; ++i) {
      add_btn_calc(tft, Strings::L_EMPTY_STR, TftColor::BLACK, TftColor::WHITE)->set_font_size(1);
    }
  }

  const uint16_t _w = TftCalc::fraction_x(tft, 10, 2);
  const uint16_t _x = _w + 20;
  const uint16_t _y = TftCalc::bottom(tft, 24, 44);

  add_btn(new Gui::Btn(10, _y, _w, 24, Strings::L_GO_UP_DIR, TftColor::CYAN,  TftColor::BLUE));
  add_btn(new Gui::Btn(_x, _y, _w, 24, Strings::L_CANCEL,    TftColor::PINKK, TftColor::RED ));
  add_btn_confirm(tft, true);

  m_files = (SdFileInfo *) malloc(m_num_rows * m_num_cols * sizeof(SdFileInfo));
}

Gui::MenuSdFileSel::~MenuSdFileSel() {
  free(m_files);
}

void Gui::MenuSdFileSel::use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files) {
  uint8_t i;

  m_num_files = sd.get_files(path, m_files, max_files);

  // Update and enable all the needed buttons.
  for (i = 0; i < m_num_files; ++i) {
    uint16_t bg = (m_files[i].is_dir ? TftColor::CYAN : TftColor::ORANGE);

    get_btn(i)->visibility(true)->operation(true)->set_text(m_files[i].name)->set_bg(bg);
  }

  // Disable everything else except the control buttons.
  for (/* no init clause */; i < get_num_btns() - 3; ++i) {
    get_btn(i)->visibility(false)->operation(false);
  }
}

Gui::MenuSdFileSel::Status Gui::MenuSdFileSel::wait_for_value(TouchCtrl &tch, TftCtrl &tft, SdCtrl &sd, char *file_path, uint8_t max_path_len) {
  if (max_path_len <= 2) return Status::FNAME_TOO_LONG;

  strcpy(file_path, "/");

  while (true) {
    tft.fillRect(0, m_marg_v - 3, tft.width(), tft.height() - m_marg_v + 6, TftColor::BLACK);

    deselect_all();
    select(0);

    use_files_in_dir(sd, file_path, m_num_rows * m_num_cols);
    uint8_t btn_id = MenuChoice::wait_for_value(tch, tft);

    if (btn_id == get_num_btns() - 2) {
      return Status::CANCELED;
    }

    if (btn_id == get_num_btns() - 3) {
      FileUtil::go_up_dir(file_path);
      continue;
    }

    if (strlen(file_path) + strlen(m_files[btn_id].name) >= max_path_len) return Status::FNAME_TOO_LONG;

    // User selected a file, not a control button

    if (!FileUtil::go_down_path(file_path, m_files + btn_id, max_path_len)) {
      return Status::FNAME_TOO_LONG;
    }

    // If the file was a regular file, user has selected the needed file, done
    if (!m_files[btn_id].is_dir) {
      return Status::OK;
    }
  }
}

FileCtrl *Dialog::ask_file(TftCtrl &tft, TouchCtrl &tch, const char *prompt, uint8_t access, AskFileStatus *status, bool must_exist, SdCtrl &sd) {
  SER_DEBUG_PRINT(must_exist, 'd');
  FileSystem fsys = ask_fsys(tft, tch, Strings::P_FILE_TYPE, sd);
  tft.fillScreen(TftColor::BLACK);

  char fpath[64];
  Memory::print_ram_analysis();

  switch (fsys) {
  case FileSystem::NONE:
    *status = AskFileStatus::CANCELED;
    Dialog::show_error(tft, tch, ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
    return nullptr;

  case FileSystem::ON_SD_CARD:
    SER_LOG_PRINT("Going into ask_fpath_sd()...\n");
    *status = ask_fpath_sd(tft, tch, prompt, fpath, ARR_LEN(fpath), must_exist, sd);
    return FileCtrl::create_file(fsys, fpath, access);

  default:
    *status = AskFileStatus::FSYS_INVALID;
    Dialog::show_error(tft, tch, ErrorLevel::ERROR, 0x1, Strings::E_INV_FSYS, STRFMT_NOBUF("No such filesystem: %d.", (uint8_t) fsys));
    return nullptr;
  }
}

Dialog::AskFileStatus Dialog::ask_fpath_sd(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *out, uint8_t len, bool must_exist, SdCtrl &sd) {
  SER_DEBUG_PRINT(must_exist, 'd');
  using FSStatus = Gui::MenuSdFileSel::Status;

  Memory::print_ram_analysis();
  SER_LOG_PRINT("A\n");

  FSStatus substatus;
  SER_LOG_PRINT("B\n");

  if (must_exist) {
    SER_LOG_PRINT("must_exist == true\n");
    substatus = ask_sel_fpath_sd(tft, tch, prompt, out, len, sd);
  }
  else {
    SER_LOG_PRINT("must_exist == false\n");
    ask_str(tft, tch, prompt, out, len);
    substatus = FSStatus::OK;
  }

  tft.fillScreen(TftColor::BLACK);

  if (substatus == FSStatus::CANCELED) {
    Dialog::show_error(tft, tch, ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
  }
  else if (substatus == FSStatus::FNAME_TOO_LONG) {
    Dialog::show_error(tft, tch, ErrorLevel::ERROR, 0x3, Strings::T_TOO_LONG, Strings::E_TOO_LONG);
  }

  return (AskFileStatus) substatus;
}

Gui::MenuSdFileSel::Status Dialog::ask_sel_fpath_sd(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *out, uint8_t len, SdCtrl &sd) {
  SER_LOG_PRINT("GOT HERE\n");
  constexpr uint8_t rows = 6, cols = 6;

  Memory::print_ram_analysis();
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);
  Memory::print_ram_analysis();

  Gui::MenuSdFileSel menu(tft, 10, 10, 50, 10, rows, cols);
  Memory::print_ram_analysis();

  return menu.wait_for_value(tch, tft, sd, out, len);
}

FileSystem Dialog::ask_fsys(TftCtrl &tft, TouchCtrl &tch, const char *prompt, SdCtrl &sd) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuChoice menu(10, 10, 50, 10, 1, 40, true, 0);
  menu.add_btn_calc(tft, Strings::L_FILE_SD,  TftColor::LGREEN, TftColor::DGREEN);
  menu.add_btn_calc(tft, Strings::L_FILE_SER, TftColor::CYAN,   TftColor::BLUE  );
  menu.add_btn_calc(tft, Strings::L_CANCEL,   TftColor::PINKK,  TftColor::DRED  );
  menu.add_btn_confirm(tft, true);

  const uint8_t avail = FileUtil::get_available_file_systems(sd);

  if (~avail & FileSystem::ON_SD_CARD) menu.get_btn(0)->operation(false);
  if (~avail & FileSystem::ON_SERIAL)  menu.get_btn(1)->operation(false);

  const uint8_t btn_pressed = menu.wait_for_value(tch, tft);
  const uint8_t btn_cancel  = menu.get_num_btns() - 2;

  if (btn_pressed != btn_cancel) {
    switch (btn_pressed) {
    case 0: return FileSystem::ON_SD_CARD;
    case 1: return FileSystem::ON_SERIAL;
    }
  }

  return FileSystem::NONE;
}

FileCtrl *FileCtrl::create_file(FileSystem fsys, const char *path, uint8_t access) {
  switch (fsys) {
  case FileSystem::ON_SD_CARD: return new FileCtrlSd(path, access);
  case FileSystem::NONE:
  default:
    return nullptr;
  }
}

bool FileCtrl::check_valid(FileCtrl *file) {
  return (file != nullptr) && (file->is_open());
}

FileCtrlSd::FileCtrlSd(const char *path, uint8_t access) {
  m_file = SD.open(path, access);
  fsys   = FileSystem::ON_SD_CARD;
}

FileCtrlSd::~FileCtrlSd() {
  m_file.close();
}

bool FileCtrlSd::is_open() {
  return m_file.operator bool();
}

const char *FileCtrlSd::name() {
  return m_file.name();
}

uint16_t FileCtrlSd::size() {
  return m_file.size();
}

uint8_t FileCtrlSd::read() {
  return m_file.read();
}

uint16_t FileCtrlSd::read(uint8_t *buf, uint16_t size) {
  return m_file.read(buf, size);
}

void FileCtrlSd::write(uint8_t val) {
  m_file.write(val);
}

uint16_t FileCtrlSd::write(const uint8_t *buf, uint16_t size) {
  return m_file.write(buf, size);
}

void FileCtrlSd::flush() {
  m_file.flush();
}

void FileCtrlSd::close() {
  m_file.close();
}

bool FileUtil::go_up_dir(char *path) {
  // Empty path is treated as root dir
  if (strlen(path) == 0) return false;

  bool success = true;

  char *_path = strdup(path);
  char *end   = _path + strlen(_path);

  // Remove trailing '/' from copy of `path`
  *(end - 1) = '\0';

  char *new_end = strrchr(_path, '/');

  if (new_end == nullptr) {
    success = false;
  }
  else {
    memset(new_end + 1, '\0', end - new_end - 2);
    strcpy(path, _path);
  }

  free(_path);
  return success;
}

bool FileUtil::go_down_file(char *path, const char *file, uint8_t len) {
  if (strlen(path) + strlen(file) >= len) {
    // Doesn't fit, fail
    return false;
  }

  strcat(path, file);
  return true;
}

bool FileUtil::go_down_dir(char *path, const char *dir, uint8_t len) {
  // Paste a trailing slash onto `dir`
  auto *temp = (char *) malloc((strlen(dir) + 2) * sizeof(char));
  strcpy(temp, dir);
  strcat(temp, "/");

  // Delegates to go_down_file but passes `dir` with a trailing slash
  bool result = go_down_file(path, temp, len);

  free(temp);

  return result;
}

bool FileUtil::go_down_path(char *path, SdFileInfo *sub_path, uint8_t len) {
  // Delegate to go_down_file() or go_down_dir()
  return (sub_path->is_dir ? go_down_dir : go_down_file)(path, sub_path->name, len);
}

uint8_t FileUtil::get_available_file_systems(SdCtrl &sd) {
  uint8_t avail = FileSystem::NONE;

  // TODO: implement serial files

  if (sd.is_enabled()) {
    avail |= FileSystem::ON_SD_CARD;
  }

  return avail;
}
