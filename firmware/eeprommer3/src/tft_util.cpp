#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "tft_util.hpp"

extern TftCtrl tft;

void TftUtil::wait_bottom_btn(const char *text) {
  Gui::Btn continue_btn(BOTTOM_BTN(text));
  continue_btn.draw();
  continue_btn.wait_for_press();
}

void TftUtil::wait_continue() {
  wait_bottom_btn(Strings::L_CONTINUE);
}
