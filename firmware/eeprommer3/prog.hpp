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

  /*** BYTE IO ***/

  uint8_t read_byte();
  uint8_t write_byte();
  uint8_t verify_byte(uint16_t addr, uint8_t data);

  /*** FILE IO ***/

  uint8_t read_file();
  uint8_t write_file();
  uint8_t verify_file(const char *fname, uint16_t addr);

  /*** VECTOR IO ***/

  uint8_t read_vector();
  uint8_t write_vector();
  uint8_t verify_vector(uint16_t addr, uint16_t data);

  // Function to ask the user to select a 6502 jump vector
  Vector ask_vector();

  /*** MULTIPLE IO ***/

  uint8_t read_range();
  uint8_t write_multi();
  uint8_t verify_multi(AddrDataArray &buf);

  // read_range() helpers

  typedef void (ProgrammerFromSd::*calc_func)(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  // These two are `calc_func`s
  void calc_hex(uint8_t *offset, char *text, uint16_t *color, uint8_t data);
  void calc_chars(uint8_t *offset, char *text, uint16_t *color, uint8_t data);

  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc);
  void show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, calc_func calc, uint8_t cur_page, uint8_t max_page);

  // write_multi() helpers

  void draw_pairs(
    uint16_t margin_l, uint16_t margin_r, uint16_t margin_u, uint16_t margin_d,
    uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, TftMenu &del_btns
  );

  bool poll_menus_and_react(TftMenu &menu, TftMenu &del_btns, AddrDataArray *buf, uint16_t *scroll, const uint16_t max_scroll);
  void add_pair_from_user(AddrDataArray *buf);

  /*** MISC ***/

  uint8_t draw();
  uint8_t debug();

  uint8_t nop();

  /*** END OF USER INTERACTION FUNCS ***/

  static constexpr uint8_t NUM_ACTIONS = 10;

  action_func action_map[NUM_ACTIONS] = {
    &ProgrammerFromSd::read_byte,   &ProgrammerFromSd::write_byte,
    &ProgrammerFromSd::read_file,   &ProgrammerFromSd::nop,
    &ProgrammerFromSd::read_vector, &ProgrammerFromSd::write_vector,
    &ProgrammerFromSd::read_range,  &ProgrammerFromSd::write_multi,
    &ProgrammerFromSd::draw,        &ProgrammerFromSd::debug,
  };

  /*
   * Enum of status codes returned from functions of
   * the type `ProgrammerFromSd::*action_func`
   */
  enum ActionFuncStatus { STATUS_OK, STATUS_ERR_INVALID, STATUS_ERR_FILE, STATUS_ERR_VERIFY, STATUS_ERR_MEMORY };

private:
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
  TouchCtrl &m_tch;
  TftCtrl &m_tft;
};

#endif
