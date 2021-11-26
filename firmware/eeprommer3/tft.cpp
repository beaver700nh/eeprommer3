#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>
#include <SD.h>
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

bool TftCtrl::drawRGBBitmapFromFile(uint16_t x, uint16_t y, const char *file, uint16_t width, uint16_t height, int32_t transparent) {
  File f = SD.open(file);
  if (!f) return false;

  size_t row_size_bytes = width * sizeof(uint16_t);

  uint16_t *buf = (uint16_t *) malloc(row_size_bytes);
  if (buf == nullptr) return false;

  for (uint16_t j = 0; j < height; ++j) {
    int16_t res = f.read((uint8_t *) buf, row_size_bytes);

    if (res < 0) {
      free(buf);
      return false;
    }

    for (uint16_t i = 0; i < width; ++i) {
      if (buf[i] != transparent) {
        writePixel(x + i, y + j, (buf[i] << 8) | (buf[i] >> 8));
      }
    }
  }

  free(buf);
  return true;
}

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg) {
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

uint16_t TftBtn::get_tx()            { return m_tx; }
void     TftBtn::set_tx(uint16_t tx) { m_tx = tx;   }
uint16_t TftBtn::get_ty()            { return m_ty; }
void     TftBtn::set_ty(uint16_t ty) { m_ty = ty;   }

uint16_t TftBtn::get_fg()            { return m_fg; }
void     TftBtn::set_fg(uint16_t fg) { m_fg = fg;   }
uint16_t TftBtn::get_bg()            { return m_bg; }
void     TftBtn::set_bg(uint16_t bg) { m_bg = bg;   }

const char *TftBtn::get_text() { return m_text; }
void        TftBtn::set_text(const char *text) {
  strncpy(m_text, text, 20);
}

void TftBtn::draw(TftCtrl &tft) {
  m_was_highlighted = m_is_highlighted;

  tft.fillRect(m_x, m_y, m_w, m_h, m_bg);
  tft.drawText(m_x + m_tx, m_y + m_ty, m_text, m_fg);

  if (m_is_highlighted) {
    tft.drawRect(m_x - 1, m_y - 1, m_w + 2, m_h + 2, TftColor::YELLOW);
    tft.drawRect(m_x - 2, m_y - 2, m_w + 4, m_h + 4, TftColor::YELLOW);
    tft.drawRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::YELLOW);
  }
}

void TftBtn::erase(TftCtrl &tft) {
  if (m_was_highlighted) {
    tft.fillRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::BLACK);
  }
  else {
    tft.fillRect(m_x, m_y, m_w, m_h, TftColor::BLACK);
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

TftMenu::~TftMenu() {
  purge_btns();
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

  auto new_arr = (TftBtn **) malloc((m_num_btns - 1) * sizeof(TftBtn *));

  if (new_arr == nullptr) {
    return false;
  }

  memcpy(new_arr, m_btns, btn_idx * sizeof(TftBtn *));
  memcpy(new_arr + btn_idx, m_btns + btn_idx + 1, (m_num_btns - btn_idx - 1) * sizeof(TftBtn *));

  free(m_btns);

  m_btns = new_arr;

  --m_num_btns;

  return true;
}

bool TftMenu::set_btn(uint8_t btn_idx, TftBtn *btn) {
  if (m_num_btns == 0 || btn_idx >= m_num_btns) return false;

  m_btns[btn_idx] = btn;
  return true;
}

TftBtn *TftMenu::get_btn(uint8_t btn_idx) {
  if (m_num_btns == 0 || btn_idx >= m_num_btns) return nullptr;

  return m_btns[btn_idx];
}

bool TftMenu::purge_btn(uint8_t btn_idx) {
  if (m_num_btns == 0 || btn_idx >= m_num_btns) return false;

  TftBtn *to_del = m_btns[btn_idx];
  if (!rm_btn(btn_idx)) return false;
  delete to_del;
  return true;
}

void TftMenu::purge_btns() {
  while (m_num_btns > 0) {
    purge_btn(m_num_btns - 1); // Purge last button in array
  }
}

uint8_t TftMenu::get_num_btns() {
  return m_num_btns;
}

void TftMenu::draw(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw(tft);
  }
}

void TftMenu::erase(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->erase(tft);
  }
}

uint8_t TftMenu::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
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

TftChoiceMenu::TftChoiceMenu(
  uint8_t v_margin, uint8_t h_margin,
  uint8_t v_padding, uint8_t h_padding,
  uint8_t num_cols, uint16_t btn_height,
  uint8_t initial_choice
)
  : m_v_margin(v_margin), m_h_margin(h_margin),
  m_v_padding(v_padding), m_h_padding(h_padding),
  m_num_cols(num_cols), m_btn_height(btn_height),
  m_cur_choice(initial_choice) {
  // Empty
}

bool TftChoiceMenu::add_btn(TftBtn *btn) {
  bool retval = TftMenu::add_btn(btn);
  if (m_num_btns - 1 == m_cur_choice) {
    get_btn(m_num_btns - 1)->highlight(true);
  }

  return retval;
}

bool TftChoiceMenu::add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg) {
  uint16_t x, y, w, h, tx, ty;

  uint8_t col = m_num_btns % m_num_cols;
  uint8_t row = m_num_btns / m_num_cols;

  h = m_btn_height;
  w = (tft.width() - (2 * m_h_margin) - ((m_num_cols - 1) * m_h_padding)) / m_num_cols;

  x = m_h_margin + col * (w + m_h_padding);
  y = m_v_margin + row * (h + m_v_padding);

  tx = (w / 2) - (6 * strlen(text) - 1);
  ty = (h / 2) - 7;

  return add_btn(new TftBtn(x, y, w, h, tx, ty, text, fg, bg));
}

bool TftChoiceMenu::add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg, uint16_t bg) {
  m_confirm_btn = m_num_btns;

  uint16_t y, w, tx;

  y = tft.height() - (force_bottom ? 10 : m_v_margin) - 24;
  w = tft.width() - (2 * m_h_margin);
  tx = (w / 2) - 41;

  return add_btn(new TftBtn(m_h_margin, y, w, 24, tx, 5, "Confirm", fg, bg));
}

uint8_t TftChoiceMenu::wait_for_value(TouchCtrl &tch, TftCtrl &tft) {
  while (true) {
    erase(tft);
    draw(tft);

    uint8_t btn_pressed = wait_for_press(tch, tft);

    if (btn_pressed == m_confirm_btn) break;

    get_btn(m_cur_choice)->highlight(false); // Old "current" choice
    m_cur_choice = (uint8_t) btn_pressed;
    get_btn(m_cur_choice)->highlight(true);
  }

  return m_cur_choice;
}

TftYesNoMenu::TftYesNoMenu(
  TftCtrl &tft,
  uint8_t v_margin, uint8_t h_margin,
  uint8_t v_padding, uint8_t h_padding,
  bool force_bottom, uint8_t initial_choice
)
  : TftChoiceMenu(v_margin, h_margin, v_padding, h_padding, 2, 120, initial_choice) {
  add_btn_calc(tft, "Yes", TftColor::BLACK, TftColor::GREEN);
  add_btn_calc(tft, "No",  TftColor::WHITE, TftColor::RED);
  add_btn_confirm(tft, force_bottom);
}

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft) {
  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (TouchCtrl::is_valid_pressure(p.z)) {
      tft.fillCircle(p.x, p.y, 3, TftColor::RED);
    }
  }
}
