#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <Arduino.h>
#include "constants.hpp"

#ifdef DEBUG_MODE

void debug_print_addr_range(uint16_t addr1, uint16_t addr2, uint8_t *data);

#endif

#endif
