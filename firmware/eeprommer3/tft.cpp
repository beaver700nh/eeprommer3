#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "touch.hpp"
#include "new_delete.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"

void TftCtrl::init(uint16_t driver_id, uint8_t orientation) {
  reset();

  delay(500);

  begin(driver_id);
  setRotation(orientation);
}

void TftCtrl::drawThickRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint8_t thickness) {
  for (uint8_t i = 0; i < thickness; ++i) {
    drawRect(x + i, y + i, w - i * 2, h - i * 2, color);
  }
}

void TftCtrl::drawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size) {
  setTextColor(color);
  setTextSize(size);
  setCursor(x, y);
  print(text);
}

void TftCtrl::drawTextBg(uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t bg, uint8_t size) {
  fillRect(x, y, TftCalc::t_width(text, size), size * 8, bg);
  drawText(x, y, text, color, size);
}

void TftCtrl::drawText(const char *text) {
  print(text);
}

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft) {
  static const uint16_t color = TftColor::LGRAY;

  TSPoint old;
  unsigned long last = 0;

  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (p.x < 3) break;

    if (TouchCtrl::is_valid_pressure(p.z)) {
      if (last == 0 || millis() - last > 50) {
        tft.drawPixel(p.x, p.y, color);
      }
      else {
        tft.drawLine(old.x, old.y, p.x, p.y, color);
      }

      old = p;
      last = millis();
    }
  }
}

void tft_print_chars(TftCtrl &tft) {
  uint8_t original_rotation = tft.getRotation();
  tft.setRotation(0);

  uint8_t i = 0;

  do {
    tft.drawText(10 + 18 * (i & 0x0F), 10 + 24 * (i >> 4), STRFMT_NOBUF("%c", i));
  }
  while (i++ != 0xFF);

  tft.setRotation(original_rotation);
}

void tft_show_colors(TftCtrl &tft) {
  using namespace TftColor;

  static const uint16_t colors[] {
    DRED, RED, ORANGE, YELLOW, LIME,
    LGREEN, GREEN, OLIVE, DGREEN,
    DCYAN, CYAN, BLUE, DBLUE,
    PURPLE, MAGENTA, PINKK,
    BLACK, DGRAY, GRAY, LGRAY, WHITE,
  };

  static const char *names[] {
    "Dark Red", "Red", "Orange", "Yellow", "Lime",
    "Light Green", "Green", "Olive", "Dark Green",
    "Dark Cyan", "Cyan", "Blue", "Dark Blue",
    "Purple", "Magenta", "Pink",
    "Black", "Dark Gray", "Gray", "Light Gray", "White",
  };

  for (uint8_t i = 0; i < 21; ++i) {
    tft.fillScreen(colors[i]);

    tft.drawRoundRect(9, 9, 212, 38, 4, WHITE);
    tft.fillRoundRect(10, 10, 210, 36, 4, BLACK);
    tft.drawText(16, 16, names[i], WHITE, 3);

    delay(1250);
  }
}
