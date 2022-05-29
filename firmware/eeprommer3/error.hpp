#ifndef ERROR_HPP
#define ERROR_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "touch.hpp"

namespace ErrorLevel {
  enum : uint8_t {INFO, WARNING, ERROR};

  static inline constexpr const char *const NAMES[] {
    "Info", "Warning", "Error"
  };

  static inline constexpr const uint16_t COLORS[] {
    TftColor::DCYAN, TftColor::MAGENTA, TftColor::DRED
  };
};

namespace Dialog {

void show_error(TftCtrl &tft, TouchCtrl &tch, uint8_t lvl, const char *title, const char *msg);

};

#endif
