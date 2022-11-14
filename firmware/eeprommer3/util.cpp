#include <Arduino.h>
#include "constants.hpp"

#include <avr/io.h>

#include "strfmt.hpp"
#include "util.hpp"

#undef swap

// NOLINTBEGIN: avr linker symbols
extern unsigned int __bss_start, __bss_end;
extern unsigned int __heap_start, __heap_end;
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

void Util::restart() {
  asm volatile ("jmp 0");
}

void Memory::calculate_bords() {
  // Internal
  bords[0] = RAMSTART;
  bords[1] = (uint16_t) &__bss_start;
  bords[2] = (uint16_t) &__bss_end;
  bords[3] = SP;
  bords[4] = RAMEND;

  // External
  bords[5] = (uint16_t) &__heap_start;
  bords[6] = (uint16_t) __brkval;
  bords[7] = (uint16_t) &__heap_end;
  bords[8] = 0xFFFF;

  // __brkval may be zero if heap is empty
  if (bords[6] == 0) bords[6] = bords[5];
}

void Memory::print_ram_analysis() {
  calculate_bords();

#ifdef LOGGING
  SER_LOG_PRINT("RAM Analysis:\n");

  for (uint8_t i = 0; i <= NUM_TYPES; ++i) {
    SER_LOG_PRINT("+----------------------+ < 0x%04X\n", bords[i]);

    if (i >= NUM_TYPES) break;

    const char *name = Util::strdup_P(NAMES[i]);
    uint16_t size = bords[i + 1] - bords[i];
    uint8_t percentage = 100 * ((float) size / (float) bords[8]);

    SER_LOG_PRINT("| %-6s %5db (%3d%%) |\n", name, size, percentage);

    free((void *) name);
  }

  SER_LOG_PRINT("\n");
#endif
}
