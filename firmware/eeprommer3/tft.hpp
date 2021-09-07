#ifndef TFT_HPP
#define TFT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>

#include "input.hpp"

namespace TftColor {
  enum : uint16_t {
    RED     = 0xF800,
    ORANGE  = 0xFEE3,
    YELLOW  = 0xFFE0,
    DGREEN  = 0x03E0,
    GREEN   = 0x07E0,
    CYAN    = 0x07FF,
    BLUE    = 0x001F,
    PURPLE  = 0xE31D,
    MAGENTA = 0xF81F,
    PINKK   = 0xFEF7,
    BLACK   = 0x0000,
    DGRAY   = 0x39E7,
    GRAY    = 0x7BEF,
    LGRAY   = 0xBDF7,
    WHITE   = 0xFFFF,
  };
};

class TouchCtrl;

class TftCtrl : public MCUFRIEND_kbv {
public:
  TftCtrl() {};

  void init(uint16_t driver_id, uint8_t orientation);

  void drawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size = 2);
  void drawText(const char *text);
};

class TftBtn {
public:
  TftBtn() {};
  TftBtn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty,
    const char *text, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  uint16_t get_x();
  void     set_x(uint16_t x);
  uint16_t get_y();
  void     set_y(uint16_t y);

  uint16_t get_w();
  void     set_w(uint16_t w);
  uint16_t get_h();
  void     set_h(uint16_t h);

  uint16_t get_tx();
  void     set_tx(uint16_t tx);
  uint16_t get_ty();
  void     set_ty(uint16_t ty);

  uint16_t get_fg();
  void     set_fg(uint16_t fg);
  uint16_t get_bg();
  void     set_bg(uint16_t bg);

  const char *get_text();
  void        set_text(const char *text);

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);

  void highlight(bool highlight);
  bool is_highlighted();

  void wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  bool is_pressed(TouchCtrl &tch, TftCtrl &tft);

private:
  uint16_t m_x, m_y;
  uint16_t m_w, m_h;
  uint16_t m_tx, m_ty;
  uint16_t m_fg, m_bg;

  bool m_is_highlighted = false;
  bool m_was_highlighted = false;

  char m_text[21];
};

class TftMenu {
public:
  TftMenu() {};
  ~TftMenu();

  bool add_btn(TftBtn *btn);
  bool rm_btn(uint8_t btn_idx);
  bool set_btn(uint8_t btn_idx, TftBtn *btn);
  TftBtn *get_btn(uint8_t btn_idx);
  bool purge_btn(uint8_t btn_idx);
  void purge_btns();

  uint8_t get_num_btns();

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);

  uint8_t wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  int16_t get_pressed(TouchCtrl &tch, TftCtrl &tft);

protected:
  TftBtn **m_btns = nullptr;
  uint8_t m_num_btns = 0;
};

template<typename T>
class TftHexSelMenu : public TftMenu {
public:
  TftHexSelMenu(TftCtrl &tft, uint16_t top_margin, uint16_t side_margin) {
    const uint16_t cell_margin = 10;
    const uint16_t cell_size = (tft.width() - 7*cell_margin - 2*side_margin) / 8;
    const uint16_t cell_dist = cell_size + cell_margin;
    const uint16_t text_margin = (cell_size - 10) / 2;

    for (uint8_t i = 0x00; i < 0x10; ++i) {
      uint16_t x = side_margin + cell_dist * (i % 8);
      uint16_t y = (i < 8 ? top_margin : top_margin + cell_size + cell_margin);
  
      add_btn(new TftBtn(x, y, cell_size, cell_size, text_margin, text_margin, STRFMT_NOBUF("%1X", i), TftColor::WHITE, TftColor::BLUE));
    }
  }

  void update_val(uint8_t k) {
    m_val = (m_val << 4) + k;
  }

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t font_size, uint16_t fg, uint16_t bg) {
    tft.fillRect(x, y, tft.width() - x, 7 * font_size, bg);

    char strfmt_buf[50];
    sprintf(strfmt_buf, "Val: [%%0%dX]", BIT_WIDTH(T) / 4);

    tft.drawText(x, y, STRFMT_NOBUF(strfmt_buf, m_val), fg, font_size);
  }

  T    get_val()      { return m_val; }
  void set_val(T val) { m_val = val;  }

private:
  T m_val = 0;
};

class TftYesNoMenu : public TftMenu {
public:
  TftYesNoMenu(TftCtrl &tft, uint16_t top_margin, uint16_t side_margin);
};

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft);

#endif
