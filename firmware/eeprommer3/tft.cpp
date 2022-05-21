#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "touch.hpp"
#include "new_delete.hpp"

#include "tft.hpp"

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
  fillRect(x, y, size * (6 * strlen(text) - 1), size * 8, bg);
  drawText(x, y, text, color, size);
}

void TftCtrl::drawText(const char *text) {
  print(text);
}

namespace TftCalc {
  uint16_t t_center_x(uint16_t box, const char *text, uint8_t size) {
    return (box + size - strlen(text) * 6 * size) / 2;
  }

  uint16_t t_center_x(TftCtrl &tft, const char *text, uint8_t size) {
    return t_center_x(tft.width(), text, size);
  }

  uint16_t t_center_x_l(uint16_t box, uint8_t len, uint8_t size) {
    return (box + size - len * 6 * size) / 2;
  }

  uint16_t t_center_x_l(TftCtrl &tft, uint8_t len, uint8_t size) {
    return t_center_x_l(tft.width(), len, size);
  }

  uint16_t t_center_y(uint16_t box, uint8_t size) {
    return (box - size * 8) / 2;
  }

  uint16_t t_center_y(TftCtrl &tft, uint8_t size) {
    return t_center_y(tft.height(), size);
  }

  uint16_t fraction(uint16_t box, uint16_t margin, uint8_t denom) {
    return (box - (denom + 1) * margin) / denom;
  }

  uint16_t fraction_x(TftCtrl &tft, uint16_t margin, uint8_t denom) {
    return fraction(tft.width(), margin, denom);
  }

  uint16_t fraction_y(TftCtrl &tft, uint16_t margin, uint8_t denom) {
    return fraction(tft.height(), margin, denom);
  }

  uint16_t right(uint16_t box, uint16_t width, uint16_t margin) {
    return box - margin - width;
  }

  uint16_t right(TftCtrl &tft, uint16_t width, uint16_t margin) {
    return right(tft.width(), margin, width);
  }

  uint16_t bottom(uint16_t box, uint16_t height, uint16_t margin) {
    return box - margin - height;
  }

  uint16_t bottom(TftCtrl &tft, uint16_t height, uint16_t margin) {
    return bottom(tft.height(), height, margin);
  }
};

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft) {
  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (p.x < 3) break;

    if (TouchCtrl::is_valid_pressure(p.z)) {
      tft.fillCircle(p.x, p.y, 3, TftColor::RED);
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

  static const uint16_t colors[] = {
    DRED, RED, ORANGE, YELLOW, LIME,
    LGREEN, GREEN, OLIVE, DGREEN,
    DCYAN, CYAN, BLUE, DBLUE,
    PURPLE, MAGENTA, PINKK,
    BLACK, DGRAY, GRAY, LGRAY, WHITE,
  };

  static const char *names[] = {
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
