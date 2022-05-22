#include <Arduino.h>
#include "constants.hpp"

#include "dialog.hpp"
#include "eeprom.hpp"
#include "file.hpp"
#include "new_delete.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "vector.hpp"

#include "prog_core.hpp"

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?"); \
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
  uint16_t addr = ask_addr(m_tft, m_tch, "Type an address:");

  uint8_t data = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, STRFMT_NOBUF("Value at address %04X:", addr),       TftColor::CYAN,   3);
  m_tft.drawText(20,  50, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(data)), TftColor::YELLOW, 2);
  m_tft.drawText(20,  80, STRFMT_NOBUF("OCT: %03o",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 110, STRFMT_NOBUF("HEX: %02X",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 140, STRFMT_NOBUF("DEC: %-3d",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 170, STRFMT_NOBUF("CHR: %c",        data),               TftColor::YELLOW, 2);

  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerByteCore::write() {
  uint16_t addr = ask_addr(m_tft, m_tch, "Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  uint8_t data = ask_val<uint8_t>(m_tft, m_tch, "Type the data:");

  m_ee.write(addr, data);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                             TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("data %02X", data),     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("address %04X.", addr), TftColor::GREEN,  4);

  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(addr, (void *) &data);
}

Status ProgrammerByteCore::verify(uint16_t addr, void *data) {
  uint8_t actual = m_ee.read(addr);

  if (actual != *(uint8_t *) data) {
    m_tft.drawText(10, 10, "Result:",                                         TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %02X", *(uint8_t *) data), TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %02X", actual),            TftColor::MAGENTA, 3);

    TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

    m_tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/***************************/
/******** FILE CORE ********/
/***************************/

ProgrammerFileCore::ProgrammerFileCore(TYPED_CONTROLLERS) : ProgrammerBaseCore(CONTROLLERS) {
  // Empty; all work delegated to base ctor
}

Status ProgrammerFileCore::check_valid(FileCtrl *file) {
  if (file == nullptr) {
    TftUtil::show_error(m_tft, m_tch, "Can't open file: no filesystem!");
    return Status::ERR_FILE;
  }

  if (!file->is_open()) {
    TftUtil::show_error(m_tft, m_tch, "Failed to open file!");
    return Status::ERR_FILE;
  }

  return Status::OK;
}

Status ProgrammerFileCore::read() {
  Status status = Status::OK;
  FileSystem fsys = ask_fsys(m_tft, m_tch, "Select a file type:", m_sd);
  m_tft.fillScreen(TftColor::BLACK);

  if (fsys == FileSystem::NONE) return Status::OK;

  char fpath[64];
  ask_str(m_tft, m_tch, "File to read to?", fpath, 63);

  m_tft.fillScreen(TftColor::BLACK);

  status = read_to_fsys(fpath, fsys);

  m_tft.fillScreen(TftColor::BLACK);

  return status;
}

Status ProgrammerFileCore::read_to_fsys(const char *fpath, FileSystem fsys) {
  FileCtrl *file = FileCtrl::create_file(fsys, fpath, O_CREAT | O_WRITE | O_TRUNC);
  Status status = check_valid(file);

  if (status == Status::OK) {
    do_read_operation(file);

    file->flush();
    file->close();
    delete file;
  }

  return status;
}

void ProgrammerFileCore::do_read_operation(FileCtrl *file) {
  uint8_t this_page[256];

  m_tft.drawText(10, 10, "Reading EEPROM to file...", TftColor::CYAN, 3);

  TftProgressIndicator bar(m_tft, 0x80, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bar.for_each(
    [this, &this_page, &file]TFT_PROGRESS_INDICATOR_LAMBDA {
      uint16_t addr = progress << 8;

      this->m_ee.read(addr, addr + 0xFF, this_page);
      file->write(this_page, 0xFF);

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, "Done reading!", TftColor::CYAN);
  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");
}

Status ProgrammerFileCore::write() {
  Status status = Status::OK;
  FileSystem fsys = ask_fsys(m_tft, m_tch, "Select a file type:", m_sd);
  m_tft.fillScreen(TftColor::BLACK);

  if (fsys == FileSystem::NONE) return Status::OK;

  char fpath[64];

  if (!get_file_to_write_from(fpath, 63, &status, fsys)) {
    m_tft.fillScreen(TftColor::BLACK);
    return status;
  }

  uint16_t addr = ask_addr(m_tft, m_tch, "Where to write in EEPROM?");

  m_tft.fillScreen(TftColor::BLACK);

  status = write_from_fsys(fpath, fsys, addr);

  m_tft.fillScreen(TftColor::BLACK);

  if (status == Status::OK) {
    RETURN_VERIFICATION_OR_VALUE(status, addr, fpath);
  }

  return status;
}

Status ProgrammerFileCore::write_from_fsys(const char *fpath, FileSystem fsys, uint16_t addr) {
  FileCtrl *file = FileCtrl::create_file(fsys, fpath, O_RDONLY);
  Status status = check_valid(file);

  if (status == Status::OK) {
    if (file->size() > (0x7FFF - addr + 1)) {
      TftUtil::show_error(m_tft, m_tch, "File is too large to fit!");
      status = Status::ERR_INVALID;
    }
    else {
      do_write_operation(file, addr);
    }

    file->flush();
    file->close();
    delete file;
  }

  return status;
}

void ProgrammerFileCore::do_write_operation(FileCtrl *file, uint16_t addr) {
  uint16_t cur_addr = addr;
  uint8_t this_page[256];

  m_tft.drawText(10, 10, "Writing file to EEPROM...", TftColor::CYAN, 3);

  TftProgressIndicator bar(m_tft, ceil((float) file->size() / 256.0), 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bar.for_each(
    [this, &this_page, &file, &cur_addr]TFT_PROGRESS_INDICATOR_LAMBDA {
      auto len = file->read(this_page, 0xFF);
      this->m_ee.write(cur_addr, this_page, MIN(len, 0xFF));

      cur_addr += 0x0100; // Next page

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, "Done writing!", TftColor::CYAN);
  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");
}

bool ProgrammerFileCore::get_file_to_write_from(char *out, uint8_t len, Status *res, FileSystem fsys) {
  switch(fsys) {
  case FileSystem::ON_SD_CARD: return sd_get_file_to_write_from(out, len, res);
  default:
    *res = Status::ERR_INVALID;
    return false;
  }
}

bool ProgrammerFileCore::sd_get_file_to_write_from(char *out, uint8_t len, Status *res) {
  TftSdFileSelMenu::Status temp = ask_file(m_tft, m_tch, m_sd, "File to write from?", out, 63);
  m_tft.fillScreen(TftColor::BLACK);

  if (temp == TftSdFileSelMenu::Status::CANCELED) {
    m_tft.drawText(10, 10, "Ok, canceled.", TftColor::CYAN, 3);
    *res = Status::OK;
  }
  else if (temp == TftSdFileSelMenu::Status::FNAME_TOO_LONG) {
    m_tft.drawText(10, 10, "File name was too long", TftColor::CYAN, 3);
    m_tft.drawText(10, 50, "to fit in the buffer.", TftColor::PURPLE, 2);
    *res = Status::ERR_FILE;
  }

  if (temp != TftSdFileSelMenu::Status::OK) {
    TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");
  }

  return temp == TftSdFileSelMenu::Status::OK;
}

Status ProgrammerFileCore::verify(uint16_t addr, void *data) {
  Status status = Status::OK;

  File file = SD.open((const char *) data, O_READ);

  uint8_t expectation[256];
  uint8_t reality[256];

  if (!file) return Status::ERR_FILE;

  m_tft.drawText(10, 10, STRFMT_NOBUF("Verifying `%s' at %04X...", (const char *) data, addr), TftColor::CYAN);

  TftProgressIndicator bar(m_tft, ceil((float) file.size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bool complete = bar.for_each(
    [this, &expectation, &reality, &file, &addr]TFT_PROGRESS_INDICATOR_LAMBDA {
      auto nbytes = file.read(expectation, 256);
      this->m_ee.read(addr, addr + nbytes, reality);

      if (memcmp(expectation, reality, nbytes) != 0) {
        this->m_tft.drawText(10, 150, STRFMT_NOBUF("Mismatch between %04X and %04X!", addr, addr + 0xFF), TftColor::RED);
        return true; // Request to quit loop
      }

      addr += 0x0100; // Next page

      return false;
    }
  );

  m_tft.drawText(10, 110, "Done verifying!", TftColor::CYAN);
  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

  // `complete` is true if the loop finished normally
  if (!complete) status = Status::ERR_VERIFY;

  file.close();
  m_tft.fillScreen(TftColor::BLACK);
  return status;
}

/*****************************/
/******** VECTOR CORE ********/
/*****************************/

Status ProgrammerVectorCore::read() {
  Vector vec = ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText( 10,  10, STRFMT_NOBUF("Value of %s vector:", Vector::NAMES[vec.m_id]), TftColor::CYAN,   3);
  m_tft.drawText(320,  50, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1),      TftColor::BLUE,   2);
  m_tft.drawText( 16,  50, STRFMT_NOBUF("HEX: %04X", vec.m_val),                         TftColor::YELLOW, 2);
  m_tft.drawText( 16,  80, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(vec.m_hi)),       TftColor::YELLOW, 2);
  m_tft.drawText( 16, 110, STRFMT_NOBUF(".... " BYTE_FMT, BYTE_FMT_VAL(vec.m_lo)),       TftColor::YELLOW, 2);

  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

Status ProgrammerVectorCore::write() {
  Vector vec = ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = ask_val<uint16_t>(m_tft, m_tch, "Type the new value:");
  m_ee.write(vec.m_addr,     new_val & 0xFF);
  m_ee.write(vec.m_addr + 1, new_val >> 8);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                                                 TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("value %04X", new_val),                     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                                    TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("vector %s.", Vector::NAMES[vec.m_id]),     TftColor::GREEN,  4);
  m_tft.drawText(10, 136, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1), TftColor::DGREEN, 2);

  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(vec.m_addr, (void *) &new_val)
}

Status ProgrammerVectorCore::verify(uint16_t addr, void *data) {
  uint16_t actual = (m_ee.read(addr + 1) << 8) | m_ee.read(addr);

  if (actual != *(uint16_t *) data) {
    m_tft.drawText(10, 10, "Mismatch found!",                                  TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %04X", *(uint16_t *) data), TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %04X", actual),             TftColor::MAGENTA, 3);

    TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

    m_tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/********************************/
/******** MULTIBYTE CORE ********/
/********************************/

Status ProgrammerMultiCore::read() {
  uint16_t addr1 = ask_addr(m_tft, m_tch, "Start address?");
  m_tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = ask_addr(m_tft, m_tch, "End address?");

  Util::validate_addrs(&addr1, &addr2);

  uint16_t nbytes = (addr2 - addr1 + 1);

  auto *data = (uint8_t *) malloc(nbytes * sizeof(uint8_t));
  if (data == nullptr) {
    m_tft.fillScreen(TftColor::BLACK);
    return Status::ERR_MEMORY;
  }

  read_with_progress_bar(data, addr1, addr2);

  m_tft.fillScreen(TftColor::BLACK);

  handle_data(data, addr1, addr2);

  m_tft.fillScreen(TftColor::BLACK);

  free(data);

  return Status::OK;
}

void ProgrammerMultiCore::read_with_progress_bar(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText(10, 10, "Working... Progress:", TftColor::CYAN, 3);

  TftProgressIndicator bar(m_tft, ceil((float) nbytes / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  uint16_t cur_addr_offset = 0x0000;

  bar.for_each(
    [this, &data, &cur_addr_offset, &addr1, &addr2]TFT_PROGRESS_INDICATOR_LAMBDA {
      uint16_t _addr1 = addr1 + cur_addr_offset;
      uint16_t _addr2 = MIN(_addr1 + 0xFF, addr2);

      this->m_ee.read(_addr1, _addr2, data + cur_addr_offset);

      cur_addr_offset += 0x0100; // Next page

      return this->m_tch.is_touching();
    }
  );

  m_tft.drawText(10, 110, "Done reading!", TftColor::CYAN);
  TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");
}

void ProgrammerMultiCore::handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2) {
  uint16_t nbytes = (addr2 - addr1 + 1);

  uint8_t viewing_method = ask_choice(
    m_tft, m_tch, "Select viewing method:", 1, 30, 0, 3,
    "Show as Raw Hexadecimal",   TftColor::BLACK,  TftColor::ORANGE,
    "Show Printable Characters", TftColor::LGREEN, TftColor::DGREEN,
    "Write Data to a File",      TftColor::CYAN,  TftColor::BLUE
  );

  m_tft.fillScreen(TftColor::BLACK);

  if      (viewing_method == 0) show_range(data, addr1, addr2, &multi_byte_repr_hex);
  else if (viewing_method == 1) show_range(data, addr1, addr2, &multi_byte_repr_chars);
  else if (viewing_method == 2) store_file(data, nbytes);
}

void ProgrammerMultiCore::show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr) {
  // Draw frame for the data
  m_tft.drawText(10, 10, STRFMT_NOBUF("%d bytes", addr2 - addr1 + 1), TftColor::CYAN, 3);
  m_tft.drawThickRect(m_tft.width() / 2 - 147, 55, 295, 166, TftColor::WHITE, 2);
  m_tft.drawFastVLine(m_tft.width() / 2, 57, 162, TftColor::GRAY);

  draw_page_axis_labels();

  TftMenu menu;
  menu.add_btn(new TftBtn(15,                 60, 40, 150, 15, 68, "\x11"));
  menu.add_btn(new TftBtn(m_tft.width() - 55, 60, 40, 150, 15, 68, "\x10"));
  menu.add_btn(new TftBtn(BOTTOM_BTN(m_tft, "Continue")));
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

void ProgrammerMultiCore::store_file(uint8_t *data, uint16_t len) {
  char fname[64];
  ask_str(m_tft, m_tch, "What filename?", fname, 63);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10, 10, "Please wait...", TftColor::PURPLE);

  File file = SD.open(fname, O_WRITE | O_TRUNC | O_CREAT);
  file.write(data, len);
  file.flush();
  file.close();
}

Status ProgrammerMultiCore::write() {
  AddrDataArray buf;

  uint16_t _y = TftCalc::bottom(m_tft, 24, 10);

  TftMenu menu;
  menu.add_btn(new TftBtn(10,                            74, 24, TftCalc::fraction_y(m_tft, 59, 1), "\x1e",     TftColor::WHITE, TftColor::DGRAY));
  menu.add_btn(new TftBtn(TftCalc::right(m_tft, 24, 10), 74, 24, TftCalc::fraction_y(m_tft, 59, 1), "\x1f",     TftColor::WHITE, TftColor::DGRAY));
  menu.add_btn(new TftBtn(10,                            40, TftCalc::fraction_x(m_tft, 10, 1), 24, "Add Pair", TftColor::WHITE, TftColor::PURPLE));
  menu.add_btn(new TftBtn(10,                            _y, TftCalc::fraction_x(m_tft, 10, 2), 24, "Done",     TftColor::RED,   TftColor::PINKK));
  menu.add_btn(new TftBtn(m_tft.width() / 2 + 5,         _y, TftCalc::fraction_x(m_tft, 10, 2), 24, "Cancel",   TftColor::CYAN,  TftColor::BLUE));

  TftMenu del_btns;

  const uint8_t num_pairs = 7;

  for (uint8_t i = 0; i < num_pairs; ++i) {
    del_btns.add_btn(new TftBtn(TftCalc::right(m_tft, 18, 44), 76 + 30 * i, 18, 18, "x", TftColor::YELLOW, TftColor::RED));
  }

  uint16_t scroll = 0;

  bool done = false;

  while (!done) {
    m_tft.drawText(10, 10, "Write Multiple Bytes", TftColor::CYAN, 3);

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
  uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, TftMenu &del_btns
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
  TftMenu &menu, TftMenu &del_btns, AddrDataArray *buf, uint16_t *scroll, const uint16_t max_scroll
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
        m_tft.drawText(10, 12, "Please wait - accessing EEPROM...", TftColor::CYAN, 2);
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
  auto addr = ask_addr(m_tft, m_tch, "Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  auto data = ask_val<uint8_t>(m_tft, m_tch, "Type the data:");
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
      m_tft.drawText(10,  10, "Mismatch found!",                         TftColor::CYAN,    3);
      m_tft.drawText(15,  50, STRFMT_NOBUF("Expected: %02X", pair.data), TftColor::PURPLE,  3);
      m_tft.drawText(15,  77, STRFMT_NOBUF("Actual:   %02X", real_data), TftColor::MAGENTA, 3);
      m_tft.drawText(15, 104, STRFMT_NOBUF("Address: %04X",  pair.addr), TftColor::ORANGE,  2);

      TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");

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

  TftMenu menu;
  menu.add_btn(new TftBtn(     10,  50, w2, 28, "WE Hi (Disable)", TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN));
  menu.add_btn(new TftBtn(w2 + 20,  50, w2, 28, "WE Lo (Enable)",  TftColor::PINKK,          TftColor::RED   ));
  menu.add_btn(new TftBtn(     10,  88, w1, 28, "Set Address/OE",  TftColor::BLACK,          TftColor::YELLOW));
  menu.add_btn(new TftBtn(     10, 126, w2, 28, "Read Data Bus",   TftColor::BLUE,           TftColor::CYAN  ));
  menu.add_btn(new TftBtn(w2 + 20, 126, w2, 28, "Write Data Bus",  TftColor::CYAN,           TftColor::BLUE  ));
  menu.add_btn(new TftBtn(     10, 164, w2, 28, "Set Data Dir",    TftColor::BLACK,          TftColor::ORANGE));
  menu.add_btn(new TftBtn(w2 + 20, 164, w2, 28, "Monitor Data",    TftColor::YELLOW,         TftColor::DCYAN ));
  menu.add_btn(new TftBtn(     10, 202, w2, 28, "Print Charset",   TftColor::PINKK,          TftColor::PURPLE));
  menu.add_btn(new TftBtn(w2 + 20, 202, w2, 28, "Show Colors",     TftColor::PINKK,          TftColor::PURPLE));
  menu.add_btn(new TftBtn(     10, 240, w2, 28, "Aux1",            TftColor::DGRAY,          TftColor::LGRAY ));
  menu.add_btn(new TftBtn(w2 + 20, 240, w2, 28, "Aux2",            TftColor::DGRAY,          TftColor::LGRAY ));
  menu.add_btn(new TftBtn(BOTTOM_BTN(m_tft, "Close")));

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
      ask_val<uint16_t>(m_tft, m_tch, "Type the value:")
    );
  }
  else if (action == DebugAction::READ_DATA_BUS) {
    m_tft.drawText(10, 10, "Value:", TftColor::CYAN, 4);
    m_tft.drawText(10, 50, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(m_ee.get_data())), TftColor::YELLOW, 2);

    TftUtil::wait_bottom_btn(m_tft, m_tch, "Continue");
  }
  else if (action == DebugAction::WRITE_DATA_BUS) {
    m_ee.set_data(
      ask_val<uint8_t>(m_tft, m_tch, "Type the value:")
    );
  }
  else if (action == DebugAction::SET_DATA_DIR) {
    m_ee.set_ddr(
      ask_choice(
        m_tft, m_tch, "Which direction?", 1, 45, 0, 2,
        "Input",        TftColor::CYAN,   TftColor::BLUE,
        "Output",       TftColor::PINKK,  TftColor::RED
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

  TftBtn quit_btn(BOTTOM_BTN(m_tft, "Quit"));
  quit_btn.draw(m_tft);

#ifdef DEBUG_MODE
  while (!quit_btn.is_pressed(m_tch, m_tft)) {
    uint8_t val = m_ee.get_io_exp(true)->read_port(MCP_EE_DATA_PORT);
    m_tft.drawTextBg(10, 10, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(val)), TftColor::CYAN, TftColor::BLACK, 3);
    delay(500);
  }
#else
  // EepromCtrl::get_io_exp() only exists in DEBUG_MODE

  m_tft.drawText(10,  10, "Error:",                 TftColor::RED,    3);
  m_tft.drawText(10,  50, "Data bus monitor",       TftColor::ORANGE, 3);
  m_tft.drawText(10,  90, "is not supported",       TftColor::ORANGE, 3);
  m_tft.drawText(10, 130, "(DEBUG_MODE disabled!)", TftColor::PURPLE, 2);

  quit_btn.wait_for_press(m_tch, m_tft);
#endif
}

void ProgrammerOtherCore::debug_action_aux1() {
  uint8_t temp[] = {
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

  TftUtil::wait_bottom_btn(m_tft, m_tch, "OK");

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

uint16_t ask_addr(TftCtrl &tft, TouchCtrl &tch, const char *prompt) {
  uint16_t addr = ask_val<uint16_t>(tft, tch, prompt);
  Util::validate_addr(&addr);

  return addr;
}

FileSystem ask_fsys(TftCtrl &tft, TouchCtrl &tch, const char *prompt, SdCtrl &sd) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  TftChoiceMenu menu(10, 10, 50, 10, 1, 40, true, 0);
  menu.add_btn_calc(tft, "SD Card File", TftColor::LGREEN, TftColor::DGREEN);
  menu.add_btn_calc(tft, "Serial File",  TftColor::CYAN,   TftColor::BLUE  );
  menu.add_btn_calc(tft, "Cancel",       TftColor::PINKK,  TftColor::DRED  );
  menu.add_btn_confirm(tft, true);

  uint8_t avail = FileUtil::get_available_file_systems(sd);

  if (~avail & FileSystem::ON_SD_CARD) menu.get_btn(0)->operation(false);
  if (~avail & FileSystem::ON_SERIAL)  menu.get_btn(1)->operation(false);

  uint8_t btn = menu.wait_for_value(tch, tft);
  uint8_t btn_cancel = menu.get_num_btns() - 2;

  if (btn != btn_cancel) {
    switch (btn) {
    case 0: return FileSystem::ON_SD_CARD;
    case 1: return FileSystem::ON_SERIAL;
    }
  }

  return FileSystem::NONE;
}

#undef RETURN_VERIFICATION_OR_VALUE
#undef RETURN_VERIFICATION_OR_OK
