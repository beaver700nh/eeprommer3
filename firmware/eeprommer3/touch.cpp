#include <Arduino.h>
#include "constants.hpp"

#include <TouchScreen.h>

#include "tft.hpp"
#include "touch.hpp"

bool TouchCtrl::is_valid_pressure(int16_t pressure, int16_t max_pressure) {
  return pressure > 10 && (max_pressure < 0 || pressure < max_pressure);
}

TSPoint TouchCtrl::get_tft_point(uint16_t minx, uint16_t maxx, uint16_t miny, uint16_t maxy, uint16_t tftw, uint16_t tfth) {
  TSPoint p = get_raw_point();

  // Map raw point from touchscreen coords to TFT coords
  uint16_t x = map(p.y, miny, maxy, 0, tftw);
  uint16_t y = map(p.x, minx, maxx, 0, tfth);

  return TSPoint(x, y, p.z);
}

TSPoint TouchCtrl::get_raw_point() {
  TSPoint p = getPoint();

  // `getPoint()` sets these to `INPUT`s, but they need to be `OUTPUT`s for TFT
  pinMode(TS_XM, OUTPUT);
  pinMode(TS_YP, OUTPUT);

  return p;
}

bool TouchCtrl::isTouching(void) {
  return is_valid_pressure(get_raw_point().z);
}

bool TouchCtrl::is_touching() {
  return isTouching();
}

void TouchCtrl::wait_for_press() {
  while (!is_touching());
}
