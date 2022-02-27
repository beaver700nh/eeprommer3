#ifndef SD_HPP
#define SD_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <SPI.h>
#include <SD.h>

class SdCtrl {
public:
  SdCtrl() {};
  SdCtrl(uint8_t cs, int8_t en = -1);

  uint8_t init();
  bool is_enabled();

  // Get names of at most `num` files from `dir` into `out`; return number of file names gotten
  uint8_t get_files(const char *dir, char (*out)[13], uint8_t num);

  // Check if `file` is a directory
  bool is_directory(const char *file);

  enum InitStatus {STATUS_OK, STATUS_DISABLED, STATUS_FAILED};

private:
  uint8_t m_cs;
  int8_t m_en;

  bool m_enabled;
};

#endif
