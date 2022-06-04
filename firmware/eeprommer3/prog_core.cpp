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
#include "vector.hpp"

#include "prog_core.hpp"

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = Dialog::ask_yesno(m_tft, m_tch, Strings::P_VERIFY); \
  m_tft.fillScreen(TftColor::BLACK); \
  \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

using Status = ProgrammerBaseCore::Status;

// Dummy function for unimplemented actions
Status ProgrammerBaseCore::nop() {
  return Status::OK;
}

/***************************/
/******** BYTE CORE ********/
/***************************/

Status ProgrammerByteCore::read() {
  uint16_t addr = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_GEN);

  uint8_t data = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);

  char title[64];
  snprintf(title, 63, "Value at addr %04X", addr);

  Dialog::show_error(
    m_tft, m_tch, ErrorLevel::INFO, title,
    STRFMT_NOBUF(
      "BIN: " BYTE_FMT "\n"
      "OCT: %03o\n"
      "HEX: %02X\n"
      "DEC: %-3d\n"
      "CHR: %c",
      BYTE_FMT_VAL(data), data, data, data, data
    )
  );

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerByteCore::write() {
  uint16_t addr = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_GEN);
  m_tft.fillScreen(TftColor::BLACK);
  uint8_t data = Dialog::ask_int<uint8_t>(m_tft, m_tch, Strings::P_DATA_GEN);

  m_ee.write(addr, data);

  m_tft.fillScreen(TftColor::BLACK);

  Dialog::show_error(
    m_tft, m_tch, ErrorLevel::INFO, Strings::T_DONE,
    STRFMT_NOBUF(
      "Wrote data %02X\n"
      "to address %04X.",
      data, addr
    )
  );

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(addr, (void *) &data);
}

Status ProgrammerByteCore::verify(uint16_t addr, void *data) {
  uint8_t actual = m_ee.read(addr);

  if (actual != *(uint8_t *) data) {
    Dialog::show_error(
      m_tft, m_tch, ErrorLevel::INFO, Strings::T_MISMATCH,
      STRFMT_NOBUF(
        "Expected: %02X\n"
        "Actual:   %02X",
        *(uint8_t *) data, actual
      )
    );

    m_tft.fillScreen(TftColor::BLACK);

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
  FileCtrl *file = Dialog::ask_file(m_tft, m_tch, Strings::P_OFILE, O_CREAT | O_TRUNC | O_WRITE, &fstatus, false, m_sd);
  m_tft.fillScreen(TftColor::BLACK);

  if (fstatus != AFStatus::OK) return (fstatus == AFStatus::CANCELED ? Status::OK : Status::ERR_FILE);

  if (!FileCtrl::check_valid(file)) {
    delete file;
    return Status::ERR_FILE;
  }

  read_operation_core(file);
  file->flush();
  file->close();
  delete file;

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

void ProgrammerFileCore::read_operation_core(FileCtrl *file) {
  uint8_t this_page[256];

  m_tft.drawText(10, 10, Strings::W_OFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(m_tft, 0x80, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bar.for_each(
    [this, &this_page, &file]GUI_PROGRESS_INDICATOR_LAMBDA {
      uint16_t addr = progress << 8;

      this->m_ee.read(addr, addr + 0xFF, this_page);
      file->write(this_page, 0xFF);

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue(m_tft, m_tch);
}

Status ProgrammerFileCore::write() {
  using AFStatus = Dialog::AskFileStatus;
  
  AFStatus fstatus;
  FileCtrl *file = Dialog::ask_file(m_tft, m_tch, Strings::P_IFILE, O_RDONLY, &fstatus, true, m_sd);
  m_tft.fillScreen(TftColor::BLACK);

  if (fstatus != AFStatus::OK) return (fstatus == AFStatus::CANCELED ? Status::OK : Status::ERR_FILE);

  if (!FileCtrl::check_valid(file)) {
    delete file;
    return Status::ERR_FILE;
  }

  uint16_t addr = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_FILE);
  m_tft.fillScreen(TftColor::BLACK);

  bool success = write_from_file(file, addr);
  m_tft.fillScreen(TftColor::BLACK);

  if (!success) {
    delete file;
    return Status::ERR_INVALID;
  }

  bool should_verify = Dialog::ask_yesno(m_tft, m_tch, Strings::P_VERIFY);
  m_tft.fillScreen(TftColor::BLACK);

  Status status = Status::OK;
  if (should_verify) status = verify(addr, file);

  file->close();
  delete file;

  return status;
}

bool ProgrammerFileCore::write_from_file(FileCtrl *file, uint16_t addr) {
  if (file->size() > (0x7FFF - addr + 1)) {
    Dialog::show_error(m_tft, m_tch, ErrorLevel::WARNING, Strings::T_TOO_BIG, Strings::E_TOO_BIG);
    return false;
  }

  write_operation_core(file, addr);
  return true;
}

void ProgrammerFileCore::write_operation_core(FileCtrl *file, uint16_t addr) {
  uint16_t cur_addr = addr;
  uint8_t this_page[256];

  m_tft.drawText(10, 10, Strings::W_IFILE, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(m_tft, ceil((float) file->size() / 256.0), 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bar.for_each(
    [this, &this_page, &file, &cur_addr]GUI_PROGRESS_INDICATOR_LAMBDA {
      auto len = file->read(this_page, 0xFF);
      this->m_ee.write(cur_addr, this_page, MIN(len, 0xFF));

      cur_addr += 0x0100; // Next page

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, Strings::F_WRITE, TftColor::CYAN);
  TftUtil::wait_continue(m_tft, m_tch);
}

Status ProgrammerFileCore::verify(uint16_t addr, void *data) {
  auto *file = (FileCtrl *) data;

  uint8_t expected[256], reality[256];

  m_tft.drawText(10, 10, STRFMT_NOBUF("Verifying `%s' at %04X...", file->name(), addr), TftColor::CYAN);

  Gui::ProgressIndicator bar(m_tft, ceil((float) file->size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bool complete = bar.for_each(
    [this, &expected, &reality, &file, &addr]GUI_PROGRESS_INDICATOR_LAMBDA {
      auto nbytes = file->read(expected, 256);
      this->m_ee.read(addr, addr + nbytes, reality);

      if (memcmp(expected, reality, nbytes) != 0) {
        this->m_tft.drawText(10, 150, STRFMT_NOBUF("Mismatch between %04X and %04X!", addr, addr + 0xFF), TftColor::RED);
        return true; // Request to quit loop
      }

      addr += 0x0100; // Next page
      return false;
    }
  );

  m_tft.drawText(10, 110, Strings::F_VERIFY, TftColor::CYAN);
  TftUtil::wait_continue(m_tft, m_tch);

  file->close();
  m_tft.fillScreen(TftColor::BLACK);
  return (complete ? Status::OK : Status::ERR_VERIFY);
}

/*****************************/
/******** VECTOR CORE ********/
/*****************************/

Status ProgrammerVectorCore::read() {
  Vector vec = Dialog::ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  char title[64];
  snprintf(title, 63, "Value of %s", Vector::NAMES[vec.m_id]);

  Dialog::show_error(
    m_tft, m_tch, ErrorLevel::INFO, title,
    STRFMT_NOBUF(
      "Addr: %04X-%04X\n"
      "HEX: %04X\n"
      "BIN: " BYTE_FMT "\n"
      ".... " BYTE_FMT,
      vec.m_addr, vec.m_addr + 1, vec.m_val, BYTE_FMT_VAL(vec.m_hi), BYTE_FMT_VAL(vec.m_lo)
    )
  );

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerVectorCore::write() {
  Vector vec = Dialog::ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = Dialog::ask_int<uint16_t>(m_tft, m_tch, Strings::P_ADDR_VEC);
  m_ee.write(vec.m_addr,     new_val & 0xFF);
  m_ee.write(vec.m_addr + 1, new_val >> 8);

  m_tft.fillScreen(TftColor::BLACK);

  Dialog::show_error(
    m_tft, m_tch, ErrorLevel::INFO, Strings::T_DONE,
    STRFMT_NOBUF(
      "Wrote value %04X\n"
      "to vector %s\n"
      "at %04X-%04X.",
      new_val, Vector::NAMES[vec.m_id], vec.m_addr, vec.m_addr + 1
    )
  );

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(vec.m_addr, (void *) &new_val)
}

Status ProgrammerVectorCore::verify(uint16_t addr, void *data) {
  uint16_t actual = (m_ee.read(addr + 1) << 8) | m_ee.read(addr);

  if (actual != *(uint16_t *) data) {
    Dialog::show_error(
      m_tft, m_tch, ErrorLevel::ERROR, Strings::T_MISMATCH,
      STRFMT_NOBUF(
        "Expected: %04X\n"
        "Actual:   %04X",
        *(uint16_t *) data, actual
      )
    );

    TftUtil::wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/********************************/
/******** MULTIBYTE CORE ********/
/********************************/

Status ProgrammerMultiCore::read() {
  uint16_t addr1 = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_BEG);
  m_tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_END);

  Util::validate_addrs(&addr1, &addr2);

  uint16_t nbytes = (addr2 - addr1 + 1);

  auto *data = (uint8_t *) malloc(nbytes * sizeof(uint8_t));
  if (data == nullptr) {
    m_tft.fillScreen(TftColor::BLACK);
    return Status::ERR_MEMORY;
  }

  read_operation_core(data, addr1, addr2);

  m_tft.fillScreen(TftColor::BLACK);

  Status status = handle_data(data, addr1, addr2);

  m_tft.fillScreen(TftColor::BLACK);

  free(data);

  return status;
}

void ProgrammerMultiCore::read_operation_core(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText(10, 10, Strings::W_RMULTI, TftColor::CYAN, 3);

  Gui::ProgressIndicator bar(m_tft, ceil((float) nbytes / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  uint16_t cur_addr_offset = 0x0000;

  bar.for_each(
    [this, &data, &cur_addr_offset, &addr1, &addr2]GUI_PROGRESS_INDICATOR_LAMBDA {
      uint16_t _addr1 = addr1 + cur_addr_offset;
      uint16_t _addr2 = MIN(_addr1 + 0xFF, addr2);

      this->m_ee.read(_addr1, _addr2, data + cur_addr_offset);

      cur_addr_offset += 0x0100; // Next page

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, Strings::F_READ, TftColor::CYAN);
  TftUtil::wait_continue(m_tft, m_tch);
}

Status ProgrammerMultiCore::handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  uint8_t viewing_method = Dialog::ask_choice(
    m_tft, m_tch, Strings::P_VIEW_METH, 1, 30, 0, 3,
    Strings::L_VM_HEX,  TftColor::BLACK,  TftColor::ORANGE,
    Strings::L_VM_CHAR, TftColor::LGREEN, TftColor::DGREEN,
    Strings::L_VM_FILE, TftColor::CYAN,   TftColor::BLUE
  );

  m_tft.fillScreen(TftColor::BLACK);

  if      (viewing_method == 0) show_range(data, addr1, addr2, &multi_byte_repr_hex);
  else if (viewing_method == 1) show_range(data, addr1, addr2, &multi_byte_repr_chars);
  else if (viewing_method == 2) return store_file(data, nbytes);

  return Status::OK;
}

void ProgrammerMultiCore::show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr) {
  // Draw frame for the data
  m_tft.drawText(10, 10, STRFMT_NOBUF("%d bytes", addr2 - addr1 + 1), TftColor::CYAN, 3);
  m_tft.drawThickRect(m_tft.width() / 2 - 147, 55, 295, 166, TftColor::WHITE, 2);
  m_tft.drawFastVLine(m_tft.width() / 2, 57, 162, TftColor::GRAY);

  draw_page_axis_labels();

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(15,                 60, 40, 150, 15, 68, "\x11"));
  menu.add_btn(new Gui::Btn(m_tft.width() - 55, 60, 40, 150, 15, 68, "\x10"));
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(m_tft, Strings::L_CONTINUE)));
  menu.draw(m_tft);

  uint8_t cur_page = 0;
  uint8_t max_page = (addr2 >> 8) - (addr1 >> 8);

  while (true) {
    show_page(data, addr1, addr2, repr, cur_page, max_page);

    uint8_t req = menu.wait_for_press(m_tch, m_tft);

    if      (req == 2) break;
    else if (req == 0) cur_page = (cur_page == 0 ? max_page : cur_page - 1);
    else if (req == 1) cur_page = (cur_page == max_page ? 0 : cur_page + 1);
  }
}

void ProgrammerMultiCore::draw_page_axis_labels() {
  for (uint8_t row = 0x00; row < 0x10; ++row) {
    uint16_t x1 = m_tft.width() / 2 - 161;
    uint16_t x2 = m_tft.width() / 2 + 151;

    uint16_t y = m_tft.height() / 2 - 100 + 10 * row;

    m_tft.drawText(x1, y, STRFMT_NOBUF("%1Xx", row), TftColor::DCYAN, 1);
    m_tft.drawText(x2, y, STRFMT_NOBUF("%1Xx", row), TftColor::DCYAN, 1);
  }

  for (uint8_t col = 0x00; col < 0x10; ++col) {
    uint8_t split_offset = (col < 8 ? 0 : 3);
    uint16_t x = m_tft.width() / 2 - 141 + 18 * col + split_offset;

    m_tft.drawText(x, m_tft.height() / 2 + 64, STRFMT_NOBUF("x%1X", col), TftColor::DCYAN, 1);
  }
}

void ProgrammerMultiCore::show_page(
  uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr, uint8_t cur_page, uint8_t max_page
) {
  m_tft.fillRect(m_tft.width() / 2 - 145, 57, 145, 162, TftColor::DGRAY);
  m_tft.fillRect(m_tft.width() / 2 +   1, 57, 145, 162, TftColor::DGRAY);

  m_tft.drawTextBg(
    10, 256, STRFMT_NOBUF("Page #%d of %d", cur_page, max_page),
    TftColor::PURPLE, TftColor::BLACK
  );

  uint16_t glob_range_start = addr1 >> 8;
  uint16_t glob_page_start  = MAX(((cur_page + glob_range_start)     << 8),     addr1);
  uint16_t glob_page_end    = MIN(((cur_page + glob_range_start + 1) << 8) - 1, addr2);

  m_tft.drawTextBg(
    TftCalc::right(m_tft, 130, 12), 12, STRFMT_NOBUF("%04X - %04X", glob_page_start, glob_page_end),
    TftColor::ORANGE, TftColor::BLACK
  );

  for (uint16_t i = glob_page_start; i <= glob_page_end; ++i) {
    uint8_t tft_byte_col = (i & 0x0F);
    uint8_t tft_byte_row = (i & 0xFF) >> 4;

    ByteRepr br = (*repr)(data[i - addr1]);

    uint8_t split_offset = (tft_byte_col < 8 ? 0 : 3);
    uint16_t tft_byte_x = m_tft.width()  / 2 - 141 + 18 * tft_byte_col + br.offset + split_offset;
    uint16_t tft_byte_y = m_tft.height() / 2 - 100 + 10 * tft_byte_row;

    m_tft.drawText(tft_byte_x, tft_byte_y, br.text, br.color, 1);
  }
}

inline ProgrammerMultiCore::ByteRepr ProgrammerMultiCore::multi_byte_repr_hex(uint8_t input_byte) {
  ByteRepr repr;

  repr.offset = 0;
  repr.color = TftColor::WHITE;
  sprintf(repr.text, "%02X", input_byte);

  return repr;
}

inline ProgrammerMultiCore::ByteRepr ProgrammerMultiCore::multi_byte_repr_chars(uint8_t input_byte) {
  ByteRepr repr;
  
  repr.offset = 3;
  repr.color = (isprint((char) input_byte) ? TftColor::WHITE : TftColor::GRAY);
  sprintf(repr.text, "%c", (isprint((char) input_byte) ? (char) input_byte : '?'));

  return repr;
}

Status ProgrammerMultiCore::store_file(uint8_t *data, uint16_t len) {
  using AFStatus = Dialog::AskFileStatus;

  Status status = Status::OK;
  AFStatus substatus;
  FileCtrl *file = Dialog::ask_file(m_tft, m_tch, Strings::P_STORE, O_WRITE | O_CREAT | O_TRUNC, &substatus, false, m_sd);

  if (substatus == AFStatus::OK) {
    m_tft.fillScreen(TftColor::BLACK);
    m_tft.drawText(10, 10, Strings::W_WAIT, TftColor::PURPLE);

    file->write(data, len);
    file->flush();
    file->close();
  }
  else if (substatus == AFStatus::CANCELED) {
    Dialog::show_error(m_tft, m_tch, ErrorLevel::INFO, Strings::T_CANCELED, Strings::E_CANCELED);
  }
  else if (substatus == AFStatus::FNAME_TOO_LONG) {
    Dialog::show_error(m_tft, m_tch, ErrorLevel::ERROR, Strings::T_TOO_LONG, Strings::E_TOO_LONG);
    status = Status::ERR_FILE;
  }
  else if (substatus == AFStatus::FSYS_INVALID) {
    Dialog::show_error(m_tft, m_tch, ErrorLevel::ERROR, Strings::T_INV_FSYS, Strings::E_INV_FSYS);
    status = Status::ERR_FILE;
  }

  delete file;
  return status;
}

Status ProgrammerMultiCore::write() {
  AddrDataArray buf;

  const uint16_t x1 = TftCalc::right(m_tft, 24, 10);
  const uint16_t _y = TftCalc::bottom(m_tft, 24, 10);
  const uint16_t w1 = TftCalc::fraction_x(m_tft, 10, 1);
  const uint16_t w2 = TftCalc::fraction_x(m_tft, 10, 2);
  const uint16_t _h = TftCalc::fraction_y(m_tft, 59, 1);

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(10,           74, 24, _h, Strings::L_LEFT,     TftColor::WHITE, TftColor::DGRAY));
  menu.add_btn(new Gui::Btn(x1,           74, 24, _h, Strings::L_RIGHT,    TftColor::WHITE, TftColor::DGRAY));
  menu.add_btn(new Gui::Btn(10,           40, w1, 24, Strings::L_ADD_PAIR, TftColor::WHITE, TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(10,           _y, w2, 24, Strings::L_DONE,     TftColor::RED,   TftColor::PINKK));
  menu.add_btn(new Gui::Btn(10 + w2 + 10, _y, w2, 24, Strings::L_CANCEL,   TftColor::CYAN,  TftColor::BLUE));

  Gui::Menu del_btns;

  const uint8_t num_pairs = 7;

  for (uint8_t i = 0; i < num_pairs; ++i) {
    del_btns.add_btn(new Gui::Btn(TftCalc::right(m_tft, 18, 44), 76 + 30 * i, 18, 18, "x", TftColor::YELLOW, TftColor::RED));
  }

  uint16_t scroll = 0;

  bool done = false;

  while (!done) {
    m_tft.drawText(10, 10, Strings::T_WMULTI, TftColor::CYAN, 3);

    menu.draw(m_tft);
    draw_pairs(44, 44, 74, 44, 22, 8, num_pairs, scroll, buf, del_btns);
    del_btns.draw(m_tft);

    int32_t diff = (int32_t) buf.get_len() - num_pairs;
    uint16_t max_scroll = MAX(0, diff);

    done = poll_menus_and_react(menu, del_btns, &buf, &scroll, max_scroll);
  }

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(0, (void *) &buf)
}

void ProgrammerMultiCore::draw_pairs(
  uint16_t margin_l, uint16_t margin_r, uint16_t margin_u, uint16_t margin_d,
  uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, Gui::Menu &del_btns
) {
  // Clear the pairs area
  m_tft.fillRect(margin_l, margin_u, m_tft.width() - margin_l - margin_r, m_tft.height() - margin_u - margin_d, TftColor::BLACK);

  for (uint8_t i = 0; i < n; ++i) {
    // Hide all the delete buttons here, later show the ones that are needed
    auto cur_btn = del_btns.get_btn(i - offset);
    cur_btn->visibility(false);
    cur_btn->operation(false);
  }

  if (buf.get_len() == 0) {
    m_tft.drawText(margin_l, margin_u,      "No pairs yet!",                   TftColor::LGRAY);
    m_tft.drawText(margin_l, margin_u + 30, "Click `Add Pair' to add a pair!", TftColor::LGRAY);
    return;
  }

  uint16_t this_pair = offset;
  uint16_t last_pair = MIN(offset + n - 1U, buf.get_len() - 1U);

  do {
    uint16_t x = margin_l;
    uint16_t y = margin_u + (this_pair - offset) * (height + padding);
    uint16_t w = m_tft.width() - margin_l - margin_r;
    uint16_t h = height;
    uint16_t ty = TftCalc::t_center_y(h, 2) + y;

    AddrDataArrayPair pair;
    buf.get_pair(this_pair, &pair);

    m_tft.fillRect(x, y, w, h, TftColor::ORANGE);
    m_tft.drawText(margin_l + 3, ty, STRFMT_NOBUF("#%05d: %04X, %02X", this_pair, pair.addr, pair.data), TftColor::BLACK, 2);

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
    pressed = menu.get_pressed(m_tch, m_tft);
    deleted = del_btns.get_pressed(m_tch, m_tft);

    if (deleted >= 0) {
      buf->remove(deleted);
      break;
    }
    else if (pressed >= 0) {
      switch (pressed) {
      case 0:  if (*scroll > 0)          --*scroll; break;
      case 1:  if (*scroll < max_scroll) ++*scroll; break;
      case 2:  add_pair_from_user(buf);   break;
      case 3:
        m_tft.fillRect(10, 10, TftCalc::fraction_x(m_tft, 10, 1), 24, TftColor::BLACK);
        m_tft.drawText(10, 12, Strings::W_WMULTI, TftColor::CYAN, 2);
        m_ee.write(buf);
        [[fallthrough]];
      default: return true;
      }

      break;
    }
  }

  return false;
}

void ProgrammerMultiCore::add_pair_from_user(AddrDataArray *buf) {
  m_tft.fillScreen(TftColor::BLACK);
  auto addr = Dialog::ask_addr(m_tft, m_tch, Strings::P_ADDR_GEN);
  m_tft.fillScreen(TftColor::BLACK);
  auto data = Dialog::ask_int<uint8_t>(m_tft, m_tch, Strings::P_DATA_GEN);
  m_tft.fillScreen(TftColor::BLACK);

  buf->append((AddrDataArrayPair) {addr, data});
}

Status ProgrammerMultiCore::verify(uint16_t addr, void *data) {
  auto *buf = (AddrDataArray *) data;

  for (uint16_t i = 0; i < buf->get_len(); ++i) {
    AddrDataArrayPair pair;
    buf->get_pair(i, &pair);

    uint8_t real_data = m_ee.read(pair.addr);

    if (pair.data != real_data) {
      Dialog::show_error(
        m_tft, m_tch, ErrorLevel::ERROR, Strings::T_MISMATCH,
        STRFMT_NOBUF(
          "Expected: %02X\n"
          "Actual:   %02X"
          "Address:  %04X",
          pair.data, real_data, pair.addr
        )
      );

      m_tft.fillScreen(TftColor::BLACK);

      return Status::ERR_VERIFY;
    }
  }

  return Status::OK;
}

/***********************************/
/******** OTHER CORE - MISC ********/
/***********************************/

Status ProgrammerOtherCore::paint() {
  tft_draw_test(m_tch, m_tft);

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerOtherCore::debug() {
  auto w1 = TftCalc::fraction_x(m_tft, 10, 1);
  auto w2 = TftCalc::fraction_x(m_tft, 10, 2);

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(     10,  50, w2, 28, "WE Hi (Disable)", TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN));
  menu.add_btn(new Gui::Btn(w2 + 20,  50, w2, 28, "WE Lo (Enable)",  TftColor::PINKK,          TftColor::RED   ));
  menu.add_btn(new Gui::Btn(     10,  88, w1, 28, "Set Address/OE",  TftColor::BLACK,          TftColor::YELLOW));
  menu.add_btn(new Gui::Btn(     10, 126, w2, 28, "Read Data Bus",   TftColor::BLUE,           TftColor::CYAN  ));
  menu.add_btn(new Gui::Btn(w2 + 20, 126, w2, 28, "Write Data Bus",  TftColor::CYAN,           TftColor::BLUE  ));
  menu.add_btn(new Gui::Btn(     10, 164, w2, 28, "Set Data Dir",    TftColor::BLACK,          TftColor::ORANGE));
  menu.add_btn(new Gui::Btn(w2 + 20, 164, w2, 28, "Monitor Data",    TftColor::YELLOW,         TftColor::DCYAN ));
  menu.add_btn(new Gui::Btn(     10, 202, w2, 28, "Print Charset",   TftColor::PINKK,          TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(w2 + 20, 202, w2, 28, "Show Colors",     TftColor::PINKK,          TftColor::PURPLE));
  menu.add_btn(new Gui::Btn(     10, 240, w2, 28, "Aux1",            TftColor::DGRAY,          TftColor::LGRAY ));
  menu.add_btn(new Gui::Btn(w2 + 20, 240, w2, 28, "Aux2",            TftColor::DGRAY,          TftColor::LGRAY ));
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(m_tft, Strings::L_CLOSE)));

  while (true) {
    m_tft.drawText(10, 10, "Debug Tools Menu", TftColor::CYAN, 4);
    menu.draw(m_tft);

    DebugAction btn = (DebugAction) menu.wait_for_press(m_tch, m_tft);

    if (btn == menu.get_num_btns() - 1) break;

    m_tft.fillScreen(TftColor::BLACK);
    do_debug_action(btn);
    m_tft.fillScreen(TftColor::BLACK);
  }

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

void ProgrammerOtherCore::do_debug_action(DebugAction action) {
  if (action == DebugAction::DISABLE_WRITE) {
    m_ee.set_we(true);
  }
  else if (action == DebugAction::ENABLE_WRITE) {
    m_ee.set_we(false);
  }
  else if (action == DebugAction::SET_ADDR_BUS_AND_OE) { 
    m_ee.set_addr_and_oe(
      Dialog::ask_int<uint16_t>(m_tft, m_tch, Strings::P_VAL_GEN)
    );
  }
  else if (action == DebugAction::READ_DATA_BUS) {
    Dialog::show_error(
      m_tft, m_tch, ErrorLevel::INFO, Strings::T_VALUE,
      STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(m_ee.get_data()))
    );
  }
  else if (action == DebugAction::WRITE_DATA_BUS) {
    m_ee.set_data(
      Dialog::ask_int<uint8_t>(m_tft, m_tch, Strings::P_VAL_GEN)
    );
  }
  else if (action == DebugAction::SET_DATA_DIR) {
    m_ee.set_ddr(
      Dialog::ask_choice(
        m_tft, m_tch, Strings::P_DATA_DIR, 1, 45, 0, 2,
        Strings::L_INPUT,  TftColor::CYAN,   TftColor::BLUE,
        Strings::L_OUTPUT, TftColor::PINKK,  TftColor::RED
      )
    );
  }
  else if (action == DebugAction::MONITOR_DATA_BUS) {
    monitor_data_bus();
  }
  else if (action == DebugAction::PRINT_CHARSET) {
    tft_print_chars(m_tft);
    m_tch.wait_for_press();
  }
  else if (action == DebugAction::SHOW_COLORS) {
    tft_show_colors(m_tft);
  }
  else if (action == DebugAction::ACTION_AUX1) {
    debug_action_aux1();
  }
  else if (action == DebugAction::ACTION_AUX2) {
    debug_action_aux2();
  }
}

void ProgrammerOtherCore::monitor_data_bus() {
  m_ee.set_ddr(false);

  Gui::Btn close_btn(BOTTOM_BTN(m_tft, Strings::L_CLOSE));
  close_btn.draw(m_tft);

#ifdef DEBUG_MODE
  while (!close_btn.is_pressed(m_tch, m_tft)) {
    uint8_t val = m_ee.get_io_exp(true)->read_port(MCP_EE_DATA_PORT);

    m_tft.drawTextBg(10, 10, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(val)), TftColor::CYAN, TftColor::BLACK, 3);
    delay(500);
  }
#else
  // EepromCtrl::get_io_exp() only exists in DEBUG_MODE

  Dialog::show_error(
    m_tft, m_tch, ErrorLevel::ERROR, Strings::T_NOT_SUPP,
    "Data bus monitor is not\n"
    "supported because DEBUG_MODE\n"
    "is disabled."
  );

  close_btn.wait_for_press(m_tch, m_tft);
#endif
}

void ProgrammerOtherCore::debug_action_aux1() {
  uint8_t temp[] {
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  };

  m_ee.write(0x200, temp, 32);
}

void ProgrammerOtherCore::debug_action_aux2() {
  // Unused
}

Status ProgrammerOtherCore::about() {
  m_tft.drawText( 10,  10, "About",                    TftColor::CYAN,   3);
  m_tft.drawText( 10,  50, "eeprommer3",               TftColor::PURPLE, 2);
  m_tft.drawText(142,  50, "- hardware/firmware side", TftColor::BLUE,   2);
  m_tft.drawText( 10,  90, "AT28C256 EEPROM programmer using 2 I2C", TftColor::LGRAY);
  m_tft.drawText( 10, 120, "MCP23017 chips. Use: standalone device", TftColor::LGRAY);
  m_tft.drawText( 10, 150, "or computer peripheral via USB. Allows", TftColor::LGRAY);
  m_tft.drawText( 10, 180, "access to SD card connected on SPI bus", TftColor::LGRAY);
  m_tft.drawText( 10, 240, "Made by beaver700nh (GitHub) 2021-2022", TftColor::DGRAY);

  TftUtil::wait_bottom_btn(m_tft, m_tch, Strings::L_CLOSE);

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

uint16_t Dialog::ask_addr(TftCtrl &tft, TouchCtrl &tch, const char *prompt) {
  uint16_t addr = ask_int<uint16_t>(tft, tch, prompt);
  Util::validate_addr(&addr);

  return addr;
}

#undef RETURN_VERIFICATION_OR_VALUE
#undef RETURN_VERIFICATION_OR_OK
