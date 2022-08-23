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

#include "prog_core.hpp"

#define RETURN_VERIFICATION_OR_VALUE(value, ...)                       \
  bool should_verify = Dialog::ask_yesno(tft, tch, Strings::P_VERIFY); \
  tft.fillScreen(TftColor::BLACK);                                     \
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
  uint16_t addr = Dialog::ask_addr(tft, tch, Strings::P_ADDR_GEN);
  uint8_t data  = ee.read(addr);

  tft.fillScreen(TftColor::BLACK);

  char title[32];
  SNPRINTF(title, "Value at addr %04X", addr);

  Dialog::show_error(
    tft, tch, ErrorLevel::INFO, 0x0, title,
    STRFMT_NOBUF(
      "BIN: " BYTE_FMT "\n"
      "OCT: %03o\n"
      "HEX: %02X\n"
      "DEC: %-3d\n"
      "CHR: %c",
      BYTE_FMT_VAL(data), data, data, data, data
    )
  );

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerByteCore::write() {
  uint16_t addr = Dialog::ask_addr(tft, tch, Strings::P_ADDR_GEN);
  tft.fillScreen(TftColor::BLACK);
  uint8_t data = Dialog::ask_int<uint8_t>(tft, tch, Strings::P_DATA_GEN);

  ee.write(addr, data);

  tft.fillScreen(TftColor::BLACK);

  Dialog::show_error(
    tft, tch, ErrorLevel::INFO, 0x1, Strings::T_DONE,
    STRFMT_NOBUF(
      "Wrote data %02X\n"
      "to address %04X.",
      data, addr
    )
  );

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(addr, (void *) &data);
}

Status ProgrammerByteCore::verify(uint16_t addr, void *data) {
  uint8_t actual = ee.read(addr);

  if (actual != *(uint8_t *) data) {
    Dialog::show_error(
      tft, tch, ErrorLevel::INFO, 0x1, Strings::T_MISMATCH,
      STRFMT_NOBUF(
        "Expected: %02X\n"
        "Actual:   %02X",
        *(uint8_t *) data, actual
      )
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
  FileCtrl *file = Dialog::ask_file(tft, tch, Strings::P_OFILE, O_CREAT | O_TRUNC | O_WRITE, &fstatus, false, sd);

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
  uint8_t this_page[256];

  tft.drawText(10, 10, Strings::W_OFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(tft, 0x80, 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bar.for_each(
    [&this_page, &file] GUI_PROGRESS_INDICATOR_LAMBDA {
      uint16_t addr = progress << 8;

      ee.read(addr, addr + 0xFF, this_page);
      file->write(this_page, 0xFF);

      return tch.is_touching();
    }
  );

  tft.drawText(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue(tft, tch);
}

Status ProgrammerFileCore::write() {
  using AFStatus = Dialog::AskFileStatus;

  AFStatus fstatus;

  Memory::print_ram_analysis();
  FileCtrl *file = Dialog::ask_file(tft, tch, Strings::P_IFILE, O_RDONLY, &fstatus, true, sd);
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

  uint16_t addr = Dialog::ask_addr(tft, tch, Strings::P_ADDR_FILE);
  tft.fillScreen(TftColor::BLACK);

  bool success = write_from_file(file, addr);
  tft.fillScreen(TftColor::BLACK);

  if (!success) {
    delete file;
    return Status::ERR_INVALID;
  }

  bool should_verify = Dialog::ask_yesno(tft, tch, Strings::P_VERIFY);
  tft.fillScreen(TftColor::BLACK);

  Status status = Status::OK;
  if (should_verify) status = verify(addr, file);

  file->close();
  delete file;

  return status;
}

bool ProgrammerFileCore::write_from_file(FileCtrl *file, uint16_t addr) {
  if (file->size() > (0x7FFF - addr + 1)) {
    Dialog::show_error(tft, tch, ErrorLevel::WARNING, 0x3, Strings::T_TOO_BIG, Strings::E_TOO_BIG);
    return false;
  }

  write_operation_core(file, addr);
  return true;
}

void ProgrammerFileCore::write_operation_core(FileCtrl *file, uint16_t addr) {
  uint16_t cur_addr = addr;
  uint8_t this_page[256];

  tft.drawText(10, 10, Strings::W_IFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(tft, ceil((float) file->size() / 256.0), 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bar.for_each(
    [&this_page, &file, &cur_addr] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      auto len = file->read(this_page, 0xFF);
      ee.write(cur_addr, this_page, MIN(len, 0xFF));

      cur_addr += 0x0100;  // Next page

      return tch.is_touching();
    }
  );

  tft.drawText(10, 110, Strings::F_WRITE, TftColor::CYAN);
  TftUtil::wait_continue(tft, tch);
}

Status ProgrammerFileCore::verify(uint16_t addr, void *data) {
  auto *file = (FileCtrl *) data;

  uint8_t expected[256], reality[256];

  tft.drawText(10, 10, STRFMT_NOBUF("Verifying `%s' at %04X...", file->name(), addr), TftColor::CYAN);

  Gui::ProgressIndicator bar(tft, ceil((float) file->size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

  bool complete = bar.for_each(
    [&expected, &reality, &file, &addr] GUI_PROGRESS_INDICATOR_LAMBDA {
      UNUSED_VAR(progress);

      auto nbytes = file->read(expected, 256);
      ee.read(addr, addr + nbytes, reality);

      if (memcmp(expected, reality, nbytes) != 0) {
        tft.drawText(10, 150, STRFMT_NOBUF("Mismatch between %04X and %04X!", addr, addr + 0xFF), TftColor::RED);
        return true;  // Request to quit loop
      }

      addr += 0x0100;  // Next page
      return false;
    }
  );

  tft.drawText(10, 110, Strings::F_VERIFY, TftColor::CYAN);
  TftUtil::wait_continue(tft, tch);

  file->close();
  tft.fillScreen(TftColor::BLACK);
  return (complete ? Status::OK : Status::ERR_VERIFY);
}

/*****************************/
/******** VECTOR CORE ********/
/*****************************/

Status ProgrammerVectorCore::read() {
  Vector vec = Dialog::ask_vector(tft, tch);
  vec.update(ee);

  tft.fillScreen(TftColor::BLACK);

  char title[16];
  SNPRINTF(title, "Value of %s", Vector::NAMES[vec.m_id]);

  Dialog::show_error(
    tft, tch, ErrorLevel::INFO, 0x0, title,
    STRFMT_NOBUF(
      "Addr: %04X-%04X\n"
      "HEX: %04X\n"
      "BIN: " BYTE_FMT "\n"
      ".... " BYTE_FMT,
      vec.m_addr, vec.m_addr + 1, vec.m_val, BYTE_FMT_VAL(vec.m_hi), BYTE_FMT_VAL(vec.m_lo)
    )
  );

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerVectorCore::write() {
  Vector vec = Dialog::ask_vector(tft, tch);
  vec.update(ee);

  tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = Dialog::ask_int<uint16_t>(tft, tch, Strings::P_ADDR_VEC);
  ee.write(vec.m_addr + 0, new_val & 0xFF);
  ee.write(vec.m_addr + 1, new_val >> 8);

  tft.fillScreen(TftColor::BLACK);

  Dialog::show_error(
    tft, tch, ErrorLevel::INFO, 0x1, Strings::T_DONE,
    STRFMT_NOBUF(
      "Wrote value %04X\n"
      "to vector %s\n"
      "at %04X-%04X.",
      new_val, Vector::NAMES[vec.m_id], vec.m_addr, vec.m_addr + 1
    )
  );

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(vec.m_addr, (void *) &new_val)
}

Status ProgrammerVectorCore::verify(uint16_t addr, void *data) {
  uint16_t actual = (ee.read(addr + 1) << 8) | ee.read(addr);

  if (actual != *(uint16_t *) data) {
    Dialog::show_error(
      tft, tch, ErrorLevel::ERROR, 0x1, Strings::T_MISMATCH,
      STRFMT_NOBUF(
        "Expected: %04X\n"
        "Actual:   %04X",
        *(uint16_t *) data, actual
      )
    );

    TftUtil::wait_continue(tft, tch);

    tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/********************************/
/******** MULTIBYTE CORE ********/
/********************************/

Status ProgrammerMultiCore::read() {
  uint16_t addr1 = Dialog::ask_addr(tft, tch, Strings::P_ADDR_BEG);
  tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = Dialog::ask_addr(tft, tch, Strings::P_ADDR_END);

  Util::validate_addrs(&addr1, &addr2);

  uint16_t nbytes = (addr2 - addr1 + 1);

  auto *data = (uint8_t *) malloc(nbytes * sizeof(uint8_t));
  if (data == nullptr) {
    tft.fillScreen(TftColor::BLACK);
    return Status::ERR_MEMORY;
  }

  read_operation_core(data, addr1, addr2);

  tft.fillScreen(TftColor::BLACK);

  Status status = handle_data(data, addr1, addr2);

  tft.fillScreen(TftColor::BLACK);

  free(data);

  return status;
}

void ProgrammerMultiCore::read_operation_core(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  tft.fillScreen(TftColor::BLACK);

  tft.drawText(10, 10, Strings::W_RMULTI, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(tft, ceil((float) nbytes / 256.0) - 1, 10, 50, TftCalc::fraction_x(tft, 10, 1), 40);

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

  tft.drawText(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue(tft, tch);
}

Status ProgrammerMultiCore::handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  uint8_t viewing_method = Dialog::ask_choice(
    tft, tch, Strings::P_VIEW_METH, 1, 30, 0, 3,
    Strings::L_VM_HEX,  TftColor::BLACK,  TftColor::ORANGE,
    Strings::L_VM_CHAR, TftColor::LGREEN, TftColor::DGREEN,
    Strings::L_VM_FILE, TftColor::CYAN,   TftColor::BLUE
  );

  tft.fillScreen(TftColor::BLACK);

  switch (viewing_method) {
  case 0:  return (show_range(data, addr1, addr2, &multi_byte_repr_hex),   Status::OK);
  case 1:  return (show_range(data, addr1, addr2, &multi_byte_repr_chars), Status::OK);
  case 2:  return store_file(data, nbytes);
  default: return Status::ERR_INVALID;
  }
}

void ProgrammerMultiCore::show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr) {
  // Draw frame for the data
  tft.drawText(10, 10, STRFMT_NOBUF("%d bytes", addr2 - addr1 + 1), TftColor::CYAN, 3);
  tft.drawThickRect(tft.width() / 2 - 147, 55, 295, 166, TftColor::WHITE, 2);
  tft.drawFastVLine(tft.width() / 2, 57, 162, TftColor::GRAY);

  draw_page_axis_labels();

  const uint16_t x2 = tft.width() - 55;

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(15, 60, 40, 150, 15, 68, Strings::L_ARROW_L));
  menu.add_btn(new Gui::Btn(x2, 60, 40, 150, 15, 68, Strings::L_ARROW_R));
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(tft, Strings::L_CONTINUE)));
  menu.draw(tft);

  const uint8_t max_page = (addr2 >> 8) - (addr1 >> 8);
  uint8_t cur_page = 0;

  while (true) {
    show_page(data, addr1, addr2, repr, cur_page, max_page);

    switch (menu.wait_for_press(tch, tft)) {
    case 0: cur_page = (cur_page == 0 ? max_page : cur_page - 1); break;  // Left
    case 1: cur_page = (cur_page == max_page ? 0 : cur_page + 1); break;  // Right
    case 2: return;                                                       // Continue
    }
  }
}

void ProgrammerMultiCore::draw_page_axis_labels() {
  for (uint8_t row = 0x00; row < 0x10; ++row) {
    const uint16_t x1 = tft.width() / 2 - 161;
    const uint16_t x2 = tft.width() / 2 + 151;

    const uint16_t y = tft.height() / 2 - 100 + 10 * row;

    tft.drawText(x1, y, STRFMT_NOBUF("%1Xx", row), TftColor::DCYAN, 1);
    tft.drawText(x2, y, STRFMT_NOBUF("%1Xx", row), TftColor::DCYAN, 1);
  }

  for (uint8_t col = 0x00; col < 0x10; ++col) {
    const uint8_t split_offset = (col < 8 ? 0 : 3);
    const uint16_t x = tft.width() / 2 - 141 + 18 * col + split_offset;

    tft.drawText(x, tft.height() / 2 + 64, STRFMT_NOBUF("x%1X", col), TftColor::DCYAN, 1);
  }
}

void ProgrammerMultiCore::show_page(
  uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr, uint8_t cur_page, uint8_t max_page
) {
  tft.fillRect(tft.width() / 2 - 145, 57, 145, 162, TftColor::DGRAY);
  tft.fillRect(tft.width() / 2 +   1, 57, 145, 162, TftColor::DGRAY);

  tft.drawTextBg(
    10, 256, STRFMT_NOBUF("Page #%d of %d", cur_page, max_page),
    TftColor::PURPLE, TftColor::BLACK
  );

  uint16_t glob_range_start = addr1 >> 8;
  uint16_t glob_page_start  = MAX(((cur_page + glob_range_start + 0) << 8) + 0, addr1);
  uint16_t glob_page_end    = MIN(((cur_page + glob_range_start + 1) << 8) - 1, addr2);

  tft.drawTextBg(
    TftCalc::right(tft, 130, 12), 12, STRFMT_NOBUF("%04X - %04X", glob_page_start, glob_page_end),
    TftColor::ORANGE, TftColor::BLACK
  );

  for (uint16_t i = glob_page_start; i <= glob_page_end; ++i) {
    uint8_t tft_byte_col = (i & 0x0F);
    uint8_t tft_byte_row = (i & 0xFF) >> 4;

    ByteRepr br          = (*repr)(data[i - addr1]);

    uint8_t split_offset = (tft_byte_col < 8 ? 0 : 3);
    uint16_t tft_byte_x  = tft.width() / 2 - 141 + 18 * tft_byte_col + br.offset + split_offset;
    uint16_t tft_byte_y  = tft.height() / 2 - 100 + 10 * tft_byte_row;

    tft.drawText(tft_byte_x, tft_byte_y, br.text, br.color, 1);
  }
}

inline ProgrammerMultiCore::ByteRepr ProgrammerMultiCore::multi_byte_repr_hex(uint8_t input_byte) {
  ByteRepr repr;

  repr.offset = 0;
  repr.color  = TftColor::WHITE;
  sprintf(repr.text, "%02X", input_byte);

  return repr;
}

inline ProgrammerMultiCore::ByteRepr ProgrammerMultiCore::multi_byte_repr_chars(uint8_t input_byte) {
  const bool printable = isprint(input_byte);
  ByteRepr repr;

  repr.offset = 3;
  repr.color  = (printable ? TftColor::WHITE : TftColor::GRAY);
  sprintf(repr.text, "%c", (printable ? input_byte : '?'));

  return repr;
}

Status ProgrammerMultiCore::store_file(uint8_t *data, uint16_t len) {
  using AFStatus = Dialog::AskFileStatus;

  AFStatus substatus;
  Status status  = Status::OK;
  FileCtrl *file = Dialog::ask_file(tft, tch, Strings::P_STORE, O_WRITE | O_CREAT | O_TRUNC, &substatus, false, sd);

  if (substatus == AFStatus::OK) {
    tft.fillScreen(TftColor::BLACK);
    tft.drawText(10, 10, Strings::W_WAIT, TftColor::PURPLE);

    file->write(data, len);
    file->flush();
    file->close();
  }
  else if (substatus == AFStatus::CANCELED) {
    Dialog::show_error(tft, tch, ErrorLevel::INFO, 0x3, Strings::T_CANCELED, Strings::E_CANCELED);
  }
  else if (substatus == AFStatus::FNAME_TOO_LONG) {
    Dialog::show_error(tft, tch, ErrorLevel::ERROR, 0x3, Strings::T_TOO_LONG, Strings::E_TOO_LONG);
    status = Status::ERR_FILE;
  }
  else if (substatus == AFStatus::FSYS_INVALID) {
    Dialog::show_error(tft, tch, ErrorLevel::ERROR, 0x3, Strings::T_INV_FSYS, Strings::E_INV_FSYS);
    status = Status::ERR_FILE;
  }

  delete file;
  return status;
}

Status ProgrammerMultiCore::write() {
  AddrDataArray buf;

  const uint16_t w1 = TftCalc::fraction_x(tft, 10, 1);
  const uint16_t w2 = TftCalc::fraction_x(tft, 10, 2);
  const uint16_t _h = TftCalc::fraction_y(tft, 59, 1);
  const uint16_t x1 = TftCalc::right(tft, 24, 10);
  const uint16_t x2 = 10 + w2 + 10;
  const uint16_t _y = TftCalc::bottom(tft, 24, 10);

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(10, 74, 24, _h, Strings::L_ARROW_U,  TftColor::WHITE, TftColor::DGRAY ));
  menu.add_btn(new Gui::Btn(x1, 74, 24, _h, Strings::L_ARROW_D,  TftColor::WHITE, TftColor::DGRAY ));
  menu.add_btn(new Gui::Btn(10, 40, w1, 24, Strings::L_ADD_PAIR, TftColor::WHITE, TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(10, _y, w2, 24, Strings::L_CONFIRM,  TftColor::RED,   TftColor::PINKK ));
  menu.add_btn(new Gui::Btn(x2, _y, w2, 24, Strings::L_CANCEL,   TftColor::CYAN,  TftColor::BLUE  ));

  Gui::Menu del_btns;

  const uint8_t num_pairs = 7;

  for (uint8_t i = 0; i < num_pairs; ++i) {
    del_btns.add_btn(new Gui::Btn(TftCalc::right(tft, 18, 44), 76 + 30 * i, 18, 18, Strings::L_X_CLOSE, TftColor::YELLOW, TftColor::RED));
  }

  uint16_t scroll = 0;

  bool done = false;

  while (!done) {
    tft.drawText(10, 10, Strings::T_WMULTI, TftColor::CYAN, 3);

    menu.draw(tft);
    draw_pairs(44, 44, 74, 44, 22, 8, num_pairs, scroll, buf, del_btns);
    del_btns.draw(tft);

    uint16_t max_scroll = MAX(0, (int32_t) buf.get_len() - num_pairs);

    done = poll_menus_and_react(menu, del_btns, &buf, &scroll, max_scroll);
  }

  tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(0, (void *) &buf)
}

void ProgrammerMultiCore::draw_pairs(
  uint16_t margin_l, uint16_t margin_r, uint16_t margin_u, uint16_t margin_d,
  uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, Gui::Menu &del_btns
) {
  // Clear the pairs area
  tft.fillRect(margin_l, margin_u, tft.width() - margin_l - margin_r, tft.height() - margin_u - margin_d, TftColor::BLACK);

  for (uint8_t i = 0; i < n; ++i) {
    // Hide all the delete buttons here, later show the ones that are needed
    auto cur_btn = del_btns.get_btn(i - offset);
    cur_btn->visibility(false);
    cur_btn->operation(false);
  }

  if (buf.get_len() == 0) {
    tft.drawText(margin_l, margin_u +  0, Strings::L_NO_PAIRS1, TftColor::LGRAY);
    tft.drawText(margin_l, margin_u + 30, Strings::L_NO_PAIRS2, TftColor::LGRAY);
    return;
  }

  uint16_t this_pair = offset;
  uint16_t last_pair = MIN(offset + n - 1U, buf.get_len() - 1U);

  do {
    uint16_t x  = margin_l;
    uint16_t y  = margin_u + (this_pair - offset) * (height + padding);
    uint16_t w  = tft.width() - margin_l - margin_r;
    uint16_t h  = height;
    uint16_t ty = TftCalc::t_center_y(h, 2) + y;

    AddrDataArrayPair pair;
    buf.get_pair(this_pair, &pair);

    tft.fillRect(x, y, w, h, TftColor::ORANGE);
    tft.drawText(margin_l + 3, ty, STRFMT_NOBUF("#%05d: %04X, %02X", this_pair, pair.addr, pair.data), TftColor::BLACK, 2);

    // Show all buttons that have pairs
    auto cur_btn = del_btns.get_btn(this_pair - offset);
    cur_btn->visibility(true);
    cur_btn->operation(true);
  }
  while (this_pair++ != last_pair);
}

bool ProgrammerMultiCore::poll_menus_and_react(
  Gui::Menu &menu, Gui::Menu &del_btns, AddrDataArray *buf, uint16_t *scroll, const uint16_t max_scroll
) {
  int16_t pressed, deleted;

  while (true) {
    deleted = del_btns.get_pressed(tch, tft);

    if (deleted >= 0) {
      buf->remove(deleted);
      break;
    }

    pressed = menu.get_pressed(tch, tft);

    if (pressed >= 0) {
      switch (pressed) {
      case 0:  if (*scroll > 0)          --*scroll; break;
      case 1:  if (*scroll < max_scroll) ++*scroll; break;
      case 2:  add_pair_from_user(buf);             break;
      case 3:
        tft.fillRect(10, 10, TftCalc::fraction_x(tft, 10, 1), 24, TftColor::BLACK);
        tft.drawText(10, 12, Strings::W_WMULTI, TftColor::CYAN, 2);
        ee.write(buf);
        [[fallthrough]];
      default: return true;
      }

      break;
    }
  }

  return false;
}

void ProgrammerMultiCore::add_pair_from_user(AddrDataArray *buf) {
  tft.fillScreen(TftColor::BLACK);
  auto addr = Dialog::ask_addr(tft, tch, Strings::P_ADDR_GEN);
  tft.fillScreen(TftColor::BLACK);
  auto data = Dialog::ask_int<uint8_t>(tft, tch, Strings::P_DATA_GEN);
  tft.fillScreen(TftColor::BLACK);

  buf->append((AddrDataArrayPair) {addr, data});
}

Status ProgrammerMultiCore::verify(uint16_t addr, void *data) {
  UNUSED_VAR(addr);  // addr is unused because `data` already contains addresses

  auto *buf = (AddrDataArray *) data;

  for (uint16_t i = 0; i < buf->get_len(); ++i) {
    AddrDataArrayPair pair;
    buf->get_pair(i, &pair);

    uint8_t real_data = ee.read(pair.addr);

    if (pair.data != real_data) {
      Dialog::show_error(
        tft, tch, ErrorLevel::ERROR, 0x1, Strings::T_MISMATCH,
        STRFMT_NOBUF(
          "Expected: %02X\n"
          "Actual:   %02X"
          "Address:  %04X",
          pair.data, real_data, pair.addr
        )
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
  tft_draw_test(tch, tft);

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
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(tft, Strings::L_CLOSE)));

  while (true) {
    tft.drawText_P(10, 10, Strings::T_DEBUGS, TftColor::CYAN, 4);
    menu.draw(tft);

    DebugAction btn = (DebugAction) menu.wait_for_press(tch, tft);

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
  case DebugAction::DISABLE_WRITE:       ee.set_we(true);                                                             return;
  case DebugAction::ENABLE_WRITE:        ee.set_we(false);                                                            return;
  case DebugAction::SET_ADDR_BUS_AND_OE: ee.set_addr_and_oe(Dialog::ask_int<uint16_t>(tft, tch, Strings::P_VAL_GEN)); return;
  case DebugAction::READ_DATA_BUS:       show_data_bus();                                                             return;
  case DebugAction::WRITE_DATA_BUS:      ee.set_data(Dialog::ask_int<uint8_t>(tft, tch, Strings::P_VAL_GEN));         return;
  case DebugAction::SET_DATA_DIR:        set_data_dir();                                                              return;
  case DebugAction::MONITOR_DATA_BUS:    monitor_data_bus();                                                          return;
  case DebugAction::PRINT_CHARSET:       tft_print_chars(tft); tch.wait_for_press();                                  return;
  case DebugAction::SHOW_COLORS:         tft_show_colors(tft);                                                        return;
  case DebugAction::ACTION_AUX1:         debug_action_aux1();                                                         return;
  case DebugAction::ACTION_AUX2:         debug_action_aux2();                                                         return;
  }
}

void ProgrammerOtherCore::show_data_bus() {
  Dialog::show_error(
    tft, tch, ErrorLevel::INFO, 0x1, Strings::T_VALUE,
    STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(ee.get_data()))
  );
}

void ProgrammerOtherCore::monitor_data_bus() {
  ee.set_ddr(false);

  Gui::Btn close_btn(BOTTOM_BTN(tft, Strings::L_CLOSE));
  close_btn.draw(tft);

#ifdef DEBUG_MODE
  while (!close_btn.is_pressed(tch, tft)) {
    uint8_t val = ee.get_io_exp(true)->read_port(MCP_EE_DATA_PORT);

    tft.drawTextBg(10, 10, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(val)), TftColor::CYAN, TftColor::BLACK, 3);
    delay(500);
  }
#else
  // EepromCtrl::get_io_exp() only exists in DEBUG_MODE

  Dialog::show_error(tft, tch, ErrorLevel::ERROR, 0x3, Strings::T_NOT_SUPP, Strings::E_NO_DB_MON);

  close_btn.wait_for_press(tch, tft);
#endif
}

void ProgrammerOtherCore::set_data_dir() {
  ee.set_ddr(
    Dialog::ask_choice(
      tft, tch, Strings::P_DATA_DIR, 1, 45, 0, 2,
      Strings::L_INPUT, TftColor::CYAN, TftColor::BLUE,
      Strings::L_OUTPUT, TftColor::PINKK, TftColor::RED
    )
  );
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

  TftUtil::wait_bottom_btn(tft, tch, Strings::L_CLOSE);

  tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

uint16_t Dialog::ask_addr(TftCtrl &tft, TouchCtrl &tch, const char *prompt) {
  uint16_t addr = ask_int<uint16_t>(tft, tch, prompt);
  Util::validate_addr(&addr);

  return addr;
}

#undef RETURN_VERIFICATION_OR_VALUE
#undef RETURN_VERIFICATION_OR_OK
