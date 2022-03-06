#include <Arduino.h>
#include "constants.hpp"

#include "debug.hpp"

#ifdef DEBUG_MODE

void debug_print_addr_range(uint16_t addr1, uint16_t addr2, uint8_t *data) {
  uint16_t j = (addr1 >> 4);
  uint16_t k = 0;

  do {
    for (uint8_t i = 0; i < 16; ++i) {
      Serial.print((IN_RANGE((j << 4) + i, addr1, addr2 + 1) ? STRFMT_NOBUF("%02X ", data[k++]) : ".. "));
    }

    Serial.println();
  }
  while (j++ != (addr2 >> 4));

  Serial.println();
  Serial.println();
}

#endif
