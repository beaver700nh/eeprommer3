#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

class ProgrammerFromSD {
public:
  ProgrammerFromSD(EepromCtrl &ee, SdCtrl &sd, TouchCtrl &tch, TftCtrl &tft);

  void run();
  uint8_t do_action(uint8_t action);
  void show_status(uint8_t code);

  uint8_t read_byte();

  template<typename T>
  T get_number();

  uint32_t write_file(const char *file, uint16_t start, uint16_t n);
  uint32_t read_file(const char *file, uint16_t start, uint16_t n);

private:
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
  TouchCtrl &m_tch;
  TftCtrl &m_tft;
};

#endif
