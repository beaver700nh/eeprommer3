#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "ad_array.hpp"
#include "eeprom.hpp"
#include "input.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

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

  tft.fillRect(10, 250, 470, 16, TftColor::BLACK);
  tft.drawText(10, 250, (btn_id < ARR_LEN(helps) ? helps[btn_id] : "No help text available."), TftColor::PURPLE, 2);
}

void ProgrammerFromSd::run() {
  TftChoiceMenu menu(50, 10, 10, 10, 2, 30);
  menu.add_btn_calc(m_tft, "Read Byte",      TftColor::BLUE,           TftColor::CYAN);
  menu.add_btn_calc(m_tft, "Write Byte",     TftColor::RED,            TftColor::PINKK);
  menu.add_btn_calc(m_tft, "Read to File",   TftColor::CYAN,           TftColor::BLUE);
  menu.add_btn_calc(m_tft, "Write File",     TftColor::PINKK,          TftColor::RED);
  menu.add_btn_calc(m_tft, "Read Vector",    TO_565(0x00, 0x17, 0x00), TO_565(0x7F, 0xFF, 0x7F));
  menu.add_btn_calc(m_tft, "Write Vector",   TO_565(0x3F, 0x2F, 0x03), TO_565(0xFF, 0xEB, 0x52));
  menu.add_btn_calc(m_tft, "Read Range",     TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN);
  menu.add_btn_calc(m_tft, "Write Multiple", TftColor::BLACK,          TftColor::ORANGE);
  menu.add_btn_calc(m_tft, "Draw Test",      TftColor::DGRAY,          TftColor::GRAY);
  menu.add_btn_calc(m_tft, "Debug Tools",    TftColor::DGRAY,          TftColor::GRAY);
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

    wait_continue();

    m_tft.fillScreen(TftColor::BLACK);
  }
}

void ProgrammerFromSd::wait_continue() {
  static TftBtn continue_btn(BOTTOM_BTN(m_tft, "Continue"));
  continue_btn.draw(m_tft);
  continue_btn.wait_for_press(m_tch, m_tft);
}

void ProgrammerFromSd::show_status(uint8_t status_code) {
  m_tft.drawText(10, 10, "Result:", TftColor::CYAN, 4);

  const char *details_buf[] = {
    "No errors.",
    "Invalid action.",
    "Failed to open file.",
    "Verification failed.",
  };

  m_tft.drawText(15, 50, (status_code ? "Failed!" : "Success!"), (status_code ? TftColor::RED : TftColor::GREEN), 3);
  m_tft.drawText(15, 80, (status_code < 4 ? details_buf[status_code] : "Unknown reason."), TftColor::PURPLE, 2);
}

uint8_t ProgrammerFromSd::read_byte() {
  uint16_t addr = ask_val<uint16_t>("Type an address:");
  uint8_t data = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, STRFMT_NOBUF("Value at address %04X:", addr),       TftColor::CYAN,   3);
  m_tft.drawText(20,  50, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(data)), TftColor::YELLOW, 2);
  m_tft.drawText(20,  80, STRFMT_NOBUF("OCT: %03o",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 110, STRFMT_NOBUF("HEX: %02X",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 140, STRFMT_NOBUF("DEC: %-3d",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 170, STRFMT_NOBUF("CHR: %c",        data),               TftColor::YELLOW, 2);

  wait_continue();

  m_tft.fillScreen(TftColor::BLACK);

  return 0;
}

uint8_t ProgrammerFromSd::write_byte() {
  uint16_t addr = ask_val<uint16_t>("Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  uint8_t data = ask_val<uint8_t>("Type the data:");

  m_ee.write(addr, data);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                             TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("data %02X", data),     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("address %04X.", addr), TftColor::GREEN,  4);

  wait_continue();

  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  m_tft.drawText(10, 10, "Verify data?", TftColor::CYAN, 4);

  TftYesNoMenu vrf_menu(m_tft, 50, 10, 10, 10, true, 0);
  uint8_t should_verify = vrf_menu.wait_for_value(m_tch, m_tft);

  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify == 0) {
    return verify_byte(addr, data);
  }

  return 0;
}

uint8_t ProgrammerFromSd::verify_byte(uint16_t addr, uint8_t data) {
  uint8_t actual = m_ee.read(addr);

  if (actual != data) {
    m_tft.drawText(10, 10, "Result:",                              TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %02X", data),   TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %02X", actual), TftColor::MAGENTA, 3);

    wait_continue();

    m_tft.fillScreen(TftColor::BLACK);

    return 3;
  }

  return 0;
}

// Helper function to ask the user to choose
// one of the 6502's three jump vectors
Vector ProgrammerFromSd::ask_vector() {
  m_tft.drawText(10, 10, "Select which vector:", TftColor::CYAN, 3);

  TftChoiceMenu menu(50, 10, 10, 10, 3, 54, 1);
  menu.add_btn_calc(m_tft, Vector::NAMES[0], TftColor::CYAN,           TftColor::BLUE);
  menu.add_btn_calc(m_tft, Vector::NAMES[1], TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN);
  menu.add_btn_calc(m_tft, Vector::NAMES[2], TftColor::PINKK,          TftColor::RED);
  menu.add_btn_confirm(m_tft, true);

  uint8_t vector_id = menu.wait_for_value(m_tch, m_tft);

  return Vector(vector_id);
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

  wait_continue();

  m_tft.fillScreen(TftColor::BLACK);

  return 0;
}

uint8_t ProgrammerFromSd::write_vector() {
  Vector vec = ask_vector();
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = ask_val<uint16_t>("Type the new value:");
  m_ee.write(vec.m_addr,     new_val & 0xFF);
  m_ee.write(vec.m_addr + 1, new_val >> 8);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                                                 TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("value %04X", new_val),                     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                                    TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("vector %s.", Vector::NAMES[vec.m_id]),     TftColor::GREEN,  4);
  m_tft.drawText(10, 136, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1), TftColor::DGREEN, 2);

  wait_continue();

  m_tft.fillScreen(TftColor::BLACK);

  // Done writing, ask to verify
  m_tft.drawText(10, 10, "Verify data?", TftColor::CYAN, 4);

  TftYesNoMenu vrf_menu(m_tft, 50, 10, 10, 10, true, 0);
  uint8_t should_verify = vrf_menu.wait_for_value(m_tch, m_tft);

  m_tft.fillScreen(TftColor::BLACK);

  if (should_verify == 0) {
    return verify_vector(vec.m_addr, new_val);
  }

  return 0;
}

uint8_t ProgrammerFromSd::verify_vector(uint16_t addr, uint16_t data) {
  uint16_t actual = (m_ee.read(addr + 1) << 8) | m_ee.read(addr);

  if (actual != data) {
    m_tft.drawText(10, 10, "Result:",                              TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %04X", data),   TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %04X", actual), TftColor::MAGENTA, 3);

    wait_continue();

    m_tft.fillScreen(TftColor::BLACK);

    return 3;
  }

  return 0;
}

uint8_t ProgrammerFromSd::read_range() {
  uint16_t addr1 = ask_val<uint16_t>("Start address?");
  m_tft.fillScreen(TftColor::BLACK);
  uint16_t addr2 = ask_val<uint16_t>("End address?");

  // Make sure addr1 <= addr2
  if (addr2 < addr1) swap<uint16_t>(&addr1, &addr2);

  m_tft.drawText(10, 252, "Please wait - accessing EEPROM...", TftColor::PURPLE, 2);

  uint8_t *data = m_ee.read(addr1, addr2);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText(10, 10, "Select viewing method:", TftColor::CYAN, 3);

  TftChoiceMenu menu(50, 10, 10, 10, 2, 80, 0);
  menu.add_btn_calc(m_tft, "Raw Hex",    TftColor::BLACK, TftColor::ORANGE);
  menu.add_btn_calc(m_tft, "Characters", TftColor::BLUE,  TftColor::CYAN);
  menu.add_btn_confirm(m_tft, true);

  uint8_t method = menu.wait_for_value(m_tch, m_tft);

  m_tft.fillScreen(TftColor::BLACK);

  if      (method == 0) show_range(data, addr1, addr2, &ProgrammerFromSd::calc_hex);
  else if (method == 1) show_range(data, addr1, addr2, &ProgrammerFromSd::calc_chars);
  else {
    m_tft.drawText(10,  10, "INTERNAL ERROR",               TftColor::RED,     3);
    m_tft.drawText(10,  50, "Got nonexistent",              TftColor::MAGENTA, 2);
    m_tft.drawText(10,  80, "viewing method.",              TftColor::MAGENTA, 2);
    m_tft.drawText(10, 110, STRFMT_NOBUF("ID: %d", method), TftColor::PURPLE,  2);

    wait_continue();
  }

  m_tft.fillScreen(TftColor::BLACK);

  free(data);

  return 0;
}

// Assumes addr1 <= addr2
void ProgrammerFromSd::show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ProgrammerFromSd::calc_func calc) {
  // Draw frame for the data
  m_tft.drawText(10, 10, STRFMT_NOBUF("%d bytes", addr2 - addr1 + 1), TftColor::CYAN, 3);
  m_tft.drawRect(m_tft.width() / 2 - 147, 50, 295, 166, TftColor::WHITE);
  m_tft.drawRect(m_tft.width() / 2 - 146, 51, 293, 164, TftColor::WHITE);
  m_tft.drawFastVLine(m_tft.width() / 2, 52, 162, TftColor::GRAY);

  TftMenu menu;
  menu.add_btn(new TftBtn(15,                 60, 40, 150, 15, 68, "<"));
  menu.add_btn(new TftBtn(m_tft.width() - 55, 60, 40, 150, 15, 68, ">"));
  menu.add_btn(new TftBtn(BOTTOM_BTN(m_tft, "Continue")));
  menu.draw(m_tft);

  uint8_t cur_page = 0;
  uint8_t max_page = (addr2 >> 8) - (addr1 >> 8);

  while (true) {
    uint8_t req = show_page(data, addr1, addr2, calc, cur_page, max_page, menu);

    if      (req == 2) break;
    else if (req == 0) cur_page = (cur_page == 0 ? max_page : cur_page - 1);
    else if (req == 1) cur_page = (cur_page == max_page ? 0 : cur_page + 1);
  }
}

// Helper function of show_range() that shows
// only one page of data from a large buffer
uint8_t ProgrammerFromSd::show_page(
  uint8_t *data, uint16_t addr1, uint16_t addr2, ProgrammerFromSd::calc_func calc, uint8_t cur_page, uint8_t max_page, TftMenu &menu
) {
  char *text = (char *) malloc(3 * sizeof(*text));

  m_tft.fillRect(m_tft.width() / 2 - 145, 52, 145, 162, TftColor::DGRAY);
  m_tft.fillRect(m_tft.width() / 2 +   1, 52, 145, 162, TftColor::DGRAY);

  m_tft.fillRect(10, 224, 300, 16, TftColor::BLACK);
  m_tft.drawText(10, 224, STRFMT_NOBUF("Page #%d of %d", cur_page, max_page), TftColor::PURPLE, 2);

  for (uint16_t i = cur_page * 0x0100; /* condition is in loop body */; ++i) {
    if (i < addr1) continue;

    // x is the left margin of the block
    uint16_t x = ((i / 8) % 2 == 0 ? m_tft.width() / 2 - 142 : m_tft.width() / 2 + 6);
    uint16_t y = (i % 0x0100) / 16 * 10 + 55;

    uint8_t offset;
    uint16_t color;

    (this->*calc)(&offset, text, &color, data[i - addr1]);
    m_tft.drawText(x + 18 * (i % 8) + offset, y, text, color, 1);

    // Stop if we have reached end of data or end of page
    if (i == addr2 || i % 0x0100 == 0xFF) break;
  }

  free(text);

  return menu.wait_for_press(m_tch, m_tft);
}

// calc_hex() and calc_chars() are helper functions
// to calculate where and how data should be displayed
// for the different modes: hex and chars

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

uint8_t ProgrammerFromSd::write_multi() {
  /*
   * Interface:
   *
   * Button to add spanning width of screen at top,
   * just below title text
   * Below that is a scrollable list of menus in this format
   *  ____________________________
   * I                            I
   * I  AAAA     BB     [Delete]  I
   * I____________________________I
   *
   * Where AAAA is the address, in 4 hexadecimal digits
   * and BB is the data, in 2 hexadecimal digits
   *
   * The DELETE button deletes the pair from the list
   * Spanning each half of the bottom is an apply button
   * and a cancel button, respectively; Apply writes pairs
   * to EEPROM and exits, cancel just exits
   */

  AddrDataArray buf;
  buf.append((AddrDataArrayPair) {0xFFFA, 0x01});
  buf.append((AddrDataArrayPair) {0xFFFB, 0x12});
  buf.append((AddrDataArrayPair) {0xFFFC, 0x23});
  buf.append((AddrDataArrayPair) {0xFFFD, 0x34});
  buf.append((AddrDataArrayPair) {0xFFFE, 0x45});
  buf.append((AddrDataArrayPair) {0xFFFF, 0x56});

  m_ee.write(&buf);

  return nop();
}

uint8_t ProgrammerFromSd::verify_multi(uint16_t addr, uint16_t length, uint8_t *data) {
  return 3;
}

uint8_t ProgrammerFromSd::draw() {
  tft_draw_test(m_tch, m_tft);

  // This return will never happen because tft_draw_test() contains an infinite loop
  return 0;
}

uint8_t ProgrammerFromSd::debug() {
  TftMenu menu;
  menu.add_btn(new TftBtn( 10,  50, 225, 30,  23, 8, "WE HI (Disable)", TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN));
  menu.add_btn(new TftBtn(245,  50, 225, 30,  29, 8, "WE LO (Enable)",  TftColor::PINKK,          TftColor::RED));
  menu.add_btn(new TftBtn( 10,  90, 460, 30, 147, 8, "Set Address/OE",  TftColor::BLACK,          TftColor::YELLOW));
  menu.add_btn(new TftBtn( 10, 130, 225, 30,  37, 8, "Read Data Bus",   TftColor::BLUE,           TftColor::CYAN));
  menu.add_btn(new TftBtn(245, 130, 225, 30,  29, 8, "Write Data Bus",  TftColor::CYAN,           TftColor::BLUE));
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
      val16 = ask_val<uint16_t>("Type the value:");
      m_ee.set_addr_and_oe(val16);
      break;
    case 3:
      m_tft.fillScreen(TftColor::BLACK); 
      val8 = m_ee.get_data();
      m_tft.drawText(10, 10, "Value:", TftColor::CYAN, 4);
      m_tft.drawText(10, 50, STRFMT_NOBUF(BYTE_FMT, BYTE_FMT_VAL(val8)), TftColor::YELLOW, 2);
      wait_continue();
      break;
    case 4:
      m_tft.fillScreen(TftColor::BLACK); 
      val8 = ask_val<uint8_t>("Type the value:");
      m_ee.set_data(val8);
      break;
    }

    m_tft.fillScreen(TftColor::BLACK);
    m_tft.drawText(10, 10, "Debug Tools Menu", TftColor::CYAN, 4);
    menu.draw(m_tft);
  }

  m_tft.fillScreen(TftColor::BLACK);

  return 0;
}

// Dummy function for unimplemented actions
uint8_t ProgrammerFromSd::nop() {
  // Always returns 0 for success
  return 0;
}
