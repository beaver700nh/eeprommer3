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

Gui::MenuSdFileSel::MenuSdFileSel(uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols) :
  MenuChoice(pad_v, pad_h, marg_v, marg_h, calc_num_cols(cols), calc_btn_height(rows, marg_v, pad_v), true) {
  m_num_rows = rows;  // NOLINT(cppcoreguidelines-prefer-member-initializer): init list taken by delegated ctor

  for (uint8_t j = 0; j < m_num_rows; ++j) {
    for (uint8_t i = 0; i < m_num_cols; ++i) {
      add_btn_calc(Strings::L_EMPTY_STR, TftColor::BLACK, TftColor::WHITE)->set_font_size(1)->ram_label(true);
    }
  }

  const uint16_t _w = TftCalc::fraction_x(tft, 10, 1);
  const uint16_t _y = TftCalc::bottom(tft, 24, 44);

  add_btn(new Gui::Btn(10, _y, _w, 24, Strings::L_CANCEL, TftColor::PINKK, TftColor::RED));
  add_btn_confirm(true);

  init_files();
}

void Gui::MenuSdFileSel::init_files() {
  const uint8_t max_files = m_num_cols * m_num_rows;

  m_files = (SdFileInfo *) malloc(max_files * sizeof(SdFileInfo));

  if (!SD.exists(Strings::M_FILE_DIR)) {
    SD.mkdir(Strings::M_FILE_DIR);
  }

  m_num_files = sd.get_files(Strings::M_FILE_DIR, m_files, max_files);

  uint8_t i = 0;

  // Update and enable all the needed buttons.
  for (/* no init clause */; i < m_num_files; ++i) {
    get_btn(i)->visibility(true)->set_text(m_files[i].name);

    if (m_files[i].is_dir) {
      get_btn(i)->set_fg(TftColor::LGRAY)->set_bg(TftColor::DGRAY)->operation(false);
    }
  }

  // Disable everything else except the control buttons.
  for (/* no init clause */; i < get_num_btns() - 2; ++i) {
    get_btn(i)->visibility(false)->operation(false);
  }
}

Gui::MenuSdFileSel::~MenuSdFileSel() {
  free(m_files);
}

Gui::MenuSdFileSel::Status Gui::MenuSdFileSel::wait_for_value(char *file_path, uint8_t max_path_len) {
  if (max_path_len <= 2) return Status::FNAME_TOO_LONG;

  if (m_num_files == 0) {
    select(m_num_btns - 2);  // Cancel button
    tft.drawText_P(10, 50, Strings::L_NO_FILES, TftColor::PINKK);
  }

  uint8_t btn_id = MenuChoice::wait_for_value();

  if (btn_id == get_num_btns() - 2) {
    return Status::CANCELED;
  }

  if (strlen(Strings::M_FILE_DIR) + strlen(m_files[btn_id].name) >= max_path_len) {
    return Status::FNAME_TOO_LONG;
  }

  snprintf_P(file_path, max_path_len, PSTR("%s%s"), Strings::M_FILE_DIR, m_files[btn_id].name);
  return Status::OK;
}

FileCtrl *Dialog::ask_file(const char *prompt, uint8_t access, AskFileStatus *status, bool must_exist) {
  SER_DEBUG_PRINT(must_exist, 'd');
  FileSystem fsys = ask_fsys(Strings::P_FILE_TYPE);
  tft.fillScreen(TftColor::BLACK);

  char fpath[64];
  Memory::print_ram_analysis();

  switch (fsys) {
  case FileSystem::NONE:
    *status = AskFileStatus::CANCELED;
    Dialog::show_error(ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
    return nullptr;

  case FileSystem::ON_SD_CARD:
    SER_LOG_PRINT("Getting file path...\n");
    *status = ask_file_sd(prompt, fpath, ARR_LEN(fpath), must_exist);
    return FileCtrl::create_file(fsys, fpath, access);

  default:
    *status = AskFileStatus::FSYS_INVALID;
    Dialog::show_error(ErrorLevel::ERROR, 0x1, Strings::E_INV_FSYS, STRFMT_P_NOBUF(PSTR("No such filesystem: %d."), (uint8_t) fsys));
    return nullptr;
  }
}

Dialog::AskFileStatus Dialog::ask_file_sd(const char *prompt, char *out, uint8_t len, bool must_exist) {
  SER_DEBUG_PRINT(must_exist, 'd');
  using FSStatus = Gui::MenuSdFileSel::Status;

  Memory::print_ram_analysis();
  SER_LOG_PRINT("A\n");

  FSStatus substatus;
  SER_LOG_PRINT("B\n");

  if (must_exist) {
    SER_LOG_PRINT("must_exist == true\n");
    substatus = ask_sel_file_sd(prompt, out, len);
  }
  else {
    SER_LOG_PRINT("must_exist == false\n");
    ask_str(prompt, out, len);
    substatus = FSStatus::OK;
  }

  tft.fillScreen(TftColor::BLACK);

  if (substatus == FSStatus::CANCELED) {
    Dialog::show_error(ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
  }
  else if (substatus == FSStatus::FNAME_TOO_LONG) {
    Dialog::show_error(ErrorLevel::ERROR, 0x3, Strings::T_TOO_LONG, Strings::E_TOO_LONG);
  }

  return (AskFileStatus) substatus;
}

Gui::MenuSdFileSel::Status Dialog::ask_sel_file_sd(const char *prompt, char *out, uint8_t len) {
  SER_LOG_PRINT("GOT HERE\n");
  constexpr uint8_t rows = 6, cols = 6;

  Memory::print_ram_analysis();
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);
  Memory::print_ram_analysis();

  Gui::MenuSdFileSel menu(10, 10, 50, 10, rows, cols);
  Memory::print_ram_analysis();

  return menu.wait_for_value(out, len);
}

FileSystem Dialog::ask_fsys(const char *prompt) {
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuChoice menu(10, 10, 50, 10, 1, 40, true, 0);
  menu.add_btn_calc(Strings::L_FILE_SD,  TftColor::LGREEN, TftColor::DGREEN);
  menu.add_btn_calc(Strings::L_FILE_SER, TftColor::CYAN,   TftColor::BLUE  );
  menu.add_btn_calc(Strings::L_CANCEL,   TftColor::PINKK,  TftColor::DRED  );
  menu.add_btn_confirm(true);

  const uint8_t avail = get_available_file_systems();

  if (~avail & FileSystem::ON_SD_CARD) menu.get_btn(0)->operation(false);
  if (~avail & FileSystem::ON_SERIAL)  menu.get_btn(1)->operation(false);

  const uint8_t btn_pressed = menu.wait_for_value();
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

uint8_t get_available_file_systems() {
  uint8_t avail = FileSystem::NONE;

  // TODO: implement serial files

  if (sd.is_enabled()) {
    avail |= FileSystem::ON_SD_CARD;
  }

  return avail;
}
