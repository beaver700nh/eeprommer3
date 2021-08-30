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

  TftBtn continue_btn(10, 286, 460, 24, 184, 5, "Continue");

  uint8_t cur_choice = 0;

  while (true) { // Main loop
    while (true) { // Loop to get action
      menu.erase(m_tft);
      menu.draw(m_tft);
  
      int16_t btn_pressed = menu.wait_for_press(m_tch, m_tft);
  
      if (btn_pressed == 6) {
        break;
      }
  
      menu.get_btn(cur_choice)->highlight(false); // Old "current" choice
      cur_choice = (uint8_t) btn_pressed;
      menu.get_btn(cur_choice)->highlight(true);
    }

    m_tft.fillScreen(TftColor::BLACK);
  
    uint8_t err_code = do_action(cur_choice);
    show_status(err_code);

    continue_btn.draw(m_tft);
    continue_btn.wait_for_press(m_tch, m_tft);

    m_tft.fillScreen(TftColor::BLACK);
  }
}

uint8_t ProgrammerFromSD::do_action(uint8_t action) {
  switch (action) {
  case 0:  return read_byte();
  default: return 1;
  }
}

void ProgrammerFromSD::show_status(uint8_t code) {
  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10, 10, "Result:", TftColor::CYAN, 4);

  char *details_buf[10] = {
    "No errors.",
    "Invalid action.",
  };

  m_tft.drawText(15, 50, (code ? "Failed!" : "Success!"), (code ? TftColor::RED : TftColor::GREEN), 3);
  m_tft.drawText(15, 80, details_buf[code], TftColor::PURPLE, 2);
}

uint8_t ProgrammerFromSD::read_byte() {
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
