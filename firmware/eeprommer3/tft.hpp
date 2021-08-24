#ifndef TFT_HPP
#define TFT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

#define SERIAL_PRINTF_BUF_SIZE 100

static char serial_printf_buf[SERIAL_PRINTF_BUF_SIZE];

#define serial_printf(fmt, ...) \
  do { \
    snprintf(serial_printf_buf, sizeof(serial_printf_buf), fmt, ##__VA_ARGS__); \
    Serial.print(serial_printf_buf); \
  } \
  while (false)

namespace TftColor {
  enum : uint16_t {
    WHITE   = 0xFFFF,
    RED     = 0xF800,
    ORANGE  = 0xFEE3,
    YELLOW  = 0xFFE0,
    GREEN   = 0x07E0,
    CYAN    = 0x07FF,
    BLUE    = 0x001F,
    PURPLE  = 0xE31D,
    MAGENTA = 0xF81F,
    PINKK   = 0xFEF7,
    BLACK   = 0x0000,
  };
};

class TouchscreenCalibration {
public:
  TouchscreenCalibration(int16_t (*table)[9][9][2]);

  TSPoint adc_data_to_tft_coords(TSPoint p, uint8_t fineness = 8);

private:
  int16_t (*m_table)[9][9][2];
};

class TouchscreenCtrl : public TouchScreen {
public:
  TouchscreenCtrl(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t resist, TouchscreenCalibration &calib);

  TSPoint getPoint(bool raw);
  TSPoint mapPoint(TSPoint p);

  static bool isValidPoint(TSPoint p, int16_t maxz = -1);
  static bool isValidPressure(int16_t z, int16_t maxz = -1);

private:
  TouchscreenCalibration &m_calib;
};

class TftCtrl : public Elegoo_TFTLCD {
public:
  TftCtrl() {};
  TftCtrl(uint8_t cs, uint8_t rs, uint8_t wr, uint8_t rd, uint8_t rst);

  void init(uint16_t driver_id, uint8_t orientation);

  void drawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size = 2);
  void drawText(const char *text);
};

class TftBtn {
public:
  TftBtn() {};
  TftBtn(
    uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    const char *text, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  void draw(TftCtrl &tft);
  bool is_pressed(TouchscreenCtrl &ts);

private:
  uint8_t m_x, m_y, m_w, m_h;
  uint16_t m_fg, m_bg;

  char m_text[21];
};

class TftMenu {
public:
  TftMenu() {};

  bool add_btn(TftBtn *btn);

  void draw(TftCtrl &tft);
  uint8_t is_pressed(TouchscreenCtrl &ts);

  uint8_t wait_any_btn_down(TouchscreenCtrl &ts);
  void wait_all_btn_up(TouchscreenCtrl &ts);

private:
  TftBtn **m_btns = nullptr;
  uint8_t m_num_btns = 0;
};

#ifdef DEBUG_MODE
void tft_print_point(TSPoint p, TftCtrl &tft);
void serial_print_point(TSPoint p);
#endif

#endif
