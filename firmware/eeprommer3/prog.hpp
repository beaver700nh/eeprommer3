#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

class ProgrammerFromSd {
public:
  ProgrammerFromSd(EepromCtrl &ee, SdCtrl &sd, TouchCtrl &tch, TftCtrl &tft);

  void run();
  void show_status(uint8_t code);

  template<typename T>
  T ask_val(const char *prompt) {
    m_tft.drawText(10, 10, prompt, TftColor::CYAN, 4);
  
    TftHexSelMenu<T> menu(m_tft, 50, 17);
    menu.add_btn(new TftBtn(10, 286, 460, 24, 184, 5, "Continue"));
  
    while (true) { // Loop to get a val
      menu.erase(m_tft);
      menu.draw(m_tft);
      menu.show_val(m_tft, 10, 170, 4, TftColor::ORANGE, TftColor::BLACK);
  
      uint8_t btn_pressed = menu.wait_for_press(m_tch, m_tft);
  
      if (btn_pressed == 16) break;
  
      menu.update_val(btn_pressed);
    }
  
    return menu.get_val();
  }

  typedef uint8_t (ProgrammerFromSd::*action_func)();

  uint8_t read_byte();
  uint8_t write_byte();
  uint8_t verify_byte(uint16_t addr, uint8_t data);

  uint8_t read_vector();
  uint8_t write_vector();
  uint8_t verify_vector(uint16_t addr, uint16_t data);

  uint8_t nop();

  static constexpr uint8_t NUM_ACTIONS = 8;

  action_func action_map[NUM_ACTIONS] = {
    &read_byte,   &write_byte,
    &nop,         &nop,
    &read_vector, &write_vector,
    &nop,         &nop,
  };

private:
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
  TouchCtrl &m_tch;
  TftCtrl &m_tft;
};

#endif
