#ifndef UTIL_HPP
#define UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

class TftCtrl;
class TouchCtrl;

#undef swap

// A namespace with various miscellaneous utility functions
namespace Util {
  // Swaps two variables `a` and `b` of type `T`
  template<typename T>
  void swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
  }

  // `exit_condition` is a template type
  // to get around annoying lambda errors
  template<typename T>
  bool skippable_delay(uint32_t time, T exit_condition) {
    auto t1 = millis();

    while (millis() - t1 < time) {
      if (exit_condition()) return true;
    }

    return false;
  }

  // Function to wait for the user to press a "Continue" button
  void wait_continue(TftCtrl &tft, TouchCtrl &tch);
};

#endif
