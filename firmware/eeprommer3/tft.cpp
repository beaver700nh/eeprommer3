#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "tft.hpp"
#include "input.hpp"

void TftCtrl::init(uint16_t driver_id, uint8_t orientation) {
  reset();

  delay(500);

  begin(driver_id);
  setRotation(orientation);
}

void TftCtrl::drawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t size) {
  setTextColor(color);
  setTextSize(size);
  setCursor(x, y);
  print(text);
}

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_fg(fg), m_bg(bg) {
  strncpy(m_text, text, 20);
}

uint16_t TftBtn::get_x()           { return m_x; }
void     TftBtn::set_x(uint16_t x) { m_x = x;    }
uint16_t TftBtn::get_y()           { return m_y; }
void     TftBtn::set_y(uint16_t y) { m_y = y;    }

uint16_t TftBtn::get_w()           { return m_w; }
void     TftBtn::set_w(uint16_t w) { m_w = w;    }
uint16_t TftBtn::get_h()           { return m_h; }
void     TftBtn::set_h(uint16_t h) { m_h = h;    }

uint16_t TftBtn::get_fg()            { return m_fg; }
void     TftBtn::set_fg(uint16_t fg) { m_fg = fg;   }
uint16_t TftBtn::get_bg()            { return m_bg; }
void     TftBtn::set_bg(uint16_t bg) { m_bg = bg;   }

const char *TftBtn::get_text() { return m_text; }
void        TftBtn::set_text(const char *text) {
  strncpy(m_text, text, 20);
}

void TftBtn::draw(TftCtrl &tft) {
  tft.fillRect(m_x, m_y, m_w, m_h, m_bg);
  tft.drawText(m_x + 3, m_y + 3, m_text, m_fg);

  if (m_is_highlighted) {
    tft.drawRect(m_x - 1, m_y - 1, m_w + 2, m_h + 2, TftColor::YELLOW);
    tft.drawRect(m_x - 2, m_y - 2, m_w + 4, m_h + 4, TftColor::YELLOW);
    tft.drawRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::YELLOW);
  }
}

void TftBtn::highlight(bool highlight) {
  m_is_highlighted = highlight;
}

bool TftBtn::is_highlighted() {
  return m_is_highlighted;
}

void TftBtn::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  while (!is_pressed(tch, tft)) {
    /* wait for press */;
  }
}

bool TftBtn::is_pressed(TouchCtrl &tch, TftCtrl &tft) {
  TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

  return (
    tch.is_valid_pressure(p.z) &&
    IN_RANGE(p.x, (int32_t) m_x, (int32_t) m_x + m_w) &&
    IN_RANGE(p.y, (int32_t) m_y, (int32_t) m_y + m_h)
  );
}

bool TftMenu::add_btn(TftBtn *btn) {
  ++m_num_btns;

  auto new_arr = (TftBtn **) malloc(m_num_btns * sizeof(TftBtn *));

  if (new_arr == nullptr) {
    return false;
  }

  memcpy(new_arr, m_btns, m_num_btns * sizeof(TftBtn *));
  new_arr[m_num_btns - 1] = btn;

  free(m_btns);

  m_btns = new_arr;

  return true;
}

bool TftMenu::rm_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  --m_num_btns;

  auto new_arr = (TftBtn **) malloc(m_num_btns * sizeof(TftBtn *));

  if (new_arr == nullptr) {
    return false;
  }

  memcpy(new_arr, m_btns, btn_idx * sizeof(TftBtn *));
  memcpy(new_arr + btn_idx, m_btns + btn_idx + 1, (m_num_btns - btn_idx - 1) * sizeof(TftBtn *));

  free(m_btns);

  m_btns = new_arr;

  return true;
}

bool TftMenu::set_btn(uint8_t btn_idx, TftBtn *btn) {
  if (btn_idx >= m_num_btns) return false;

  m_btns[btn_idx] = btn;
  return true;
}

TftBtn *TftMenu::get_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return nullptr;

  return m_btns[btn_idx];
}

uint8_t TftMenu::get_num_btns() {
  return m_num_btns;
}

void TftMenu::draw(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw(tft);
  }
}

int16_t TftMenu::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  int16_t btn = 0;

  do {
    btn = get_pressed(tch, tft);
  }
  while (btn < 0);

  return btn;
}

int16_t TftMenu::get_pressed(TouchCtrl &tch, TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    if (m_btns[i]->is_pressed(tch, tft)) {
      return i;
    }
  }

  return -1;
}

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft) {
  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (TouchCtrl::is_valid_pressure(p.z)) {
      tft.fillCircle(p.x, p.y, 3, TftColor::RED);
    }
  }
}
