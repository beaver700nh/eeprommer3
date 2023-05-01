#ifndef XRAM_HPP
#define XRAM_HPP

#include <Arduino.h>
#include "constants.hpp"

/*
 * Namespace for controlling the XMEM interface.
 */
namespace xram {
  /*
   * Initializes XMEM with parameters as per ATmega datasheet:
   * - `waits`      - SRW11, SRW10
   * - `portc_mask` - XMM2, XMM1, XMM0
   */
  void init(uint8_t waits, uint8_t portc_mask);

  bool is_initialized();

  // Returns pointer to address in RAM (XMEM is 0x8000-0xFFFF)
  volatile uint8_t *access(uint16_t addr);

  // Tests to see if XMEM is working
  // WARNING: Overwrites contents of XMEM!
  struct TestResults {
    uint16_t successes;
    unsigned long time;
  };

  TestResults test();
}

#endif
