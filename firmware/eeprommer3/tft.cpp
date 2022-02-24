#include <Arduino.h>
#include "constants.hpp"

#include <stdarg.h>

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

bool TftCtrl::drawRGBBitmapFromFile(
  uint16_t x, uint16_t y, const char *file, uint16_t width, uint16_t height,
  bool swap_endian, bool (*check_skip)()
) {
  bool success = true;

  File f = SD.open(file);
  if (!f) return false;

  size_t row_size_bytes = width * sizeof(uint16_t);
  uint16_t *buf = (uint16_t *) malloc(row_size_bytes);

  if (buf == nullptr) {
    f.close();
    return false;
  }

  for (uint16_t j = 0; j < height; ++j) {
    int16_t res = f.read((uint8_t *) buf, row_size_bytes);

    if (res < 0) {
      success = false;
      break;
    }

    if (swap_endian) {
      for (uint16_t i = 0; i < width; ++i) {
        buf[i] = (buf[i] << 8) | (buf[i] >> 8);
      }
    }

    setAddrWindow(x, y + j, x + width, y + j);
    pushColors(buf, width, true);

    if ((*check_skip)()) break;
  }

  f.close();
  free(buf);
  return success;
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

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg) {
  m_text = text;
}

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg)
  : TftBtn(x, y, w, h, TftCalc::t_center_x(w, text, 2), TftCalc::t_center_y(h, 2), text, fg, bg) {
  // Empty because all work delegated to other ctor
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

const char *TftBtn::get_text()                 { return m_text; }
void        TftBtn::set_text(const char *text) { m_text = text; }

void TftBtn::draw(TftCtrl &tft) {
  m_was_highlighted = m_is_highlighted;
  m_was_visible = m_is_visible;

  if (!m_is_visible) return; // If it is invisible, there is nothing to draw.

  tft.fillRect(m_x, m_y, m_w, m_h, m_bg);
  tft.drawText(m_x + m_tx, m_y + m_ty, m_text, m_fg);

  draw_highlight(tft);
}

void TftBtn::erase(TftCtrl &tft) {
  if (!m_was_visible) return; // If it was invisible, there is nothing to erase.

  if (m_was_highlighted) {
    tft.fillRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::BLACK);
  }
  else {
    tft.fillRect(m_x, m_y, m_w, m_h, TftColor::BLACK);
  }
}

void TftBtn::draw_highlight(TftCtrl &tft) {
  auto color = (m_is_highlighted ? TftColor::YELLOW : TftColor::BLACK);

  tft.drawRect(m_x - 1, m_y - 1, m_w + 2, m_h + 2, color);
  tft.drawRect(m_x - 2, m_y - 2, m_w + 4, m_h + 4, color);
  tft.drawRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, color);
}

void TftBtn::highlight(bool highlight) {
  m_is_highlighted = highlight;
}

bool TftBtn::is_highlighted() {
  return m_is_highlighted;
}

void TftBtn::visibility(bool visibility) {
  m_is_visible = visibility;
}

bool TftBtn::is_visible() {
  return m_is_visible;
}

void TftBtn::operation(bool operation) {
  m_is_operational = operation;
}

bool TftBtn::is_operational() {
  return m_is_operational;
}

void TftBtn::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  while (!is_pressed(tch, tft)) {
    /* wait for press */;
  }
}

bool TftBtn::is_pressed(TouchCtrl &tch, TftCtrl &tft) {
  if (!m_is_operational) return false; // Ignore presses if non-operational.

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
  auto new_arr = (TftBtn **) malloc((m_num_btns + 1) * sizeof(TftBtn *));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_btns, m_num_btns * sizeof(TftBtn *));
  new_arr[m_num_btns] = btn;

  free(m_btns);

  m_btns = new_arr;
  ++m_num_btns;

  return true;
}

bool TftMenu::rm_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  auto new_arr = (TftBtn **) malloc((m_num_btns - 1) * sizeof(TftBtn *));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_btns, btn_idx * sizeof(TftBtn *));
  memcpy(new_arr + btn_idx, m_btns + btn_idx + 1, (m_num_btns - btn_idx - 1) * sizeof(TftBtn *));

  free(m_btns);

  m_btns = new_arr;
  --m_num_btns;

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

bool TftMenu::purge_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  TftBtn *to_del = m_btns[btn_idx];

  if (!rm_btn(btn_idx)) return false;
  delete to_del;

  return true;
}

void TftMenu::purge_btns() {
  while (m_num_btns > 0) {
    purge_btn(m_num_btns - 1); // Purge last button in array
  }

  m_num_btns = 0;
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

TftKeyboardLayout::TftKeyboardLayout(const uint8_t *layout, uint8_t length, uint8_t width)
  : m_layout(layout), m_length(length), m_width(width) {
  // Empty
}

const uint8_t *TftKeyboardLayout::get_layout() {
  return m_layout;
}

uint8_t TftKeyboardLayout::get_width() {
  return m_width;
}

uint8_t TftKeyboardLayout::get_height() {
  return ceil((float) m_length / (float) m_width);
}

const char *const TftKeyboardLayout::get_ptr_char(uint8_t x, uint8_t y) {
  return (char *) m_layout + 2 * (y * m_width + x);
}

char TftKeyboardLayout::get_char(uint8_t x, uint8_t y) {
  return *get_ptr_char(x, y);
}

TftKeyboardLayout &get_glob_kbd_hex_layout() {
  static TftKeyboardLayout layout(
    (const uint8_t *)
    "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
    "\x38\x00\x39\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00",
    16, 8
  );

  return layout;
}

TftKeyboardLayout &get_glob_kbd_str_layout() {
  static TftKeyboardLayout layout(
    (const uint8_t *)
    "\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00\x38\x00\x39\x00\x30\x00\x7e\x00"
    "\x51\x00\x57\x00\x45\x00\x52\x00\x54\x00\x59\x00\x55\x00\x49\x00\x4f\x00\x50\x00\x11\x00"
    "\x41\x00\x53\x00\x44\x00\x46\x00\x47\x00\x48\x00\x4a\x00\x4b\x00\x4c\x00\x5f\x00\x2d\x00"
    "\x7f\x00\x5a\x00\x58\x00\x43\x00\x56\x00\x42\x00\x4e\x00\x4d\x00\xb0\x00\x2c\x00\x2e\x00",
    44, 11
  );

  return layout;
}

TftKeyboardMenu::TftKeyboardMenu(
  TftCtrl &tft, uint8_t t_debounce,
  uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h,
  TftKeyboardLayout &layout, float btn_height = 1.2
)
  : m_t_debounce(t_debounce), m_layout(layout),
  m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h) {
  uint16_t cell_width = TftCalc::fraction(tft.width() - 2 * marg_h + 2 * pad_h, pad_h, layout.get_width());
  uint16_t cell_height = (float) cell_width * btn_height;

  for (uint8_t row = 0; row < layout.get_height(); ++row) {
    for (uint8_t col = 0; col < layout.get_width(); ++col) {
      uint16_t x = marg_h + col * (cell_width  + pad_h);
      uint16_t y = marg_v + row * (cell_height + pad_v);

      add_btn(new TftBtn(x, y, cell_width, cell_height, layout.get_ptr_char(col, row), TftColor::WHITE, TftColor::BLUE));
    }
  }

  add_btn(new TftBtn(BOTTOM_BTN(tft, "Continue")));
}

TftKeyboardMenu::~TftKeyboardMenu() {
  free(m_val);
};

void TftKeyboardMenu::update_val(char c) {
  uint8_t len = strlen(m_val);

  if (len >= BUF_LEN()) return;

  m_val[len]     = c;
  m_val[len + 1] = '\0';
}

void TftKeyboardMenu::show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg) {
  tft.fillRect(x, y, tft.width() - x, 8 * size, bg);

  char fmt_str[16];
  sprintf(fmt_str, "[%%-%d.%ds]", len, len);
  tft.drawText(x, y, STRFMT_NOBUF(fmt_str, m_val), fg, size);
}

void TftKeyboardMenu::get_val(char *buf, uint8_t len) {
  strncpy(buf, m_val, len);
}

char *TftKeyboardMenu::get_ptr_val() {
  return m_val;
}

void TftKeyboardMenu::set_val(const char *buf, uint8_t len) {
  strncpy(m_val, buf, len);
}

TftKeyboardLayout &TftKeyboardMenu::get_layout() {
  return m_layout;
}

TftStringMenu::TftStringMenu(
  TftCtrl &tft, uint8_t debounce,
  uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h,
  uint8_t buf_len
)
  : TftKeyboardMenu(tft, debounce, pad_v, pad_h, marg_v, marg_h, get_glob_kbd_str_layout()), m_buf_len(buf_len) {
  m_val = (char *) malloc((BUF_LEN() + 2) * sizeof(char));
  m_val[0] = '\0';
}

int16_t TftStringMenu::get_pressed(TouchCtrl &tch, TftCtrl &tft) {
  auto retval = TftMenu::get_pressed(tch, tft);

  if (retval > 0) {
    unsigned long t_since_last_press = millis() - m_t_last_press;

    if (t_since_last_press > m_t_debounce) {
      m_t_last_press = millis();
    }
    else {
      return -1; // Ignore press because insufficient elapsed time
    }
  }

  return retval;
}

void TftStringMenu::update_val(char c) {
  uint8_t len = strlen(m_val);

  switch (c) {
  case '\x7f':
    m_capitalize = true;
    break;

  case '\x11':
    if (len == 0) return;

    m_val[len - 1] = '\0';

    break;

  case '\xb0':
    update_val(' ');
    break;

  default:
    if (len >= BUF_LEN()) return;

    m_val[len]     = capitalize(c);
    m_val[len + 1] = '\0';

    m_capitalize = false;

    break;
  }
}

char TftStringMenu::capitalize(char c) {
  if (m_capitalize) {
    return toupper(c);
  }
  else {
    return tolower(c);
  }
}

TftChoiceMenu::TftChoiceMenu(
  uint8_t pad_v, uint8_t pad_h,
  uint8_t marg_v, uint8_t marg_h,
  uint8_t num_cols, float btn_height,
  bool btn_height_px, uint8_t initial_choice
)
  : m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h), m_num_cols(num_cols),
  m_btn_height(btn_height), m_btn_height_px(btn_height_px), m_cur_choice(initial_choice) {
  // Empty
}

void TftChoiceMenu::set_callback(Callback callback) {
  m_callback = callback;
}

TftChoiceMenu::Callback TftChoiceMenu::get_callback() {
  return m_callback;
}

bool TftChoiceMenu::add_btn(TftBtn *btn) {
  bool retval = TftMenu::add_btn(btn);

  if (m_num_btns - 1 == m_cur_choice) {
    get_btn(m_num_btns - 1)->highlight(true);
  }

  return retval;
}

bool TftChoiceMenu::add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg) {
  uint8_t col = m_num_btns % m_num_cols;
  uint8_t row = m_num_btns / m_num_cols;

  uint16_t w = TftCalc::fraction(tft.width() - 2 * m_marg_h + 2 * m_pad_h, m_pad_h, m_num_cols);
  uint16_t h = (m_btn_height_px ? (uint16_t) m_btn_height : (float) w * m_btn_height);

  uint16_t x = m_marg_h + col * (w + m_pad_h);
  uint16_t y = m_marg_v + row * (h + m_pad_v);

  return add_btn(new TftBtn(x, y, w, h, text, fg, bg));
}

bool TftChoiceMenu::add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg, uint16_t bg) {
  m_confirm_btn = m_num_btns;

  uint16_t y = TftCalc::bottom(tft, 24, (force_bottom ? 10 : m_marg_v));
  uint16_t w = TftCalc::fraction_x(tft, m_marg_h, 1);

  return add_btn(new TftBtn(m_marg_h, y, w, 24, "Confirm", fg, bg));
}

uint8_t TftChoiceMenu::wait_for_value(TouchCtrl &tch, TftCtrl &tft) {
  draw(tft);

  while (true) {
    uint8_t btn_pressed = wait_for_press(tch, tft);
    (*m_callback)(tft, btn_pressed, btn_pressed == m_confirm_btn);

    if (btn_pressed == m_confirm_btn) break;

    m_old_choice = m_cur_choice;
    m_cur_choice = (uint8_t) btn_pressed;
    get_btn(m_old_choice)->highlight(false);
    get_btn(m_cur_choice)->highlight(true);

    update(tft);
  }

  return m_cur_choice;
}

void TftChoiceMenu::update(TftCtrl &tft) {
  get_btn(m_old_choice)->draw_highlight(tft);
  get_btn(m_cur_choice)->draw_highlight(tft);
}

TftYesNoMenu::TftYesNoMenu(
  TftCtrl &tft,
  uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h,
  bool force_bottom, uint8_t initial_choice
)
  : TftChoiceMenu(pad_v, pad_h, marg_v, marg_h, 2, 0.7, false, initial_choice) {
  add_btn_calc(tft, "Yes", TftColor::BLACK, TftColor::GREEN);
  add_btn_calc(tft, "No",  TftColor::WHITE, TftColor::RED);
  add_btn_confirm(tft, force_bottom);
}

uint8_t ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt,
  int8_t cols, int32_t btn_height, int16_t initial_choice,
  uint8_t num, ...
) {
  va_list args;
  va_start(args, num);

  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  if (cols < 0) {
    cols = (int8_t) floor(sqrt(num));

    // param num...   |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | etc...
    // ---------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+------~
    // calc'd cols... |  1 |  1 |  1 |  2 |  2 |  2 |  2 |  2 |  3 |  3 |  3 |  3 |  3 |  3 |  3 |  4 | etc...
  }

  TftChoiceMenu menu(10, 10, 50, 10, cols, (btn_height < 0 ? 24 : btn_height), true, (initial_choice < 0 ? 0 : initial_choice));

  while (num --> 0) {
    const char *text  = va_arg(args, const char *);
    uint16_t fg_color = va_arg(args, uint16_t);
    uint16_t bg_color = va_arg(args, uint16_t);

    menu.add_btn_calc(tft, text, fg_color, bg_color);
  }

  menu.add_btn_confirm(tft, true);

  va_end(args);

  uint8_t val = menu.wait_for_value(tch, tft);
  return val;
}

void ask_str(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *buf, uint8_t len) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 4);

  TftStringMenu menu(tft, T_DEBOUNCE, 10, 10, 50, 10, len);
  menu.draw(tft);

  while (true) { // Loop to get a val
    menu.show_val(tft, 10, 240, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == menu.get_num_btns() - 1) break;

    auto w = menu.get_layout().get_width();
    char ch = menu.get_layout().get_char(btn_pressed % w, btn_pressed / w);

    menu.update_val(ch);
  }

  menu.get_val(buf, len);
}

bool ask_yesno(TftCtrl &tft, TouchCtrl &tch, const char *prompt, int16_t initial_choice) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 4);

  TftYesNoMenu menu(tft, 10, 10, 50, 10, true, (initial_choice < 0 ? 0 : initial_choice));
  uint8_t btn_pressed = menu.wait_for_value(tch, tft);

  return btn_pressed == 0;
}

void tft_draw_test(TouchCtrl &tch, TftCtrl &tft) {
  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (TouchCtrl::is_valid_pressure(p.z)) {
      tft.fillCircle(p.x, p.y, 3, TftColor::RED);
    }
  }
}

#ifdef DEBUG_MODE

void tft_print_chars(TftCtrl &tft) {
  uint8_t i = 0;

  do {
    tft.drawText(10 + 7 * (i & 0x0F), 10 + 10 * (i >> 4), STRFMT_NOBUF("%c", i), TftColor::WHITE, 1);
  }
  while (i++ != 0xFF);
}

#endif
