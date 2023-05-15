#ifndef ERROR_HPP
#define ERROR_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "util.hpp"

namespace ErrorLevel {
  enum : uint8_t {INFO, WARNING, ERROR};

  // NOLINTBEGIN: not unused

  static inline constexpr const char *const NAMES[] {
    Strings::T_EL_INFO, Strings::T_EL_WARN, Strings::T_EL_ERR,
  };

  static inline constexpr const uint16_t COLORS[] {
    TftColor::DCYAN, TftColor::MAGENTA, TftColor::DRED,
  };

  // NOLINTEND
};

namespace Dialog {
  void show_error(uint8_t lvl, uint8_t str_types, const char *title, const char *msg);
  void wait_error(uint8_t lvl, uint8_t str_types, const char *title, const char *msg);
};

#endif
