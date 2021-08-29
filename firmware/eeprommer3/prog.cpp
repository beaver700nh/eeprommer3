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
  int16_t cur_choice = 0;

  TftMenu menu;

  menu.add_btn(new TftBtn( 10,  10, 225, 24, 61, 5, "Read Byte"));
  menu.add_btn(new TftBtn(245,  10, 225, 24, 55, 5, "Write Byte"));
  menu.add_btn(new TftBtn( 10,  44, 225, 24, 50, 5, "Read EEPROM"));
  menu.add_btn(new TftBtn(245,  44, 225, 24, 55, 5, "Write File"));
  menu.add_btn(new TftBtn( 10,  78, 225, 24, 50, 5, "Read Vector"));
  menu.add_btn(new TftBtn(245,  78, 225, 24, 44, 5, "Write Vector"));
  menu.add_btn(new TftBtn(180, 116, 120, 24, 20, 5, "Confirm"));

  while (true) {
    menu.erase(m_tft);
    menu.draw(m_tft);

    int16_t btn_pressed = menu.wait_for_press(m_tch, m_tft);

#ifdef DEBUG_MODE
    char buf[50];
    sprintf(buf, "Press: `%d'", btn_pressed);

    m_tft.fillRect(10, 170, 250, 22, TftColor::BLACK);
    m_tft.drawText(10, 170, buf, TftColor::CYAN, 3);
#endif

    if (btn_pressed != 6) {
      menu.get_btn(cur_choice)->highlight(false);
      cur_choice = btn_pressed;
      menu.get_btn(cur_choice)->highlight(true);
    }
    else {
      m_tft.drawText(10, 220, "Done!", TftColor::ORANGE, 3);
      while (true);
    }
  }
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
