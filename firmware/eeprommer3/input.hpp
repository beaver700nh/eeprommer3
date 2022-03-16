#ifndef INPUT_HPP
#define INPUT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <TouchScreen.h>

#include "tft.hpp"

class TftCtrl;

/*
 * `TouchCtrl` is a wrapper class around the third-party `TouchScreen` class that
 * helps get more accurate and helpful readings from a resistive touch screen.
 */
class TouchCtrl : public TouchScreen {
public:
  TouchCtrl(uint16_t xp, uint16_t xm, uint16_t yp, uint16_t ym, uint16_t resist): TouchScreen(xp, yp, xm, ym, resist) {};

  static bool is_valid_pressure(int16_t pressure, int16_t max_pressure = -1);

  bool isTouching(void); // Define this because it's undefined in base class
  bool is_touching();    // "Alias" to match naming scheme

  void wait_for_press();

  TSPoint get_tft_point(uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy, TftCtrl &tft);
  TSPoint get_raw_point();
};

#endif
