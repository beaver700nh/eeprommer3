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

TftBtn::TftBtn(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_fg(fg), m_bg(bg) {
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
    IN_RANGE(p.x, m_x, m_x + m_w) &&
    IN_RANGE(p.y, m_y, m_y + m_h)
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

uint8_t TftMenu::get_num_btns() {
  return m_num_btns;
}

void TftMenu::draw(TftCtrl &tft) {
  if (!m_btns[0]->is_highlighted()) {
    m_btns[0]->highlight(true);
  }

  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw(tft);
  }
}

uint8_t TftMenu::is_pressed(TouchCtrl &tch, TftCtrl &tft) {
  (void) tch;
  (void) tft;

  return false;
}

uint8_t TftMenu::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  (void) tch;
  (void) tft;

  return false;
}
