#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "tft.hpp"
#include "touch.hpp"

// Vector is a helper class for vector-related actions.
struct Vector {
  Vector(uint8_t id) : m_id(id), m_addr(0xFFF8 + 2 * (id + 1)) {};

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

namespace Dialog {

// Helper function to ask user to select a 6502 jump vector
Vector ask_vector(TftCtrl &tft, TouchCtrl &tch);

};

#endif
