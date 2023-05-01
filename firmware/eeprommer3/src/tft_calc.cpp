#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "touch.hpp"

#include "tft_calc.hpp"

uint16_t TftCalc::t_center_x(uint16_t box, const char *text, uint8_t size) {
  return (box - t_width(text, size)) / 2;
}

uint16_t TftCalc::t_center_x(TftCtrl &tft, const char *text, uint8_t size) {
  return t_center_x(tft.width(), text, size);
}

uint16_t TftCalc::t_center_x_l(uint16_t box, uint8_t len, uint8_t size) {
  return (box - t_width(len, size)) / 2;
}

uint16_t TftCalc::t_center_x_l(TftCtrl &tft, uint8_t len, uint8_t size) {
  return t_center_x_l(tft.width(), len, size);
}

uint16_t TftCalc::t_center_y(uint16_t box, uint8_t size) {
  return (box - size * 8) / 2;
}

uint16_t TftCalc::t_center_y(TftCtrl &tft, uint8_t size) {
  return t_center_y(tft.height(), size);
}

// Get width in pixels of a PROGMEM string
uint16_t TftCalc::t_width(const char *text, uint8_t size) {
  return t_width(strlen_P(text), size);
}

uint16_t TftCalc::t_width(uint8_t len, uint8_t size) {
  return size * (len * 6 - 1);
}

uint16_t TftCalc::fraction(uint16_t box, uint16_t margin, uint8_t denom) {
  return (box - (denom + 1) * margin) / denom;
}

uint16_t TftCalc::fraction_x(TftCtrl &tft, uint16_t margin, uint8_t denom) {
  return fraction(tft.width(), margin, denom);
}

uint16_t TftCalc::fraction_y(TftCtrl &tft, uint16_t margin, uint8_t denom) {
  return fraction(tft.height(), margin, denom);
}

uint16_t TftCalc::right(uint16_t box, uint16_t width, uint16_t margin) {
  return box - margin - width;
}

uint16_t TftCalc::right(TftCtrl &tft, uint16_t width, uint16_t margin) {
  return right(tft.width(), margin, width);
}

uint16_t TftCalc::bottom(uint16_t box, uint16_t height, uint16_t margin) {
  return box - margin - height;
}

uint16_t TftCalc::bottom(TftCtrl &tft, uint16_t height, uint16_t margin) {
  return bottom(tft.height(), height, margin);
}
