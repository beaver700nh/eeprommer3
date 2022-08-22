#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "tft_util.hpp"

void TftUtil::wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
  static Gui::Btn continue_btn(BOTTOM_BTN(tft, text)); // todo - make sure this is only constructed once
  continue_btn.draw(tft);
  continue_btn.wait_for_press(tch, tft);
}

void TftUtil::wait_continue(TftCtrl &tft, TouchCtrl &tch) {
  wait_bottom_btn(tft, tch, Strings::L_CONTINUE);
}
