#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "eeprom.hpp"
#include "input.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

ProgrammerFromSD::ProgrammerFromSD(EepromCtrl &ee, SdCtrl &sd, TouchCtrl &tch, TftCtrl &tft)
  : m_ee(ee), m_sd(sd), m_tch(tch), m_tft(tft) {
  // Empty
}

void ProgrammerFromSD::run() {
  TftMenu menu;
  menu.add_btn(new TftBtn( 10,  10, 225, 24,  61, 5, "Read Byte",    TftColor::BLUE,   TftColor::CYAN));
  menu.add_btn(new TftBtn(245,  10, 225, 24,  55, 5, "Write Byte",   TftColor::RED,    TftColor::PINKK));
  menu.add_btn(new TftBtn( 10,  44, 225, 24,  44, 5, "Read to File", TftColor::CYAN,   TftColor::BLUE));
  menu.add_btn(new TftBtn(245,  44, 225, 24,  55, 5, "Write File",   TftColor::PINKK,  TftColor::RED));
  menu.add_btn(new TftBtn( 10,  78, 225, 24,  50, 5, "Read Vector",  TftColor::BLACK,  TftColor::GREEN));
  menu.add_btn(new TftBtn(245,  78, 225, 24,  44, 5, "Write Vector", TftColor::BLACK,  TftColor::ORANGE));
  menu.add_btn(new TftBtn( 10, 112, 460, 24, 190, 5, "Confirm",      TftColor::BLACK,  TftColor::WHITE));

  menu.get_btn(0)->highlight(true);

  uint8_t cur_choice = 0;

  while (true) { // Main loop
    while (true) { // Loop to get action
      menu.erase(m_tft);
      menu.draw(m_tft);
  
      uint8_t btn_pressed = menu.wait_for_press(m_tch, m_tft);
  
      if (btn_pressed == 6) break;
  
      menu.get_btn(cur_choice)->highlight(false); // Old "current" choice
      cur_choice = (uint8_t) btn_pressed;
      menu.get_btn(cur_choice)->highlight(true);
    }
  
    do_action(cur_choice);
  }
}

uint8_t ProgrammerFromSD::do_action(uint8_t action) {
  m_tft.fillScreen(TftColor::BLACK);

  uint8_t status_code = 1;

  switch (action) {
  case 0: status_code = read_byte(); break;
  case 1: status_code = 1; break;
  case 2: status_code = 1; break;
  case 3: status_code = 1; break;
  case 4: status_code = 1; break;
  case 5: status_code = 1; break;
  }

  show_status(status_code);

  TftBtn continue_btn(10, 286, 460, 24, 184, 5, "Continue");
  continue_btn.draw(m_tft);
  continue_btn.wait_for_press(m_tch, m_tft);

  m_tft.fillScreen(TftColor::BLACK);

  return status_code;
}

void ProgrammerFromSD::show_status(uint8_t code) {
  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10, 10, "Result:", TftColor::CYAN, 4);

  const char *details_buf[4] = {
    "No errors.",
    "Invalid action.",
    "Failed to open file.",
    "Verification failed.",
  };

  m_tft.drawText(15, 50, (code ? "Failed!" : "Success!"), (code ? TftColor::RED : TftColor::GREEN), 3);
  m_tft.drawText(15, 80, (code < 4 ? details_buf[code] : "Unknown reason."), TftColor::PURPLE, 2);
}

uint8_t ProgrammerFromSD::read_byte() {
  m_tft.drawText(10, 10, "What address?", TftColor::CYAN, 4);

  TftMenu menu;

  for (uint8_t i = 0x00; i < 0x10; ++i) {
    uint16_t x = 17 + 57 * (i % 8);
    uint16_t y = (i < 8 ? 50 : 107);

    menu.add_btn(new TftBtn(x, y, 47, 47, 18, 18, STRFMT_NOBUF("%1X", i), TftColor::WHITE, TftColor::BLUE));
  }

  menu.add_btn(new TftBtn(10, 286, 460, 24, 184, 5, "Continue"));

  uint16_t addr = 0x0000;

  while (true) { // Loop to get an addr
    menu.erase(m_tft);
    menu.draw(m_tft);

    m_tft.fillRect(90, 170, 332, 28, TftColor::BLACK);
    m_tft.drawText(90, 170, STRFMT_NOBUF("Addr: [%04X]", addr), TftColor::ORANGE, 4);

    uint8_t btn_pressed = menu.wait_for_press(m_tch, m_tft);

    if (btn_pressed == 16) break;

    addr = (addr << 4) + btn_pressed;
  }

  uint8_t val = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, STRFMT_NOBUF("Value at address %04X:", addr),      TftColor::CYAN,   3);
  m_tft.drawText(20,  50, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(val)), TftColor::YELLOW, 2);
  m_tft.drawText(20,  80, STRFMT_NOBUF("OCT: %03o",      val),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 110, STRFMT_NOBUF("HEX: %02X",      val),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 140, STRFMT_NOBUF("DEC: %-3d",      val),               TftColor::YELLOW, 2);

  TftBtn continue_btn(10, 286, 460, 24, 184, 5, "Continue");
  continue_btn.draw(m_tft);
  continue_btn.wait_for_press(m_tch, m_tft);

  menu.purge_btns();

  m_tft.fillScreen(TftColor::BLACK);

  return 0;
}

uint32_t ProgrammerFromSD::write_file(const char *file, uint16_t start, uint16_t n) {
//  File f = SD.open(file);
//  uint8_t b;
//  uint32_t i;
//
//  for (i = start; (f.read(&b, 1) != -1) && (i < 0x10000) && (i < start + n); ++i) {
//    ee.write(i, b);
//  }
//
//  return i - start;
}

uint32_t ProgrammerFromSD::read_file(const char *file, uint16_t start, uint16_t n) {
  return 0;
}
