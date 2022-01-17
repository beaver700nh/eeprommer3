#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

/*
 * Vector is a helper class for
 * ProgrammerFromSd's vector-related actions.
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

// Function to wait for the user to press a "Continue" button
void wait_continue(TftCtrl &tft, TouchCtrl &tch);

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

  typedef uint8_t (ProgrammerFromSd::*action_func)();

  uint8_t read_byte();
  uint8_t write_byte();
  uint8_t verify_byte(uint16_t addr, uint8_t data);

  uint8_t read_vector();
  uint8_t write_vector();
  uint8_t verify_vector(uint16_t addr, uint16_t data);

  // Function to ask the user to select a 6502 jump vector
  Vector ask_vector();

  uint8_t read_range();
  uint8_t write_multi();
  uint8_t verify_multi(uint16_t addr, uint16_t length, uint8_t *data);

  typedef void (ProgrammerFromSd::*calc_func)(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  // These two are `calc_func`s
  void calc_hex(uint8_t *offset, char *text, uint16_t *color, uint8_t data);
  void calc_chars(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc);
  void show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc, uint8_t cur_page, uint8_t max_page);

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

  enum { STATUS_OK, STATUS_ERR_INVALID, STATUS_ERR_FILE, STATUS_ERR_VERIFY, STATUS_ERR_MEMORY };

private:
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
  TouchCtrl &m_tch;
  TftCtrl &m_tft;
};

#endif
