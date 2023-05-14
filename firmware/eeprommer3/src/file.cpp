#include <Arduino.h>
#include "constants.hpp"

#include "comm.hpp"
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

namespace Gui {

MenuSdFileSel::MenuSdFileSel(uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols) :
  MenuChoice(pad_v, pad_h, marg_v, marg_h, calc_num_cols(cols), calc_btn_height(rows, marg_v, pad_v), true) {
  m_num_rows = rows;  // NOLINT(cppcoreguidelines-prefer-member-initializer): init list taken by delegated ctor

  for (uint8_t j = 0; j < m_num_rows; ++j) {
    for (uint8_t i = 0; i < m_num_cols; ++i) {
      add_btn_calc(Strings::L_EMPTY_STR, TftColor::BLACK, TftColor::WHITE)->set_font_size(1)->ram_label(true);
    }
  }

  const uint16_t _w = TftCalc::fraction_x(tft, 10, 1);
  const uint16_t _y = TftCalc::bottom(tft, 24, 44);

  add_btn(new Btn(10, _y, _w, 24, Strings::L_CANCEL, TftColor::PINKK, TftColor::RED));
  add_btn_confirm(true);

  init_files();
}

void MenuSdFileSel::init_files() {
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

MenuSdFileSel::~MenuSdFileSel() {
  free(m_files);
}

MenuSdFileSel::Status MenuSdFileSel::wait_for_value(char *file_path, uint8_t max_path_len) {
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

};

namespace Dialog {

FileCtrl *ask_file(const char *prompt, uint8_t access, AskFileStatus *status, bool must_exist) {
  FileSystem fsys = ask_fsys(Strings::P_FILE_TYPE);
  tft.fillScreen(TftColor::BLACK);

  char fpath[64];
  Memory::print_ram_analysis();

  SER_LOG_PRINT("Filesystem: %d\n", fsys);

  switch (fsys) {
  case FileSystem::NONE:
    *status = AskFileStatus::CANCELED;
    show_error(ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
    return nullptr;

  case FileSystem::ON_SD_CARD:
    *status = ask_file_sd(prompt, fpath, ARR_LEN(fpath), must_exist);
    return FileCtrl::create_file(fsys, fpath, access);

  case FileSystem::ON_SERIAL:
    *status = ask_file_serial(prompt, must_exist);
    return FileCtrl::create_file(fsys, "", access);

  default:
    *status = AskFileStatus::FSYS_INVALID;
    show_error(ErrorLevel::ERROR, 0x1, Strings::T_INV_FSYS, STRFMT_P_NOBUF(Strings::E_INV_FSYS, (uint8_t) fsys));
    return nullptr;
  }
}

AskFileStatus ask_file_sd(const char *prompt, char *out, uint8_t len, bool must_exist) {
  using FSStatus = Gui::MenuSdFileSel::Status;

  Memory::print_ram_analysis();

  FSStatus substatus;

  if (must_exist) {
    substatus = ask_sel_file_sd(prompt, out, len);
  }
  else {
    ask_str(prompt, out, len);
    substatus = FSStatus::OK;
  }

  tft.fillScreen(TftColor::BLACK);

  if (substatus == FSStatus::CANCELED) {
    show_error(ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
  }
  else if (substatus == FSStatus::FNAME_TOO_LONG) {
    show_error(ErrorLevel::ERROR, 0x3, Strings::T_TOO_LONG, Strings::E_TOO_LONG);
  }

  return (AskFileStatus) substatus;
}

Gui::MenuSdFileSel::Status ask_sel_file_sd(const char *prompt, char *out, uint8_t len) {
  constexpr uint8_t rows = 6, cols = 6;

  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuSdFileSel menu(10, 10, 50, 10, rows, cols);

  return menu.wait_for_value(out, len);
}

AskFileStatus ask_file_serial(const char *prompt, bool must_exist) {
  Comm::Packet pkt = {0x01, {PKT_FILEOPEN, must_exist}};
  Comm::send(&pkt);

  Comm::Packet::copy_str_P(&pkt, prompt);
  Comm::send(&pkt);

  show_error(ErrorLevel::INFO, 0x3, Strings::W_SOFTWARE, Strings::L_SOFTWARE);

  return AskFileStatus::OK;
}

FileSystem ask_fsys(const char *prompt) {
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

};

FileCtrl *FileCtrl::create_file(FileSystem fsys, const char *path, uint8_t access) {
  switch (fsys) {
  case FileSystem::ON_SD_CARD: return new FileCtrlSd(path, access);
  case FileSystem::ON_SERIAL:  return new FileCtrlSerial(path, access);
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
  fsys = FileSystem::ON_SD_CARD;
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

bool FileCtrlSd::seek(uint16_t position) {
  return m_file.seek(position);
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

FileCtrlSerial::FileCtrlSerial(const char *path, uint8_t access) {
  UNUSED_VAR(path);

  Comm::Packet pkt = {0x01, {PKT_FILECONF, access}};
  Comm::send(&pkt);

  fsys = FileSystem::ON_SERIAL;
}

FileCtrlSerial::~FileCtrlSerial() {
  // Empty
}

bool FileCtrlSerial::is_open() {
  return true;
}

const char *FileCtrlSerial::name() {
  return "<serial-file>";
}

uint16_t FileCtrlSerial::size() {
  Comm::Packet pkt = {0x00, {PKT_FILESIZE}};
  Comm::send(&pkt);
  Comm::recv(&pkt);

  if (pkt.buffer[0] != PKT_FILESIZE) {
    return 0; // Recieved the wrong packet, error
  }

  tft.drawText(0, 150, STRFMT_NOBUF("buf %d %d", pkt.buffer[1], pkt.buffer[2]), TftColor::WHITE, 1);

  return pkt.buffer[1] | (pkt.buffer[2] << 8);
}

bool FileCtrlSerial::seek(uint16_t position) {
  Comm::Packet pkt = {0x02, {PKT_FILESEEK, position & 0xFF, position >> 8}};
  Comm::send(&pkt);

  return true;
}

uint8_t FileCtrlSerial::read() {
  Comm::Packet pkt = {0x00, {PKT_FILEREAD}};
  Comm::send(&pkt);
  Comm::recv(&pkt);

  if (pkt.buffer[0] != PKT_FILEREAD) {
    return 0; // Recieved the wrong packet, error
  }

  return pkt.buffer[1];
}

uint16_t FileCtrlSerial::read(uint8_t *buf, uint16_t size) {
  //
}

void FileCtrlSerial::write(uint8_t val) {
  Comm::Packet pkt = {0x01, {PKT_FILEWRIT, val}};
  Comm::send(&pkt);
}

uint16_t FileCtrlSerial::write(const uint8_t *buf, uint16_t size) {
  //
}

void FileCtrlSerial::flush() {
  Comm::Packet pkt = {0x00, {PKT_FILEFLUS}};
  Comm::send(&pkt);
}

void FileCtrlSerial::close() {
  Comm::Packet pkt = {0x00, {PKT_FILECLOS}};
  Comm::send(&pkt);
}

uint8_t get_available_file_systems() {
  uint8_t avail = FileSystem::NONE;

  if (Comm::ping()) {
    avail |= FileSystem::ON_SERIAL;
  }

  if (sd.is_enabled()) {
    avail |= FileSystem::ON_SD_CARD;
  }

  return avail;
}
