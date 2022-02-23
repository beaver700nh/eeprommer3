#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "sd.hpp"

SdCtrl::SdCtrl(uint8_t cs, int8_t en)
  : m_cs(cs), m_en(en) {
  // Empty
}

uint8_t SdCtrl::init() {
  pinMode(m_en, INPUT_PULLUP);

  if (digitalRead(m_en) == LOW) {
    m_enabled = false;
    return STATUS_DISABLED;
  }
  else {
    m_enabled = true;

    pinMode(m_cs, OUTPUT);

    if (m_cs != 10) {
      pinMode(10, OUTPUT);
    }

    if (m_cs != 53) {
      pinMode(53, OUTPUT);
    }

    if (!SD.begin(m_cs)) {
      m_enabled = false;
      return STATUS_FAILED;
    }

    return STATUS_OK;
  }
}

bool SdCtrl::is_enabled() {
  return m_enabled;
}

uint8_t SdCtrl::get_files(const char *dir, char (*out)[13], uint8_t num) {
  File root = SD.open(dir);
  File file;

  uint8_t file_num = 0;

  while ((file = root.openNextFile())) {
    if (file_num >= num) break;

    strcpy(out[file_num++], file.name());
  }

  file.close();
  root.close();

  return file_num;
}
