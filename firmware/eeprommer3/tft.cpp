#include <Arduino.h>
#include "macros.hpp"

#include <Elegoo_TFTLCD.h>

#include "tft.hpp"

TftCtrl::TftCtrl(uint8_t cs, uint8_t rs, uint8_t wr, uint8_t rd, uint8_t rst)
  : Elegoo_TFTLCD(cs, rs, wr, rd, rst) {
  // Empty
}

void TftCtrl::init(uint16_t driver_id, uint8_t orientation, uint16_t text_color, uint8_t text_size) {
  reset();

  delay(500);

  begin(driver_id);
  setRotation(orientation);
  setTextColor(text_color);
  setTextSize(text_size);
}

void TftCtrl::drawText(uint16_t x, uint16_t y, const char *text, uint16_t color) {
  setTextColor(color);
  setCursor(x, y);
  print(text);
}
