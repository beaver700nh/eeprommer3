#ifndef TFT_UTIL_HPP
#define TFT_UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"

namespace TftUtil {
  // Function to wait for the user to press a button at the bottom of the screen
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text);

  // Function to show an error message screen
  void show_error(TftCtrl &tft, TouchCtrl &tch, const char *text);


  /*
    * Some reused lambdas
    */
  namespace Lambdas {
    // Tells whether screen is being touched
    auto is_tching_tft = [](TouchCtrl &tch) {
      return [&]() -> bool { return tch.is_touching(); };
    };

    // Tells whether buttons is being pressed
    auto is_tching_btn = [](TftBtn &btn, TouchCtrl &tch,  TftCtrl &tft) {
      return [&]() -> bool { return btn.is_pressed(tch, tft); };
    };
  };
};

#endif
