#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "tft.hpp"
#include "input.hpp"

#include "vector.hpp"

Vector ask_vector(TftCtrl &tft, TouchCtrl &tch) {
  return Vector(
    ask_choice(
      tft, tch, "Which vector?", 3, 54, 1, 3,
      Vector::NAMES[0], TftColor::CYAN,           TftColor::BLUE,
      Vector::NAMES[1], TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN,
      Vector::NAMES[2], TftColor::PINKK,          TftColor::RED
    )
  );
}