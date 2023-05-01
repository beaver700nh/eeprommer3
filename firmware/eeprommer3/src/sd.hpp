#ifndef SD_HPP
#define SD_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <SPI/src/SPI.h>
#include <SD/src/SD.h>

/*
 * Simple little helper struct to store name of file and whether it is a directory.
 */
struct SdFileInfo {
  char name[13];
  bool is_dir;
};

class SdCtrl {
public:
  SdCtrl() {};
  SdCtrl(uint8_t cs, int8_t en = -1);

  enum Status {
    OK,       // No errors
    DISABLED, // Hardware switch has disabled SD
    FAILED,   // Could not initialize SD
  };

  Status init();
  bool is_enabled();

  // Get names of at most `num` files from `dir` into `out`; return number of file names gotten
  uint8_t get_files(const char *dir, SdFileInfo *out, uint8_t num);

  // Check if `file` is a directory
  bool is_directory(const char *file);

private:
  uint8_t m_cs;
  int8_t m_en;

  bool m_enabled;
};

#endif
