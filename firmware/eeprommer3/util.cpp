#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"

#include "util.hpp"

#undef swap

namespace Util {
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    static TftBtn continue_btn(BOTTOM_BTN(tft, text));
    continue_btn.draw(tft);
    continue_btn.wait_for_press(tch, tft);
  }

  void validate_addr(uint16_t *addr) {
    *addr &= ~0x8000;
  }

  void validate_addrs(uint16_t *addr1, uint16_t *addr2) {
    validate_addr(addr1);
    validate_addr(addr2);

    if (*addr1 > *addr2) swap(addr1, addr2);
  }
};
