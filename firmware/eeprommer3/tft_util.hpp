#ifndef TFT_UTIL_HPP
#define TFT_UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft.hpp"
#include "touch.hpp"

namespace TftUtil {
  // Function to wait for the user to press a button at the bottom of the screen
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text);

  // Function to wait for the user to press a "Continue" button at the bottom of the screen
  void wait_continue(TftCtrl &tft, TouchCtrl &tch);

  /*
   * Some reused lambdas
   */
  namespace Lambdas {
    // Tells whether screen is being touched
    auto is_tching_tft = [](TouchCtrl &tch) {
      return [&]() -> bool { return tch.is_touching(); };
    };

    // Tells whether buttons is being pressed
    auto is_tching_btn = [](Gui::Btn &btn, TouchCtrl &tch, TftCtrl &tft) {
      return [&]() -> bool { return btn.is_pressed(tch, tft); };
    };
  };
};

#endif
