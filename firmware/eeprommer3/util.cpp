#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"

#include "util.hpp"

namespace Util {
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    static TftBtn continue_btn(BOTTOM_BTN(tft, text));
    continue_btn.draw(tft);
    continue_btn.wait_for_press(tch, tft);
  }
};
