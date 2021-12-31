#ifdef DEBUG_MODE

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <Arduino.h>
#include "constants.hpp"

void debug_print_addr_range(uint16_t addr1, uint16_t addr2, uint8_t *data);

#endif

#endif
