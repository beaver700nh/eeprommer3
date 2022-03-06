#ifndef UTIL_HPP
#define UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

namespace Util {
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
};

#endif
