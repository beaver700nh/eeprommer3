#ifndef INPUT_HPP
#define INPUT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <TouchScreen.h>

#include "tft.hpp"

class TftCtrl;

class TouchCtrl : public TouchScreen {
public:
  TouchCtrl(uint16_t xp, uint16_t xm, uint16_t yp, uint16_t ym, uint16_t resist);

  static bool is_valid_pressure(int16_t pressure, int16_t max_pressure = -1);

  TSPoint get_tft_point(uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy, TftCtrl &tft);
  TSPoint get_raw_point();
};

#endif
