#include <Arduino.h>
#include "constants.hpp"

#include <TouchScreen.h>

#include "input.hpp"
#include "tft.hpp"

TouchCtrl::TouchCtrl(uint8_t xp, uint8_t xm, uint8_t yp, uint8_t ym, uint8_t resist)
  : TouchScreen(xp, yp, xm, ym, resist) {
  // Empty
}

bool TouchCtrl::is_valid_pressure(int16_t pressure, int16_t max_pressure) {
  if (max_pressure > 0) {
    return IN_RANGE(pressure, 10, max_pressure + 1);
  }
  else {
    return (pressure > 10);
  }
}

TSPoint TouchCtrl::get_tft_point(uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy, TftCtrl &tft) {
  TSPoint p = get_raw_point();

  uint16_t x = map(p.y, miny, maxy, 0, tft.width());
  uint16_t y = map(p.x, minx, maxx, 0, tft.height());

  return TSPoint(x, y, p.z);
}

TSPoint TouchCtrl::get_raw_point() {
  TSPoint p = getPoint();

  pinMode(TS_XM, OUTPUT);
  pinMode(TS_YP, OUTPUT);

  return p;
}
