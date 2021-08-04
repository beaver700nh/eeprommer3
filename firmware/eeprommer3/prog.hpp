#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>

#include "sd.hpp"
#include "eeprom.hpp"

class ProgrammerFromSD {
public:
  static uint32_t write_file(EepromCtrl &ee, const char *file, uint16_t start, uint16_t n);
  static uint32_t read_file(EepromCtrl &ee, const char *file, uint16_t start, uint16_t n);
};

#endif
