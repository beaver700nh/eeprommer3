#ifndef UTIL_HPP
#define UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

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
  template<typename Func>
  bool skippable_delay(uint32_t time, Func exit_condition) {
    auto t1 = millis();

    while (millis() - t1 < time) {
      if (exit_condition()) return true;
    }

    return false;
  }

  // Function to validate an address
  void validate_addr(uint16_t *addr);

  // Function to validate two addresses (and make sure first is not greater than second)
  void validate_addrs(uint16_t *addr1, uint16_t *addr2);

  int available_memory();
};

#endif
