#include <Arduino.h>
#include "constants.hpp"

#include <avr/io.h>

#include "util.hpp"

#undef swap

// NOLINTBEGIN: avr linker symbols
extern unsigned int __bss_start, __heap_start;
extern char *__brkval;
// NOLINTEND

char *Util::strdup_P(const char *pstr) {
  auto *buf = (char *) malloc(strlen_P(pstr) * sizeof(char));
  strcpy_P(buf, pstr);

  return buf;
}

void Util::validate_addr(uint16_t *addr) {
  *addr &= ~0x8000;
}

void Util::validate_addrs(uint16_t *addr1, uint16_t *addr2) {
  validate_addr(addr1);
  validate_addr(addr2);

  if (*addr1 > *addr2) swap(addr1, addr2);
}

void Memory::calculate() {
  bords[RAM_START]  = RAMSTART;
  bords[BSS_START]  = (uint32_t) &__bss_start;
  bords[HEAP_START] = (uint32_t) &__heap_start;
  bords[HEAP_END]   = (__brkval == 0 ? bords[HEAP_START] : (uint32_t) __brkval);
  bords[STACK_PTR]  = SP;
  bords[RAM_END]    = RAMEND;

  sizes[DATA]  =     /**/             /**/               /**/              /**/                bords[BSS_START] - bords[RAM_START];
  sizes[BSS]   =     /**/             /**/               /**/              bords[HEAP_START] - bords[BSS_START];  /**/
  sizes[HEAP]  =     /**/             /**/               bords[HEAP_END] - bords[HEAP_START];  /**/               /**/
  sizes[FREE]  =     /**/             bords[STACK_PTR] - bords[HEAP_END];  /**/                /**/               /**/
  sizes[STACK] = 1 + bords[RAM_END] - bords[STACK_PTR];  /**/              /**/                /**/               /**/
  sizes[TOTAL] = 1 + bords[RAM_END] - /**/               /**/              /**/                /**/               bords[RAM_START];
  //                 /**/             /**/               /**/              /**/                /**/               /**/
  // Memory map:     /**/ <- stack -> /**/ <-- free ---> /**/ <-- heap --> /**/ <--- bss ----> /**/ <-- data ---> /**/
  //                  ^^               ^^                 ^^                ^^                  ^^                 ^^
  //                RAMEND             SP             __brkval         __heap_start        __bss_start          RAMSTART
}

void Memory::repr() {
  for (uint8_t i = 0; i < NUM_TYPES; ++i) {
    uint8_t percentage = 100 * ((float) sizes[i] / (float) sizes[Types::TOTAL]);
    SNPRINTF(repr_sizes[i], "%-5s %4ld bytes (%3d%%)", NAMES[i], sizes[i], percentage);
  }

  SNPRINTF(repr_bords[Types::DATA],  "%04lX (RAMSTART)",     bords[RAM_START] );
  SNPRINTF(repr_bords[Types::BSS],   "%04lX (__bss_start)",  bords[BSS_START] );
  SNPRINTF(repr_bords[Types::HEAP],  "%04lX (__heap_start)", bords[HEAP_START]);
  SNPRINTF(repr_bords[Types::FREE],  "%04lX (__brkval)",     bords[HEAP_END]  );
  SNPRINTF(repr_bords[Types::STACK], "%04lX (SP)",           bords[STACK_PTR] );
  SNPRINTF(repr_bords[Types::TOTAL], "%04lX (RAMEND)",       bords[RAM_END]   );
}

void Memory::print_ram_analysis() {
  calculate();
  repr();

  SER_LOG_PRINT("RAM Analysis:\n");

  for (uint8_t i = 0; i <= NUM_TYPES; ++i) {
    SER_LOG_PRINT("+-------------------------+");

    if (i < NUM_TYPES) {
      PRINTF_NOBUF(Serial, " < 0x%s\n", repr_bords[i]);
      SER_LOG_PRINT("| %s |\n", repr_sizes[i]);
    }
  }

  Serial.println();
}
