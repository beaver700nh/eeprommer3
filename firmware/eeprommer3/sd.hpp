#ifndef SD_HPP
#define SD_HPP

#include <Arduino.h>
#include "constants.hpp"

#define MEGA_SOFT_SPI 1
#include <SD.h>

class SdCtrl {
public:
  SdCtrl() {};
  SdCtrl(uint8_t cs, int8_t en = -1);

  bool init();

private:
  uint8_t m_cs;
  int8_t m_en;

  bool m_enabled;
};

#endif
