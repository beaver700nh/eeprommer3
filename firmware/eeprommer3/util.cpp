#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"

#include "util.hpp"

namespace Util {
  void wait_continue(TftCtrl &tft, TouchCtrl &tch) {
    static TftBtn continue_btn(BOTTOM_BTN(tft, "Continue"));
    continue_btn.draw(tft);
    continue_btn.wait_for_press(tch, tft);
  }
};
