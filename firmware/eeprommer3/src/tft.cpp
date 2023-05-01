#include <Arduino.h>
#include "constants.hpp"

#include <avr/pgmspace.h>

#include <MCUFRIEND_kbv/MCUFRIEND_kbv.h>
#include <Adafruit_TouchScreen/TouchScreen.h>

#include "touch.hpp"
#include "new_delete.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "util.hpp"

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

void TftCtrl::drawText_P(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size) {
  const char *_text = Util::strdup_P(text);
  drawText(x, y, _text, color, size);
  free((void *) _text);
}

void TftCtrl::drawTextBg(uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t bg, uint8_t size) {
  fillRect(x, y, TftCalc::t_width(text, size), size * 8, bg);
  drawText(x, y, text, color, size);
}

void TftCtrl::drawTextBg_P(uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t bg, uint8_t size) {
  fillRect(x, y, TftCalc::t_width(text, size), size * 8, bg);
  drawText_P(x, y, text, color, size);
}

extern TftCtrl tft;
extern TouchCtrl tch;

void tft_draw_test() {
  static const uint16_t color = TftColor::LGRAY;

  TSPoint old;
  unsigned long last = 0;

  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft.width(), tft.height());

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

void tft_print_chars() {
  uint8_t original_rotation = tft.getRotation();
  tft.setRotation(0);

  uint8_t i = 0;

  do {
    tft.drawText(10 + 18 * (i & 0x0F), 10 + 24 * (i >> 4), STRFMT_NOBUF("%c", i));
  }
  while (i++ != 0xFF);

  tft.setRotation(original_rotation);
}

static const uint16_t PSTR_COLORS[] PROGMEM {
  TftColor::DRED,   TftColor::RED,     TftColor::ORANGE,  TftColor::YELLOW,
  TftColor::LIME,   TftColor::LGREEN,  TftColor::GREEN,   TftColor::OLIVE,
  TftColor::DGREEN, TftColor::DCYAN,   TftColor::CYAN,    TftColor::BLUE,
  TftColor::DBLUE,  TftColor::PURPLE,  TftColor::MAGENTA, TftColor::PINKK,
  TftColor::BLACK,  TftColor::DGRAY,   TftColor::GRAY,    TftColor::LGRAY,  TftColor::WHITE,
};

static const char PSTR_NAMES_0[] PROGMEM = "Dark Red";
static const char PSTR_NAMES_1[] PROGMEM = "Red";
static const char PSTR_NAMES_2[] PROGMEM = "Orange";
static const char PSTR_NAMES_3[] PROGMEM = "Yellow";
static const char PSTR_NAMES_4[] PROGMEM = "Lime";
static const char PSTR_NAMES_5[] PROGMEM = "Light Green";
static const char PSTR_NAMES_6[] PROGMEM = "Green";
static const char PSTR_NAMES_7[] PROGMEM = "Olive";
static const char PSTR_NAMES_8[] PROGMEM = "Dark Green";
static const char PSTR_NAMES_9[] PROGMEM = "Dark Cyan";
static const char PSTR_NAMES_A[] PROGMEM = "Cyan";
static const char PSTR_NAMES_B[] PROGMEM = "Blue";
static const char PSTR_NAMES_C[] PROGMEM = "Dark Blue";
static const char PSTR_NAMES_D[] PROGMEM = "Purple";
static const char PSTR_NAMES_E[] PROGMEM = "Magenta";
static const char PSTR_NAMES_F[] PROGMEM = "Pink";
static const char PSTR_NAMES_G[] PROGMEM = "Black";
static const char PSTR_NAMES_H[] PROGMEM = "Dark Gray";
static const char PSTR_NAMES_I[] PROGMEM = "Gray";
static const char PSTR_NAMES_J[] PROGMEM = "Light Gray";
static const char PSTR_NAMES_K[] PROGMEM = "White";

static const char *const PSTR_NAMES[] PROGMEM {
  PSTR_NAMES_0, PSTR_NAMES_1, PSTR_NAMES_2, PSTR_NAMES_3,
  PSTR_NAMES_4, PSTR_NAMES_5, PSTR_NAMES_6, PSTR_NAMES_7,
  PSTR_NAMES_8, PSTR_NAMES_9, PSTR_NAMES_A, PSTR_NAMES_B,
  PSTR_NAMES_C, PSTR_NAMES_D, PSTR_NAMES_E, PSTR_NAMES_F,
  PSTR_NAMES_G, PSTR_NAMES_H, PSTR_NAMES_I, PSTR_NAMES_J, PSTR_NAMES_K,
};

void tft_show_colors() {
  using namespace TftColor;

  for (uint8_t i = 0; i < 21; ++i) {
    auto color = pgm_read_word_near(PSTR_COLORS + i);

    auto *const name = (const char *) pgm_read_word_near(PSTR_NAMES + i);

    tft.fillScreen(color);

    tft.drawRoundRect(9, 9, 212, 38, 4, WHITE);
    tft.fillRoundRect(10, 10, 210, 36, 4, BLACK);
    tft.drawText_P(16, 16, name, WHITE, 3);

    delay(1250);
  }
}
