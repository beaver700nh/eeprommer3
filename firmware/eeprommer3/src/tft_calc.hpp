#ifndef TFT_CALC_HPP
#define TFT_CALC_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "touch.hpp"

/*
 * A set of functions to help calculate the positions of elements on a TFT screen
 */
namespace TftCalc {
  uint16_t t_center_x(uint16_t box, const char *text, uint8_t size);
  uint16_t t_center_x_P(uint16_t box, const char *text, uint8_t size);
  uint16_t t_center_x(TftCtrl &tft, const char *text, uint8_t size);
  uint16_t t_center_x_P(TftCtrl &tft, const char *text, uint8_t size);

  uint16_t t_center_x(uint16_t box, uint8_t len, uint8_t size);
  uint16_t t_center_x(TftCtrl &tft, uint8_t len, uint8_t size);

  uint16_t t_center_y(uint16_t box, uint8_t size);
  uint16_t t_center_y(TftCtrl &tft, uint8_t size);

  uint16_t t_width(uint8_t len, uint8_t size);
  uint16_t t_width(const char *text, uint8_t size);
  uint16_t t_width_P(const char *text, uint8_t size);

  uint16_t fraction(uint16_t box, uint16_t margin, uint8_t denom);
  uint16_t fraction_x(TftCtrl &tft, uint16_t margin, uint8_t denom);
  uint16_t fraction_y(TftCtrl &tft, uint16_t margin, uint8_t denom);

  uint16_t right(uint16_t box, uint16_t width, uint16_t margin);
  uint16_t right(TftCtrl &tft, uint16_t width, uint16_t margin);

  uint16_t bottom(uint16_t box, uint16_t height, uint16_t margin);
  uint16_t bottom(TftCtrl &tft, uint16_t height, uint16_t margin);
};

#endif
