#ifndef TFT_HPP
#define TFT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

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

class TouchscreenCtrl : public TouchScreen {
public:
  TouchscreenCtrl(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t resist);

  TSPoint getPoint();
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

TSPoint map_point(TSPoint p);

#ifdef DEBUG_MODE
void tft_print_point(TSPoint p, TftCtrl &tft);
void serial_print_point(TSPoint p);
#endif

#endif
