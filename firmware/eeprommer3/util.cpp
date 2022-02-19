#include <Arduino.h>
#include "constants.hpp"

#include "util.hpp"

namespace Util {
  int get_free_memory() {
    char top;

  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
  }
};
