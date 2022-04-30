#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"

#include "tft_util.hpp"

namespace TftUtil {
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    static TftBtn continue_btn(BOTTOM_BTN(tft, text));
    continue_btn.draw(tft);
    continue_btn.wait_for_press(tch, tft);
  }

  void show_error(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    tft.drawText(10, 10, "Error!",   TftColor::RED,    3);
    tft.drawText(10, 50, "Message:", TftColor::CYAN,   2);
    tft.drawText(10, 85, text,       TftColor::PURPLE, 2);

    wait_bottom_btn(tft, tch, "OK");
  }
};
