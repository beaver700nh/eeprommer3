#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft_calc.hpp"
#include "tft.hpp"
#include "touch.hpp"

#include "tft_util.hpp"

void TftUtil::wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
  static Gui::Btn continue_btn(BOTTOM_BTN(tft, text));
  continue_btn.draw(tft);
  continue_btn.wait_for_press(tch, tft);
}
