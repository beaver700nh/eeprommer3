#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "ad_array.hpp"
#include "debug.hpp"
#include "eeprom.hpp"
#include "input.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

void wait_continue(TftCtrl &tft, TouchCtrl &tch) {
  static TftBtn continue_btn(BOTTOM_BTN(tft, "Continue"));
  continue_btn.draw(tft);
  continue_btn.wait_for_press(tch, tft);
}

ProgrammerFromSd::ProgrammerFromSd(EepromCtrl &ee, SdCtrl &sd, TouchCtrl &tch, TftCtrl &tft)
  : m_ee(ee), m_sd(sd), m_tch(tch), m_tft(tft) {
  // Empty
}

void show_help(TftCtrl &tft, uint8_t btn_id, bool is_confirm) {
  if (is_confirm) return;

  static const char *helps[] = {
    "Read a byte from EEPROM.",
    "Write a byte to EEPROM.",
    "Read entire EEPROM to a file.",
    "Write file to EEPROM somewhere.",
    "Read 6502 jump vector contents.",
    "Write to a 6502 jump vector.",
    "Read multiple bytes from EEPROM.",
    "Write multiple bytes to EEPROM.",
  };

  tft.fillRect(10, 250, tft.width(), 16, TftColor::BLACK);
  tft.drawText(10, 250, (btn_id < ARR_LEN(helps) ? helps[btn_id] : "No help text available."), TftColor::PURPLE, 2);
}

void ProgrammerFromSd::run() {
  TftChoiceMenu menu(10, 10, 50, 10, 2, 30, true);
  menu.add_btn_calc(m_tft, "Read Byte",       TftColor::BLUE,           TftColor::CYAN);
  menu.add_btn_calc(m_tft, "Write Byte",      TftColor::RED,            TftColor::PINKK);
  menu.add_btn_calc(m_tft, "Read to File",    TftColor::CYAN,           TftColor::BLUE);
  menu.add_btn_calc(m_tft, "Write from File", TftColor::PINKK,          TftColor::RED);
  menu.add_btn_calc(m_tft, "Read Vector",     TO_565(0x00, 0x17, 0x00), TftColor::LGREEN);
  menu.add_btn_calc(m_tft, "Write Vector",    TO_565(0x3F, 0x2F, 0x03), TO_565(0xFF, 0xEB, 0x52));
  menu.add_btn_calc(m_tft, "Read Range",      TftColor::LGREEN,         TftColor::DGREEN);
  menu.add_btn_calc(m_tft, "Write Multiple",  TftColor::BLACK,          TftColor::ORANGE);
  menu.add_btn_calc(m_tft, "Draw Test",       TftColor::DGRAY,          TftColor::GRAY);
  menu.add_btn_calc(m_tft, "Debug Tools",     TftColor::DGRAY,          TftColor::GRAY);
  menu.add_btn_confirm(m_tft, true);

  menu.set_callback(show_help);

  uint8_t cur_choice = 0;

  while (true) { // Main loop
    m_tft.drawText(10, 10, "Choose an action:", TftColor::CYAN, 3);
    show_help(m_tft, cur_choice, false);
    cur_choice = menu.wait_for_value(m_tch, m_tft);

    m_tft.fillScreen(TftColor::BLACK);

    uint8_t status_code = (cur_choice < NUM_ACTIONS ? (this->*(action_map[cur_choice]))() : 1);
    show_status(status_code);

    wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);
  }
}

void ProgrammerFromSd::show_status(uint8_t status_code) {
  m_tft.drawText(10, 10, "Result:", TftColor::CYAN, 4);

  const char *details_buf[] = {
    "No errors.",
    "Invalid action.",
    "Failed to open file.",
    "Verification failed.",
    "Failed to allocate memory.",
  };

  m_tft.drawText(15, 50, (status_code ? "Failed!" : "Success!"), (status_code ? TftColor::RED : TftColor::GREEN), 3);
  m_tft.drawText(15, 80, (status_code < 5 ? details_buf[status_code] : "Unknown reason."), TftColor::PURPLE, 2);
}

uint8_t ProgrammerFromSd::read_byte() {
  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Type an address:");
  uint8_t data = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, STRFMT_NOBUF("Value at address %04X:", addr),       TftColor::CYAN,   3);
  m_tft.drawText(20,  50, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(data)), TftColor::YELLOW, 2);
  m_tft.drawText(20,  80, STRFMT_NOBUF("OCT: %03o",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 110, STRFMT_NOBUF("HEX: %02X",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 140, STRFMT_NOBUF("DEC: %-3d",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 170, STRFMT_NOBUF("CHR: %c",        data),               TftColor::YELLOW, 2);

  wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::write_byte() {
  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  uint8_t data = ask_val<uint8_t>(m_tft, m_tch, "Type the data:");

  m_ee.write(addr, data);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                             TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("data %02X", data),     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("address %04X.", addr), TftColor::GREEN,  4);

  wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?");
  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify) {
    return verify_byte(addr, data);
  }

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::verify_byte(uint16_t addr, uint8_t data) {
  uint8_t actual = m_ee.read(addr);

  if (actual != data) {
    m_tft.drawText(10, 10, "Result:",                              TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %02X", data),   TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %02X", actual), TftColor::MAGENTA, 3);

    wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);

    return STATUS_ERR_VERIFY;
  }

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::read_file() {
  ActionFuncStatus status = STATUS_OK;

  char fname[64];
  ask_str(m_tft, m_tch, "File to read to?", fname, 63);

  m_tft.fillScreen(TftColor::BLACK);

  uint8_t this_page[256];
  File file = SD.open(fname, O_CREAT | O_WRITE | O_TRUNC);

  if (!file) {
    status = STATUS_ERR_FILE;
  }
  else {
    m_tft.drawText(10, 10, "Working... Progress:", TftColor::CYAN, 3);

    TftProgressIndicator bar(m_tft, 127, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

    bar.for_each(
      [this, &this_page, &file]TFT_PROGRESS_INDICATOR_LAMBDA {
        uint16_t addr = progress * 0x0100;

        this->m_ee.read(addr, addr + 0xFF, this_page);
        file.write(this_page, 256);

        return this->m_tch.is_touching();
      }
    );

    m_tft.drawText(10, 110, "Done reading!", TftColor::CYAN);
    wait_continue(m_tft, m_tch);
  }

  file.flush();
  file.close();
  m_tft.fillScreen(TftColor::BLACK);
  return status;
}

uint8_t ProgrammerFromSd::write_file() {
  ActionFuncStatus status = STATUS_OK;

  char fname[64];
  ask_file(m_tft, m_tch, m_sd, "File to write from?", fname, 63);
  m_tft.fillScreen(TftColor::BLACK);

  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Where to write in EEPROM?");
  uint16_t cur_addr = addr;
  m_tft.fillScreen(TftColor::BLACK);

  uint8_t this_page[256];
  File file = SD.open(fname, O_READ);

  if (!file) {
    status = STATUS_ERR_FILE;
  }
  else {
    m_tft.drawText(10, 10, "Working... Progress:", TftColor::CYAN, 3);

    TftProgressIndicator bar(m_tft, ceil((float) file.size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

    bar.for_each(
      [this, &this_page, &file, &cur_addr]TFT_PROGRESS_INDICATOR_LAMBDA {
        auto len = file.read(this_page, 256);
        this->m_ee.write(cur_addr, this_page, MIN(len, 256));

        cur_addr += 0x0100; // Next page

        return this->m_tch.is_touching();
      }
    );

    m_tft.drawText(10, 110, "Done writing!", TftColor::CYAN);
    wait_continue(m_tft, m_tch);
  }

  file.close();
  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?");
  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify) {
    return verify_file(fname, addr);
  }

  return status;
}

uint8_t ProgrammerFromSd::verify_file(const char *fname, uint16_t addr) {
  ActionFuncStatus status = STATUS_OK;

  File file = SD.open(fname, O_READ);

  uint8_t expectation[256];
  uint8_t reality[256];

  if (!file) return STATUS_ERR_FILE;

  m_tft.drawText(10, 10, STRFMT_NOBUF("Verifying `%s' at %04X...", fname, addr), TftColor::CYAN);

  TftProgressIndicator bar(m_tft, ceil((float) file.size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bool complete = bar.for_each(
    [this, &expectation, &reality, &file, &addr]TFT_PROGRESS_INDICATOR_LAMBDA {
      auto nbytes = file.read(expectation, 256);
      this->m_ee.read(addr, addr + nbytes, reality);

      if (memcmp(expectation, reality, nbytes) != 0) {
        this->m_tft.drawText(10, 110, STRFMT_NOBUF("Mismatch between %04X and %04X!", addr, addr + 0xFF), TftColor::RED);

        // Request to quit loop
        return true;
      }

      addr += 0x0100; // Next page

      return false;
    }
  );

  wait_continue(m_tft, m_tch);

  // `complete` is true if the loop finished normally
  if (!complete) status = STATUS_ERR_VERIFY;

  file.close();
  m_tft.fillScreen(TftColor::BLACK);
  return status;
}

uint8_t ProgrammerFromSd::read_vector() {
  Vector vec = ask_vector();
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText( 10,  10, STRFMT_NOBUF("Value of %s vector:", Vector::NAMES[vec.m_id]), TftColor::CYAN,   3);
  m_tft.drawText(320,  50, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1),      TftColor::BLUE,   2);
  m_tft.drawText( 16,  50, STRFMT_NOBUF("HEX: %04X", vec.m_val),                         TftColor::YELLOW, 2);
  m_tft.drawText( 16,  80, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(vec.m_hi)),       TftColor::YELLOW, 2);
  m_tft.drawText( 16, 110, STRFMT_NOBUF(".... " BYTE_FMT, BYTE_FMT_VAL(vec.m_lo)),       TftColor::YELLOW, 2);

  wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::write_vector() {
  Vector vec = ask_vector();
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

  wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?");
  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify) {
    return verify_vector(vec.m_addr, new_val);
  }

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::verify_vector(uint16_t addr, uint16_t data) {
  uint16_t actual = (m_ee.read(addr + 1) << 8) | m_ee.read(addr);

  if (actual != data) {
    m_tft.drawText(10, 10, "Result:",                              TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %04X", data),   TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %04X", actual), TftColor::MAGENTA, 3);

    wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);

    return STATUS_ERR_VERIFY;
  }

  return STATUS_OK;
}

Vector ProgrammerFromSd::ask_vector() {
  return Vector(
    ask_choice(
      m_tft, m_tch, "Which vector?", 3, 54, 1, 3,
      Vector::NAMES[0], TftColor::CYAN,           TftColor::BLUE,
      Vector::NAMES[1], TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN,
      Vector::NAMES[2], TftColor::PINKK,          TftColor::RED
    )
  );
}

uint8_t ProgrammerFromSd::read_range() {
  uint16_t addr1 = ask_val<uint16_t>(m_tft, m_tch, "Start address?");
  m_tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = ask_val<uint16_t>(m_tft, m_tch, "End address?");
  uint16_t nbytes = (addr2 - addr1 + 1);

  // Turn off top bit of both addresses ensure validity
  addr1 &= ~0x8000;
  addr2 &= ~0x8000;

  // Make sure addr1 <= addr2
  if (addr2 < addr1) Util::swap<uint16_t>(&addr1, &addr2);

  m_tft.drawText(10, 252, "Please wait - accessing EEPROM...", TftColor::PURPLE, 2);

  auto *data = (uint8_t *) malloc(nbytes * sizeof(uint8_t));
  if (data == nullptr) {
    m_tft.fillScreen(TftColor::BLACK);
    return STATUS_ERR_MEMORY;
  }

  m_ee.read(addr1, addr2, data);

#ifdef DEBUG_MODE
  PRINTF_NOBUF(Serial, "ProgrammerFromSd::read_range(): reading range {%d..%d}\n", addr1, addr2);
  debug_print_addr_range(addr1, addr2, data);
#endif

  m_tft.fillScreen(TftColor::BLACK);

  uint8_t viewing_method = ask_choice(
    m_tft, m_tch, "Select viewing method:", 1, 30, 0, 3,
    "Show as Raw Hexadecimal",   TftColor::BLACK,  TftColor::ORANGE,
    "Show Printable Characters", TftColor::LGREEN, TftColor::DGREEN,
    "Write Data to a File",      TftColor::CYAN,  TftColor::BLUE
  );

  m_tft.fillScreen(TftColor::BLACK);

  if      (viewing_method == 0) show_range(data, addr1, addr2, &ProgrammerFromSd::calc_hex);
  else if (viewing_method == 1) show_range(data, addr1, addr2, &ProgrammerFromSd::calc_chars);
  else if (viewing_method == 2) store_file(data, nbytes);

  m_tft.fillScreen(TftColor::BLACK);

#ifdef DEBUG_MODE
  Serial.println();
#endif

  free(data);

  return STATUS_OK;
}

// Assumes addr1 <= addr2
void ProgrammerFromSd::show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ProgrammerFromSd::calc_func calc) {
  // Draw frame for the data
  m_tft.drawText(10, 10, STRFMT_NOBUF("%d bytes", addr2 - addr1 + 1), TftColor::CYAN, 3);
  m_tft.drawRect(m_tft.width() / 2 - 147, 50, 295, 166, TftColor::WHITE);
  m_tft.drawRect(m_tft.width() / 2 - 146, 51, 293, 164, TftColor::WHITE);
  m_tft.drawFastVLine(m_tft.width() / 2, 52, 162, TftColor::GRAY);

  TftMenu menu;
  menu.add_btn(new TftBtn(15,                 60, 40, 150, 15, 68, "\x11"));
  menu.add_btn(new TftBtn(m_tft.width() - 55, 60, 40, 150, 15, 68, "\x10"));
  menu.add_btn(new TftBtn(BOTTOM_BTN(m_tft, "Continue")));
  menu.draw(m_tft);

#ifdef DEBUG_MODE
  PRINTF_NOBUF(Serial, "ProgrammerFromSd::show_range(): showing range {%d..%d}\n\n", addr1, addr2);
#endif

  uint8_t cur_page = 0;
  uint8_t max_page = (addr2 >> 8) - (addr1 >> 8);

  while (true) {
    show_page(data, addr1, addr2, calc, cur_page, max_page);

    uint8_t req = menu.wait_for_press(m_tch, m_tft);

    if      (req == 2) break;
    else if (req == 0) cur_page = (cur_page == 0 ? max_page : cur_page - 1);
    else if (req == 1) cur_page = (cur_page == max_page ? 0 : cur_page + 1);
  }
}

// Helper function of show_range() that shows
// only one page of data from a large buffer
void ProgrammerFromSd::show_page(
  uint8_t *data, uint16_t addr1, uint16_t addr2, ProgrammerFromSd::calc_func calc, uint8_t cur_page, uint8_t max_page
) {
#ifdef DEBUG_MODE
  PRINTF_NOBUF(
    Serial,
    "ProgrammerFromSd::show_page(): showing page #%d out of %d pages, from range {%d..%d}\n",
    cur_page, max_page + 1, addr1, addr2
  );
#endif

  m_tft.fillRect(m_tft.width() / 2 - 145, 52, 145, 162, TftColor::DGRAY);
  m_tft.fillRect(m_tft.width() / 2 +   1, 52, 145, 162, TftColor::DGRAY);

  m_tft.drawTextBg(
    10, 224, STRFMT_NOBUF("Page #%d of %d", cur_page, max_page),
    TftColor::PURPLE, TftColor::BLACK, 2
  );

  // Variables for formatting a byte
  uint8_t byte_offset;
  char byte_as_text[3];
  uint16_t color;

  uint16_t glob_range_start = addr1 >> 8;
  uint16_t glob_page_start = MAX(((cur_page + glob_range_start)     << 8),     addr1);
  uint16_t glob_page_end   = MIN(((cur_page + glob_range_start + 1) << 8) - 1, addr2);

  for (uint16_t i = glob_page_start; i <= glob_page_end; ++i) {
    uint8_t tft_byte_col = (i & 0x0F);
    uint8_t tft_byte_row = (i & 0xFF) >> 4;

    (*calc)(&byte_offset, byte_as_text, &color, data[i - addr1]);

    uint8_t split_offset = (tft_byte_col < 8 ? 0 : 3);
    uint16_t tft_byte_x = m_tft.width()  / 2 - 141 + 18 * tft_byte_col + byte_offset + split_offset;
    uint16_t tft_byte_y = m_tft.height() / 2 - 105 + 10 * tft_byte_row;

    m_tft.drawText(tft_byte_x, tft_byte_y, byte_as_text, color, 1);
  }
}

// Hex mode shows the data as raw hexadecimal values in white
void ProgrammerFromSd::calc_hex(uint8_t *offset, char *text, uint16_t *color, uint8_t data) {
  *offset = 0;
  *color = TftColor::WHITE;

  sprintf(text, "%02X", data);
}

// Chars mode shows the data as printable characters
// White character if printable, gray "?" if not
void ProgrammerFromSd::calc_chars(uint8_t *offset, char *text, uint16_t *color, uint8_t data) {
  *offset = 3;
  *color = (isprint((char) data) ? TftColor::WHITE : TftColor::GRAY);

  sprintf(text, "%c", (isprint((char) data) ? (char) data : '?'));
}

void ProgrammerFromSd::store_file(uint8_t *data, uint16_t len) {
  char fname[64];
  ask_str(m_tft, m_tch, "What filename?", fname, 63);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10, 10, "Please wait...", TftColor::PURPLE);

  File file = SD.open(fname, O_WRITE | O_TRUNC | O_CREAT);
  file.write(data, len);
  file.flush();
  file.close();
}

uint8_t ProgrammerFromSd::write_multi() {
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

#ifdef DEBUG_MODE
    PRINTF_NOBUF(Serial, "Scroll: %d out of maximum %d\n", scroll, max_scroll);
#endif

    done = poll_menus_and_react(menu, del_btns, &buf, &scroll, max_scroll);
  }

  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?");
  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify) {
    return verify_multi(buf);
  }

  return STATUS_OK;
}

void ProgrammerFromSd::draw_pairs(
  uint16_t margin_l, uint16_t margin_r, uint16_t margin_u, uint16_t margin_d,
  uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, TftMenu &del_btns
) {
  // Clear the pairs area
  m_tft.fillRect(margin_l, margin_u, TftCalc::right(m_tft, margin_l, margin_r), TftCalc::bottom(m_tft, margin_u, margin_d), TftColor::BLACK);

  for (uint8_t i = 0; i < n; ++i) {
    // Hide all the delete buttons here, later show the ones that are needed
    auto cur_btn = del_btns.get_btn(i - offset);
    cur_btn->visibility(false);
    cur_btn->operation(false);
  }

  if (buf.get_len() == 0) {
    m_tft.drawText(margin_l, margin_u,      "No pairs yet!",                     TftColor::LGRAY);
    m_tft.drawText(margin_l, margin_u + 30, "Click \"Add Pair\" to add a pair!", TftColor::LGRAY);
    return;
  }

  uint16_t this_pair = offset;
  uint16_t last_pair = MIN(offset + n - 1, buf.get_len() - 1);

  do {
    uint16_t x = margin_l;
    uint16_t y = margin_u + (this_pair - offset) * (height + padding);
    uint16_t w = TftCalc::right(m_tft, margin_l, margin_r);
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

bool ProgrammerFromSd::poll_menus_and_react(TftMenu &menu, TftMenu &del_btns, AddrDataArray *buf, uint16_t *scroll, const uint16_t max_scroll) {
  int16_t pressed;
  int16_t deleted;

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
      case 2:  add_pair_from_user(buf);             break;
      case 3:
        m_tft.fillRect(10, 10, TftCalc::fraction_x(m_tft, 10, 1), 24, TftColor::BLACK);
        m_tft.drawText(10, 12, "Please wait - accessing EEPROM...", TftColor::CYAN, 2);
        m_ee.write(buf);
        // fall through to next case
      default: return true;
      }

      break;
    }
  }

  return false;
}

void ProgrammerFromSd::add_pair_from_user(AddrDataArray *buf) {
  m_tft.fillScreen(TftColor::BLACK);
  auto addr = ask_val<uint16_t>(m_tft, m_tch, "Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  auto data = ask_val<uint8_t>(m_tft, m_tch, "Type the data:");
  m_tft.fillScreen(TftColor::BLACK);

  buf->append((AddrDataArrayPair) {addr, data});
}

uint8_t ProgrammerFromSd::verify_multi(AddrDataArray &buf) {
  for (uint16_t i = 0; i < buf.get_len(); ++i) {
    AddrDataArrayPair pair;
    buf.get_pair(i, &pair);

    uint8_t real_data = m_ee.read(pair.addr);

#ifdef DEBUG_MODE
    PRINTF_NOBUF(Serial, "ProgrammerFromSd::verify_multi: addr %04X; expectation %02X, reality %02X.\n", pair.addr, pair.data, real_data);
#endif

    if (pair.data != real_data) return STATUS_ERR_VERIFY;
  }

  return STATUS_OK;
}

uint8_t ProgrammerFromSd::draw() {
  tft_draw_test(m_tch, m_tft);

  // This return will never happen because tft_draw_test() contains an infinite loop
  return STATUS_OK;
}

uint8_t ProgrammerFromSd::debug() {
  auto w1 = TftCalc::fraction_x(m_tft, 10, 1);
  auto w2 = TftCalc::fraction_x(m_tft, 10, 2);

  TftMenu menu;
  menu.add_btn(new TftBtn(     10,  50, w2, 30, "WE HI (Disable)", TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN));
  menu.add_btn(new TftBtn(w2 + 20,  50, w2, 30, "WE LO (Enable)",  TftColor::PINKK,          TftColor::RED));
  menu.add_btn(new TftBtn(     10,  90, w1, 30, "Set Address/OE",  TftColor::BLACK,          TftColor::YELLOW));
  menu.add_btn(new TftBtn(     10, 130, w2, 30, "Read Data Bus",   TftColor::BLUE,           TftColor::CYAN));
  menu.add_btn(new TftBtn(w2 + 20, 130, w2, 30, "Write Data Bus",  TftColor::CYAN,           TftColor::BLUE));
  menu.add_btn(new TftBtn(     10, 170, w1, 30, "Print Charset",   TftColor::PINKK,          TftColor::PURPLE));
  menu.add_btn(new TftBtn(BOTTOM_BTN(m_tft, "Close")));

  m_tft.drawText(10, 10, "Debug Tools Menu", TftColor::CYAN, 4);
  menu.draw(m_tft);

  while (true) {
    uint8_t btn = menu.wait_for_press(m_tch, m_tft);

    if (btn == menu.get_num_btns() - 1) break;

    uint16_t val16;
    uint8_t val8;

    switch (btn) {
    case 0: m_ee.set_we(true);  continue;
    case 1: m_ee.set_we(false); continue;
    case 2:
      m_tft.fillScreen(TftColor::BLACK); 
      val16 = ask_val<uint16_t>(m_tft, m_tch, "Type the value:");
      m_ee.set_addr_and_oe(val16);
      break;
    case 3:
      m_tft.fillScreen(TftColor::BLACK); 
      val8 = m_ee.get_data();
      m_tft.drawText(10, 10, "Value:", TftColor::CYAN, 4);
      m_tft.drawText(10, 50, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(val8)), TftColor::YELLOW, 2);
      wait_continue(m_tft, m_tch);
      break;
    case 4:
      m_tft.fillScreen(TftColor::BLACK); 
      val8 = ask_val<uint8_t>(m_tft, m_tch, "Type the value:");
      m_ee.set_data(val8);
      break;
    case 5:
      m_tft.fillScreen(TftColor::BLACK);
#ifdef DEBUG_MODE
      tft_print_chars(m_tft);
#endif
      // Wait for press
      while (!m_tch.is_touching());
      break;
    }

    m_tft.fillScreen(TftColor::BLACK);
    m_tft.drawText(10, 10, "Debug Tools Menu", TftColor::CYAN, 4);
    menu.draw(m_tft);
  }

  m_tft.fillScreen(TftColor::BLACK);

  return STATUS_OK;
}

// Dummy function for unimplemented actions
uint8_t ProgrammerFromSd::nop() {
  // Always returns 0 for success
  return STATUS_OK;
}
