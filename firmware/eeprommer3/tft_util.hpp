#ifndef TFT_UTIL_HPP
#define TFT_UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft.hpp"
#include "touch.hpp"

extern TouchCtrl tch;

namespace TftUtil {
  // Function to wait for the user to press a button at the bottom of the screen
  void wait_bottom_btn(const char *text);

  // Function to wait for the user to press a "Continue" button at the bottom of the screen
  void wait_continue();

  /*
   * Some reused lambdas
   */
  namespace Lambdas {
    // Tells whether screen is being touched
    inline auto is_tching_tft() {
      return [&]() -> bool { return tch.is_touching(); };
    }

    // Tells whether buttons is being pressed
    inline auto is_tching_btn(Gui::Btn &btn) {
      return [&]() -> bool { return btn.is_pressed(); };
    }
  };
};

#endif
