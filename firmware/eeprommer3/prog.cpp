#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "prog.hpp"
#include "sd.hpp"

uint32_t ProgrammerFromSD::write_file(EepromCtrl &ee, const char *file, uint16_t start, uint16_t n) {
  File f = SD.open(file);
  uint8_t b;
  uint32_t i;

  for (i = start; (f.read(&b, 1) != -1) && (i < 0x10000) && (i < start + n); ++i) {
    ee.write(i, b);
  }

  return i - start;
}

uint32_t ProgrammerFromSD::read_file(EepromCtrl &ee, const char *file, uint16_t start, uint16_t n) {
  return 0;
}
