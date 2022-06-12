#include <Arduino.h>
#include "constants.hpp"

#include "util.hpp"

#undef swap

extern unsigned int __bss_start, __heap_start, __brkval;

/*
 * Memory map:
 *
 * +-------------+ < RAMEND
 * |    Stack    |
 * +-------------+ < SP
 * | Free Memory |
 * +-------------+ < __brkval
 * |    Heap     |
 * +-------------+ < __heap_start
 * |     BSS     |
 * +-------------+ < __bss_start
 * | Static Data |
 * +-------------+ < RAMSTART
 */

void Util::validate_addr(uint16_t *addr) {
  *addr &= ~0x8000;
}

void Util::validate_addrs(uint16_t *addr1, uint16_t *addr2) {
  validate_addr(addr1);
  validate_addr(addr2);

  if (*addr1 > *addr2) swap(addr1, addr2);
}

void Memory::calculate() {
  sizes[Types::DATA]  =                                                                     (uint32_t) &__bss_start - RAMSTART;
  sizes[Types::BSS]   =                                          (uint32_t) &__heap_start - (uint32_t) &__bss_start;       /**/
  sizes[Types::HEAP]  =                   (uint32_t) &__brkval - (uint32_t) &__heap_start;                      /**/       /**/
  sizes[Types::FREE]  =              SP - (uint32_t) &__brkval;                       /**/                      /**/       /**/
  sizes[Types::STACK] = 1 + RAMEND - SP;                   /**/                       /**/                      /**/       /**/
  sizes[Types::TOTAL] = 1 + RAMEND - /*****************************************************************************/  RAMSTART;
}

void Memory::repr() {
  for (uint8_t i = 0; i < NUM_TYPES; ++i) {
    snprintf(repr_sizes[i], 26, "%-5s %6ld bytes (%3d%%)", NAMES[i], sizes[i], 100 * (uint8_t) (sizes[i] / sizes[Types::TOTAL]));
  }

  snprintf(repr_bords[Types::DATA],  20, "%04lX (RAMSTART)",     (uint32_t) RAMSTART     );
  snprintf(repr_bords[Types::BSS],   20, "%04lX (__bss_start)",  (uint32_t) &__bss_start );
  snprintf(repr_bords[Types::HEAP],  20, "%04lX (__heap_start)", (uint32_t) &__heap_start);
  snprintf(repr_bords[Types::FREE],  20, "%04lX (__brkval)",     (uint32_t) &__brkval    );
  snprintf(repr_bords[Types::STACK], 20, "%04lX (SP)",           (uint32_t) SP           );
  snprintf(repr_bords[Types::TOTAL], 20, "%04lX (RAMEND)",       (uint32_t) RAMEND       );
}

void Memory::print_ram_analysis() {
  calculate();
  repr();

  SER_LOG_PRINT("RAM Analysis:");

  for (uint8_t i = 0; i < NUM_TYPES; ++i) {
    SER_LOG_PRINT("+---------------------------+ < 0x%s", repr_bords[i]);
    SER_LOG_PRINT("| %s |", repr_sizes[i]);
  }

  SER_LOG_PRINT("+---------------------------+\n");
}
