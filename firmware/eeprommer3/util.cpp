#include <Arduino.h>
#include "constants.hpp"

#include "util.hpp"

#undef swap

namespace Util {
  void validate_addr(uint16_t *addr) {
    *addr &= ~0x8000;
  }

  void validate_addrs(uint16_t *addr1, uint16_t *addr2) {
    validate_addr(addr1);
    validate_addr(addr2);

    if (*addr1 > *addr2) swap(addr1, addr2);
  }
};
