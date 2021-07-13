#include <Arduino.h>
#include "constants.hpp"

#include "sd.hpp"

SdCtrl::SdCtrl(uint8_t cs, int8_t en) {
  m_cs = cs;
  m_en = en;
}

bool SdCtrl::init() {
  pinMode(m_en, INPUT_PULLUP);

  if (digitalRead(m_en) == LOW) {
    m_enabled = false;
  }
  else {
    m_enabled = true;

    pinMode(m_cs, OUTPUT);
    
    if (m_cs != 10) {
      pinMode(10, OUTPUT);
    }

    if (!SD.begin(m_cs)) {
      m_enabled = false;
    }
  }

  return m_enabled;
}
