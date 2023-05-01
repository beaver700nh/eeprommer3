#include <Arduino.h>
#include "constants.hpp"

#include "dialog.hpp"
#include "eeprom.hpp"
#include "util.hpp"

#include "vector.hpp"

Vector Dialog::ask_vector() {
  return Vector(
    ask_choice(
      Strings::P_VECTOR, 3, 54, 1, 3,
      Vector::NAMES[0], TftColor::CYAN,           TftColor::BLUE,
      Vector::NAMES[1], TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN,
      Vector::NAMES[2], TftColor::PINKK,          TftColor::RED
    )
  );
}
