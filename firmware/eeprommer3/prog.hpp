#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

/*
 * Vector is a helper class for the
 * "read vector" and "write vector"
 * actions.
 */

struct Vector {
  Vector(uint8_t id)
    : m_id(id), m_addr(0xFFF8 + 2 * (id + 1)) {
    // Empty
  }

  void update(EepromCtrl &ee) {
    m_lo = ee.read(m_addr);
    m_hi = ee.read(m_addr + 1);
    m_val = (m_hi << 8) | m_lo;
  }

  uint8_t m_id;
  uint16_t m_addr;

  uint8_t m_lo, m_hi;
  uint16_t m_val;

  inline static const char *NAMES[3] = {"NMI", "RESET", "IRQ"};
};

/*
 * ProgrammerFromSd is a class that
 * connects the front-end and back-end
 * for programming the EEPROM from the
 * on-board SD card.
 */

class ProgrammerFromSd {
public:
  ProgrammerFromSd(EepromCtrl &ee, SdCtrl &sd, TouchCtrl &tch, TftCtrl &tft);

  void run();
  void show_status(uint8_t code);

  // Helper function to ask the user for
  // an arbitrarily-sized integer
  template<typename T>
  T ask_val(const char *prompt) {
    m_tft.drawText(10, 10, prompt, TftColor::CYAN, 4);
  
    TftHexSelMenu<T> menu(m_tft, 50, 17);
    menu.add_btn(new TftBtn(10, 286, 460, 24, 184, 5, "Continue"));
    menu.draw(m_tft);
  
    while (true) { // Loop to get a val
      menu.show_val(m_tft, 10, 170, 4, TftColor::ORANGE, TftColor::BLACK);
  
      uint8_t btn_pressed = menu.wait_for_press(m_tch, m_tft);
  
      if (btn_pressed == 16) break;
  
      menu.update_val(btn_pressed);
    }
  
    return menu.get_val();
  }

  Vector ask_vector();

  void wait_continue();

  typedef uint8_t (ProgrammerFromSd::*action_func)();

  /* 
   * Values returned from these functions are indexes
   * into the array details_buf in show_status()
   */

  uint8_t read_byte();
  uint8_t write_byte();
  uint8_t verify_byte(uint16_t addr, uint8_t data);

  uint8_t read_vector();
  uint8_t write_vector();
  uint8_t verify_vector(uint16_t addr, uint16_t data);

  uint8_t read_range();
  uint8_t write_multi();
  uint8_t verify_multi(uint16_t addr, uint16_t length, uint8_t *data);

  typedef void (ProgrammerFromSd::*calc_func)(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc);
  uint8_t show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc, uint8_t cur_page, uint8_t max_page, TftMenu &menu);

  // These two are `calc_func`s
  void calc_hex(uint8_t *offset, char *text, uint16_t *color, uint8_t data);
  void calc_chars(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  uint8_t draw();
  uint8_t debug();

  uint8_t nop();

  static constexpr uint8_t NUM_ACTIONS = 10;

  action_func action_map[NUM_ACTIONS] = {
    &ProgrammerFromSd::read_byte,   &ProgrammerFromSd::write_byte,
    &ProgrammerFromSd::nop,         &ProgrammerFromSd::nop,
    &ProgrammerFromSd::read_vector, &ProgrammerFromSd::write_vector,
    &ProgrammerFromSd::read_range,  &ProgrammerFromSd::write_multi,
    &ProgrammerFromSd::draw,        &ProgrammerFromSd::debug,
  };

private:
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
  TouchCtrl &m_tch;
  TftCtrl &m_tft;
};

#endif
