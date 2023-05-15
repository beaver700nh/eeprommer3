#include <Arduino.h>
#include "constants.hpp"

#include "dialog.hpp"
#include "eeprom.hpp"
#include "error.hpp"
#include "file.hpp"
#include "new_delete.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"
#include "vector.hpp"
#include "xram.hpp"

#include "prog_core.hpp"

#define RETURN_VERIFICATION_OR_VALUE(value, ...)             \
  bool should_verify = Dialog::ask_yesno(Strings::P_VERIFY); \
  tft.fillScreen(TftColor::BLACK);                           \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

using Status = ProgrammerBaseCore::Status;

extern TftCtrl tft;
extern TouchCtrl tch;
extern EepromCtrl ee;
extern SdCtrl sd;

// Dummy function for unimplemented actions
Status ProgrammerBaseCore::nop() {
  return Status::OK;
}

/***************************/
/******** BYTE CORE ********/
/***************************/

Status ProgrammerByteCore::read() {
  uint16_t addr = Dialog::ask_addr(Strings::P_ADDR_GEN);
  uint8_t data  = ee.read(addr);

  tft.fillScreen(TftColor::BLACK);

  char title[32];
  snprintf_P_sz(title, Strings::T_VALUE_AT, addr);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x0, title,
    STRFMT_P_NOBUF(Strings::G_REPR_8, BYTE_FMT_VAL(data), data, data, data, (data == '\0' ? ' ' : data))
  );

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerByteCore::write() {
  uint16_t addr = Dialog::ask_addr(Strings::P_ADDR_GEN);
  tft.fillScreen(TftColor::BLACK);
  uint8_t data = Dialog::ask_int<uint8_t>(Strings::P_DATA_GEN);

  ee.write(addr, data);

  tft.fillScreen(TftColor::BLACK);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x1, Strings::T_DONE,
    STRFMT_P_NOBUF(Strings::G_W_BYTE, data, addr)
  );

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(addr, (void *) &data);
}

Status ProgrammerByteCore::verify(uint16_t addr, void *data) {
  uint8_t actual = ee.read(addr);

  if (actual != *(uint8_t *) data) {
    Dialog::wait_error(
      ErrorLevel::INFO, 0x1, Strings::T_MSMCH,
      STRFMT_NOBUF(Strings::G_VERIFY_8, *(uint8_t *) data, actual)
    );

    tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/***************************/
/******** FILE CORE ********/
/***************************/

Status ProgrammerFileCore::read() {
  using AFStatus = Dialog::AskFileStatus;

  AFStatus fstatus;
  FileCtrl *file = Dialog::ask_file(Strings::P_OFILE, O_CREAT | O_TRUNC | O_WRITE, &fstatus, false);

  tft.fillScreen(TftColor::BLACK);

  if (fstatus != AFStatus::OK) return (fstatus == AFStatus::CANCELED ? Status::OK : Status::ERR_FILE);

  if (!FileCtrl::check_valid(file)) {
    delete file;
    return Status::ERR_FILE;
  }

  read_operation_core(file);
  file->flush();
  file->close();
  delete file;

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

void ProgrammerFileCore::read_operation_core(FileCtrl *file) {
  uint8_t *buffer = (uint8_t *) xram::access(XRAM_8K_BUF);

  tft.drawText_P(10, 10, Strings::W_OFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(4, 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bar.for_each(
    [&buffer, &file] GUI_PROGRESS_INDICATOR_LAMBDA {
      uint16_t addr = progress << 13;

      ee.read(addr, addr + 0x1FFF, buffer);
      file->write(buffer, 0x2000);

      return tch.is_touching();
    }
  );

  tft.drawText_P(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue();
}

Status ProgrammerFileCore::write() {
  using AFStatus = Dialog::AskFileStatus;

  AFStatus fstatus;

  Memory::print_ram_analysis();
  FileCtrl *file = Dialog::ask_file(Strings::P_IFILE, O_RDONLY, &fstatus, true);
  Memory::print_ram_analysis();

  tft.fillScreen(TftColor::BLACK);

  if (fstatus != AFStatus::OK) {
    delete file;
    return (fstatus == AFStatus::CANCELED ? Status::OK : Status::ERR_FILE);
  }

  if (!FileCtrl::check_valid(file)) {
    delete file;
    return Status::ERR_FILE;
  }

  uint16_t addr = Dialog::ask_addr(Strings::P_ADDR_FILE);
  tft.fillScreen(TftColor::BLACK);

  bool success = write_from_file(file, addr);
  tft.fillScreen(TftColor::BLACK);

  if (!success) {
    delete file;
    return Status::ERR_INVALID;
  }

  bool should_verify = Dialog::ask_yesno(Strings::P_VERIFY);
  tft.fillScreen(TftColor::BLACK);

  Status status = Status::OK;
  if (should_verify) status = verify(addr, file);

  file->close();
  delete file;

  return status;
}

bool ProgrammerFileCore::write_from_file(FileCtrl *file, uint16_t addr) {
  if (file->size() > (0x7FFF - addr + 1)) {
    Dialog::wait_error(ErrorLevel::WARNING, 0x3, Strings::T_TOO_BIG, Strings::E_TOO_BIG);
    return false;
  }

  write_operation_core(file, addr);
  return true;
}

void ProgrammerFileCore::write_operation_core(FileCtrl *file, uint16_t addr) {
  uint16_t cur_addr = addr;
  uint8_t *buffer = (uint8_t *) xram::access(XRAM_8K_BUF);

  tft.drawText_P(10, 10, Strings::W_IFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(ceil((float) file->size() / 0x2000), 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bar.for_each(
    [&buffer, &file, &cur_addr] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      uint16_t len = file->read(buffer, 0x2000);

      if (len == 0) {
        return false;  // Nothing to write
      }

      ee.write(cur_addr, buffer, len);

      cur_addr += 0x2000;  // Next page

      return tch.is_touching();
    }
  );

  tft.drawText_P(10, 110, Strings::F_WRITE, TftColor::CYAN);
  TftUtil::wait_continue();
}

Status ProgrammerFileCore::verify(uint16_t addr, void *data) {
  auto *file = (FileCtrl *) data;
  file->seek(0);

  uint8_t *expected = (uint8_t *) xram::access(XRAM_8K_BUF + 0x0000);
  uint8_t *reality  = (uint8_t *) xram::access(XRAM_8K_BUF + 0x1000);

  tft.drawText(10, 10, STRFMT_P_NOBUF(Strings::W_VERIFY, file->name(), addr), TftColor::CYAN);

  Gui::ProgressIndicator bar(ceil((float) file->size() / 0x1000), 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bool complete = bar.for_each(
    [&expected, &reality, &file, &addr] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      uint16_t nbytes = file->read(expected, 0x1000);

      if (nbytes == 0) {
        return false;  // Nothing to check
      }

      ee.read(addr, (addr + nbytes) - 1, reality);

      if (memcmp(expected, reality, nbytes) != 0) {
        tft.drawText(10, 150, STRFMT_P_NOBUF(Strings::E_MISMATCH, addr, addr + 0x0FFF), TftColor::RED);
        return true;  // Request to quit loop
      }

      addr += 0x1000;  // Next sector
      return false;
    }
  );

  tft.drawText_P(10, 110, Strings::F_VERIFY, TftColor::CYAN);
  TftUtil::wait_continue();

  file->close();
  tft.fillScreen(TftColor::BLACK);
  return (complete ? Status::OK : Status::ERR_VERIFY);
}

/*****************************/
/******** VECTOR CORE ********/
/*****************************/

Status ProgrammerVectorCore::read() {
  Vector vec = Dialog::ask_vector();
  vec.update(ee);

  tft.fillScreen(TftColor::BLACK);

  const char *const name = Util::strdup_P(Vector::NAMES[vec.m_id]);

  char title[32];
  snprintf_P_sz(title, Strings::T_VALUE_OF, name, vec.m_addr, vec.m_addr + 1);

  free((void *) name);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x0, title,
    STRFMT_P_NOBUF(Strings::G_REPR_16, BYTE_FMT_VAL(vec.m_hi), BYTE_FMT_VAL(vec.m_lo), vec.m_val, vec.m_val, vec.m_val)
  );

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerVectorCore::write() {
  Vector vec = Dialog::ask_vector();
  vec.update(ee);

  tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = Dialog::ask_int<uint16_t>(Strings::P_ADDR_VEC);
  ee.write(vec.m_addr + 0, new_val & 0xFF);
  ee.write(vec.m_addr + 1, new_val >> 8);

  tft.fillScreen(TftColor::BLACK);

  const char *const name = Util::strdup_P(Vector::NAMES[vec.m_id]);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x1, Strings::T_DONE,
    STRFMT_P_NOBUF(Strings::G_W_VECTOR, new_val, name, vec.m_addr, vec.m_addr + 1)
  );

  free((void *) name);

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(vec.m_addr, (void *) &new_val)
}

Status ProgrammerVectorCore::verify(uint16_t addr, void *data) {
  uint16_t actual = (ee.read(addr + 1) << 8) | ee.read(addr);

  if (actual != *(uint16_t *) data) {
    Dialog::wait_error(
      ErrorLevel::ERROR, 0x1, Strings::T_MSMCH,
      STRFMT_P_NOBUF(Strings::G_VERIFY_16, *(uint16_t *) data, actual)
    );

    TftUtil::wait_continue();

    tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/********************************/
/******** MULTIBYTE CORE ********/
/********************************/

Status ProgrammerMultiCore::read() {
  uint16_t addr1 = Dialog::ask_addr(Strings::P_ADDR_BEG);
  tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = Dialog::ask_addr(Strings::P_ADDR_END);

  Util::validate_addrs(&addr1, &addr2);

  uint16_t nbytes = (addr2 - addr1 + 1);

  if (nbytes > 0x2000) {
    tft.fillScreen(TftColor::BLACK);
    return Status::ERR_MEMORY;
  }

  uint8_t *data = (uint8_t *) xram::access(XRAM_8K_BUF);

  read_operation_core(data, addr1, addr2);
  tft.fillScreen(TftColor::BLACK);

  Gui::MenuChoice menu(10, 10, 50, 10, 1, 30, true);

  menu.add_btn_calc(Strings::L_VM_HEX,  TftColor::PINKK,  TftColor::RED   );
  menu.add_btn_calc(Strings::L_VM_CHAR, TftColor::LGREEN, TftColor::DGREEN);
  menu.add_btn_calc(Strings::L_VM_FILE, TftColor::CYAN,   TftColor::BLUE  );
  menu.add_btn_calc(Strings::L_CLOSE,   TftColor::BLACK,  TftColor::WHITE );
  menu.add_btn_confirm(true);

  Status status = Status::OK;

  while (menu.get_choice() != menu.get_num_btns() - 2) {
    status = handle_data(data, addr1, addr2, &menu);
    tft.fillScreen(TftColor::BLACK);
  }

  free(data);

  return status;
}

void ProgrammerMultiCore::read_operation_core(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  tft.fillScreen(TftColor::BLACK);

  tft.drawText_P(10, 10, Strings::W_RMULTI, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(ceil((float) nbytes / 256.0), 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  uint16_t cur_addr_offset = 0x0000;

  bar.for_each(
    [&data, &cur_addr_offset, &addr1, &addr2] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      uint16_t _addr1 = addr1 + cur_addr_offset;
      uint16_t _addr2 = MIN(_addr1 + 0xFF, addr2);

      ee.read(_addr1, _addr2, data + cur_addr_offset);

      cur_addr_offset += 0x0100;  // Next page

      return tch.is_touching();
    }
  );

  tft.drawText_P(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue();
}

Status ProgrammerMultiCore::handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2, Gui::MenuChoice *menu) {
  tft.drawText_P(10, 10, Strings::P_VIEW_METH, TftColor::CYAN, 3);

  uint8_t viewing_method = menu->wait_for_value();

  tft.fillScreen(TftColor::BLACK);

  switch (viewing_method) {
  case 0:
    Gui::PageDisplay(data, addr1, addr2, &Gui::PageDisplay::repr_hex).show_range();
    return Status::OK;
  case 1:
    Gui::PageDisplay(data, addr1, addr2, &Gui::PageDisplay::repr_chars).show_range();
    return Status::OK;
  case 2:
    return store_file(data, addr2 - addr1 + 1);
  case 3:
    return Status::OK;
  default:
    return Status::ERR_INVALID;
  }
}

Status ProgrammerMultiCore::store_file(uint8_t *data, uint16_t len) {
  using AFStatus = Dialog::AskFileStatus;

  AFStatus substatus;
  FileCtrl *file = Dialog::ask_file(Strings::P_STORE, O_WRITE | O_CREAT | O_TRUNC, &substatus, false);

  tft.fillScreen(TftColor::BLACK);

  if (substatus == AFStatus::OK) {
    store_file_operation_core(data, len, file);
    file->close();
  }

  delete file;

  if (substatus == AFStatus::FNAME_TOO_LONG || substatus == AFStatus::FSYS_INVALID) {
    return Status::ERR_FILE;
  }
  else {
    return Status::OK;
  }
}

void ProgrammerMultiCore::store_file_operation_core(uint8_t *data, uint16_t len, FileCtrl *file) {
  tft.drawText_P(10, 10, Strings::W_WAIT, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(ceil((float) len / 256.0), 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bar.for_each(
    [&data, &len, &file] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      tft.fillRect(0, 0, 20, 20, TftColor::RED);
      file->write(data, min(256, len));
      tft.drawText(0, 0, STRFMT_NOBUF("%d", progress), TftColor::WHITE, 1);

      if (len < 0x0100) {
        return false;  // Request to quit loop
      }

      data += 0x0100;
      len -= 0x0100;

      return tch.is_touching();
    }
  );

  file->flush();

  tft.drawText_P(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue();
}

Status ProgrammerMultiCore::write() {
  AddrDataArray buf;

  bool confirmed = Dialog::ask_pairs(Strings::T_WMULTI, &buf);

  tft.fillScreen(TftColor::BLACK);

  if (!confirmed) {
    return Status::OK;
  }

  write_operation_core(&buf);

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(0 /* dummy addr */, (void *) &buf)
}

void ProgrammerMultiCore::write_operation_core(AddrDataArray *buf) {
  tft.fillScreen(TftColor::BLACK);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x1,
    Strings::W_WMULTI, STRFMT_P_NOBUF(Strings::L_W_N_PAIRS, buf->get_len())
  );

  ee.write(buf);

  tft.fillScreen(TftColor::BLACK);

  Dialog::wait_error(
    ErrorLevel::INFO, 0x3,
    Strings::F_WRITE, Strings::L_CONTINUE
  );
}

Status ProgrammerMultiCore::verify(uint16_t addr, void *data) {
  UNUSED_VAR(addr);  // addr is unused because `data` already contains addresses

  auto *buf = (AddrDataArray *) data;

  for (uint16_t i = 0; i < buf->get_len(); ++i) {
    AddrDataArrayPair pair;
    buf->get_pair(i, &pair);

    uint8_t real_data = ee.read(pair.addr);

    if (pair.data != real_data) {
      char title[32];
      snprintf_P_sz(title, Strings::T_MSMCH_AT, pair.addr);

      Dialog::wait_error(
        ErrorLevel::ERROR, 0x0, title,
        STRFMT_P_NOBUF(Strings::G_VERIFY_8, pair.data, real_data)
      );

      tft.fillScreen(TftColor::BLACK);

      return Status::ERR_VERIFY;
    }
  }

  return Status::OK;
}

/***********************************/
/******** OTHER CORE - MISC ********/
/***********************************/

Status ProgrammerOtherCore::paint() {
  tft_draw_test();

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerOtherCore::debug() {
  const auto w1 = TftCalc::fraction_x(tft, 10, 1);
  const auto w2 = TftCalc::fraction_x(tft, 10, 2);
  const auto x2 = w2 + 20;

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(10, 50,  w2, 28, Strings::D_WE_HI,     TftColor::LGREEN, TftColor::DGREEN));
  menu.add_btn(new Gui::Btn(x2, 50,  w2, 28, Strings::D_WE_LO,     TftColor::PINKK,  TftColor::RED   ));
  menu.add_btn(new Gui::Btn(10, 88,  w1, 28, Strings::D_SET_ADDR,  TftColor::BLACK,  TftColor::YELLOW));
  menu.add_btn(new Gui::Btn(10, 126, w2, 28, Strings::D_RD_DATA,   TftColor::BLUE,   TftColor::CYAN  ));
  menu.add_btn(new Gui::Btn(x2, 126, w2, 28, Strings::D_WR_DATA,   TftColor::CYAN,   TftColor::BLUE  ));
  menu.add_btn(new Gui::Btn(10, 164, w2, 28, Strings::D_SET_DDIR,  TftColor::BLACK,  TftColor::ORANGE));
  menu.add_btn(new Gui::Btn(x2, 164, w2, 28, Strings::D_MON_DATA,  TftColor::YELLOW, TftColor::DCYAN ));
  menu.add_btn(new Gui::Btn(10, 202, w2, 28, Strings::D_P_CHARSET, TftColor::PINKK,  TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(x2, 202, w2, 28, Strings::D_SHOW_COL,  TftColor::PINKK,  TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(10, 240, w2, 28, Strings::D_AUX1,      TftColor::DGRAY,  TftColor::LGRAY ));
  menu.add_btn(new Gui::Btn(x2, 240, w2, 28, Strings::D_AUX2,      TftColor::DGRAY,  TftColor::LGRAY ));
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(Strings::L_CLOSE)));

  while (true) {
    tft.drawText_P(10, 10, Strings::T_DEBUGS, TftColor::CYAN, 4);
    menu.draw();

    DebugAction btn = (DebugAction) menu.wait_for_press();

    if (btn == menu.get_num_btns() - 1) break;

    tft.fillScreen(TftColor::BLACK);
    do_debug_action(btn);
    tft.fillScreen(TftColor::BLACK);
  }

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

void ProgrammerOtherCore::do_debug_action(DebugAction action) {
  switch (action) {
  case DebugAction::DISABLE_WRITE:       ee.set_we(true);      return;
  case DebugAction::ENABLE_WRITE:        ee.set_we(false);     return;
  case DebugAction::SET_ADDR_BUS_AND_OE: set_addr_and_oe();    return;
  case DebugAction::READ_DATA_BUS:       read_data_bus();      return;
  case DebugAction::WRITE_DATA_BUS:      write_data_bus();     return;
  case DebugAction::SET_DATA_DIR:        set_data_dir();       return;
  case DebugAction::MONITOR_DATA_BUS:    monitor_data_bus();   return;
  case DebugAction::PRINT_CHARSET:       print_charset_wait(); return;
  case DebugAction::SHOW_COLORS:         tft_show_colors();    return;
  case DebugAction::ACTION_AUX1:         debug_action_aux1();  return;
  case DebugAction::ACTION_AUX2:         debug_action_aux2();  return;
  }
}

void ProgrammerOtherCore::set_addr_and_oe() {
  ee.set_addr_and_oe(
    Dialog::ask_int<uint16_t>(Strings::P_VAL_GEN)
  );
}

void ProgrammerOtherCore::read_data_bus() {
  Dialog::wait_error(
    ErrorLevel::INFO, 0x1, Strings::T_VALUE,
    STRFMT_P_NOBUF(PSTR(BYTE_FMT), BYTE_FMT_VAL(ee.get_data()))
  );
}

void ProgrammerOtherCore::write_data_bus() {
  ee.set_data(
    Dialog::ask_int<uint8_t>(Strings::P_VAL_GEN)
  );
}

void ProgrammerOtherCore::set_data_dir() {
  ee.set_ddr(
    Dialog::ask_choice(
      Strings::P_DATA_DIR, 1, 45, 0, 2,
      Strings::L_INPUT, TftColor::CYAN, TftColor::BLUE,
      Strings::L_OUTPUT, TftColor::PINKK, TftColor::RED
    )
  );
}

void ProgrammerOtherCore::monitor_data_bus() {
  ee.set_ddr(false);

  Gui::Btn close_btn(BOTTOM_BTN(Strings::L_CLOSE));
  close_btn.draw();

#ifdef DEBUG_MODE
  while (!close_btn.is_pressed()) {
    uint8_t val = ee.get_io_exp(true)->read_port(PORT_A);

    tft.drawTextBg(10, 10, STRFMT_P_NOBUF(PSTR(BYTE_FMT), BYTE_FMT_VAL(val)), TftColor::CYAN, TftColor::BLACK, 3);
    delay(500);
  }
#else
  // EepromCtrl::get_io_exp() only exists in DEBUG_MODE

  Dialog::wait_error(ErrorLevel::ERROR, 0x3, Strings::T_NOT_SUPP, Strings::E_NO_DB_MON);

  close_btn.wait_for_press();
#endif
}

void ProgrammerOtherCore::print_charset_wait() {
  tft_print_chars();
  tch.wait_for_press();
}

void ProgrammerOtherCore::debug_action_aux1() {
  uint8_t temp[] {
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  };

  ee.write(0x200, temp, 32);
}

void ProgrammerOtherCore::debug_action_aux2() {
  // Unused
}

Status ProgrammerOtherCore::about() {
  tft.drawText_P( 10,  10, Strings::T_ABOUT,     TftColor::CYAN, 3);
  tft.drawText_P( 10,  50, Strings::L_PROJ_NAME, TftColor::PURPLE);
  tft.drawText_P(142,  50, Strings::I_SUBTITLE,  TftColor::BLUE);
  tft.drawText_P( 10,  90, Strings::I_LINE_1,    TftColor::LGRAY);
  tft.drawText_P( 10, 120, Strings::I_LINE_2,    TftColor::LGRAY);
  tft.drawText_P( 10, 150, Strings::I_LINE_3,    TftColor::LGRAY);
  tft.drawText_P( 10, 180, Strings::I_LINE_4,    TftColor::LGRAY);
  tft.drawText_P( 10, 240, Strings::I_LINE_5,    TftColor::DGRAY);

  TftUtil::wait_bottom_btn(Strings::L_CLOSE);

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerOtherCore::restart() {
  SER_LOG_PRINT("Restarting...\n\n");
  delay(1000);
  Util::restart();

  return Status::OK;
}

#undef RETURN_VERIFICATION_OR_VALUE
#undef RETURN_VERIFICATION_OR_OK
