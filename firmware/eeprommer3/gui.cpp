#include <Arduino.h>
#include "constants.hpp"

#include <stdarg.h>

#include "touch.hpp"

#include "gui.hpp"

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg) {
  m_text = text;
}

TftBtn::TftBtn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg)
  : TftBtn(x, y, w, h, 0, 0, text, fg, bg) { // Passes `tx/ty` as (0, 0) temporarily
  m_auto_center = true;
  auto_center(); // Overwrites dummy (0, 0) `tx/ty` with real center
}

void TftBtn::draw(TftCtrl &tft) {
  m_was_highlighted = m_is_highlighted;
  m_was_visible = m_is_visible;

  if (!m_is_visible) return; // If it is invisible, there is nothing to draw.

  uint16_t fg = (m_is_operational ? m_fg : TftColor::DGRAY);
  uint16_t bg = (m_is_operational ? m_bg : TftColor::GRAY);

  tft.fillRect(m_x, m_y, m_w, m_h, bg);
  tft.drawText(m_x + m_tx, m_y + m_ty, m_text, fg, m_font_size);

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
  auto color = ([=]() -> uint16_t {
    if (!m_is_highlighted) {
      return TftColor::BLACK;
    }

    if (!m_is_operational) {
      return TftColor::LGRAY;
    }

    uint16_t res = ~m_bg;

    // Prevent gray-ish colors
    if (IN_RANGE(RED_565(res), 0x0C, 0x10) || IN_RANGE(GRN_565(res), 0x18, 0x20) || IN_RANGE(BLU_565(res), 0x0C, 0x10)) {
      res &= ~0x2104;
    }
    else if (IN_RANGE(RED_565(res), 0x10, 0x14) || IN_RANGE(GRN_565(res), 0x20, 0x28) || IN_RANGE(BLU_565(res), 0x10, 0x14)) {
      res |= 0x39E7;
    }

    // Prevent too-dark colors
    if (res < 0x4000) res |= 0x39E7;

    return res;
  })();

  tft.drawThickRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, color, 3);
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

void TftMenu::deselect_all() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->highlight(false);
  }
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
  TftCtrl &tft, uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, TftKeyboardLayout &layout, float btn_height
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
  strncpy(buf, m_val, len + 1);
}

char *TftKeyboardMenu::get_ptr_val() {
  return m_val;
}

void TftKeyboardMenu::set_val(const char *buf, uint8_t len) {
  strncpy(m_val, buf, len);
}

bool TftKeyboardMenu::handle_key(uint8_t key) {
  UNUSED_VAR(key);

  unsigned long t_since_last_press = millis() - m_t_last_press;

  if (t_since_last_press > m_t_debounce) {
    m_t_last_press = millis();
    return true;
  }

  return false;
}

TftKeyboardLayout &TftKeyboardMenu::get_layout() {
  return m_layout;
}

TftStringMenu::TftStringMenu(
  TftCtrl &tft, uint16_t debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, uint8_t buf_len
)
  : TftKeyboardMenu(tft, debounce, pad_v, pad_h, marg_v, marg_h, get_glob_kbd_str_layout()), m_buf_len(buf_len) {
  m_val = (char *) malloc((BUF_LEN() + 1) * sizeof(char));
  m_val[0] = '\0';
}

void TftStringMenu::update_val(char c) {
  uint8_t len = strlen(m_val);

  switch (c) {
  case '\x7f':
    m_capitalize = !m_capitalize;
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

void TftStringMenu::show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
  // Show caps indicator
  tft.drawTextBg(TftCalc::right(tft, 10, 10), 10, (m_capitalize ? "A" : "a"));

  uint8_t char_width = 6 * (size == 0 ? 1 : size); // Prevent divide-by-zero
  uint8_t maxfit_len = (tft.width() - size - 2 * m_marg_h) / char_width - 2;

  uint8_t working_text_len = MIN(maxfit_len, m_buf_len);

  TftKeyboardMenu::show_val(tft, x, y, working_text_len, size, fg, bg);
}

bool TftStringMenu::handle_key(uint8_t key) {
  if (!TftKeyboardMenu::handle_key(key)) return false;

  auto w = get_layout().get_width();
  char ch = get_layout().get_char(key % w, key / w);

  update_val(ch);
  return true;
}

char TftStringMenu::capitalize(char c) {
  if (m_capitalize) {
    return toupper(c);
  }
  else {
    return tolower(c);
  }
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
  set_confirm_btn(m_num_btns);

  uint16_t y = TftCalc::bottom(tft, 24, (force_bottom ? 10 : m_marg_v));
  uint16_t w = TftCalc::fraction_x(tft, m_marg_h, 1);

  return add_btn(new TftBtn(m_marg_h, y, w, 24, "Confirm", fg, bg));
}

void TftChoiceMenu::set_confirm_btn(uint8_t btn_id) {
  m_confirm_btn = btn_id;
}

uint8_t TftChoiceMenu::wait_for_value(TouchCtrl &tch, TftCtrl &tft) {
  draw(tft);

  while (true) {
    uint8_t btn_pressed = wait_for_press(tch, tft);
    (*m_callback)(tft, btn_pressed, btn_pressed == m_confirm_btn);

    if (btn_pressed == m_confirm_btn) {
      if (get_btn(m_cur_choice)->is_operational()) {
        break;
      }
    }
    else {
      select(btn_pressed);
      update(tft);
    }
  }

  return m_cur_choice;
}

void TftChoiceMenu::select(uint8_t choice) {
  set_choice(choice);
  get_btn(m_old_choice)->highlight(false);
  get_btn(choice)->highlight(true);
}

void TftChoiceMenu::set_choice(uint8_t btn) {
  m_old_choice = m_cur_choice;
  m_cur_choice = btn;
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

TftProgressIndicator::TftProgressIndicator(
  TftCtrl &tft, uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color_frac, uint16_t color_perc, uint16_t color_bar1, uint16_t color_bar2
)
  : m_tft(tft), m_max_val(max_val), m_x(x), m_y(y), m_w(w), m_h(h),
  m_color_frac(color_frac), m_color_perc(color_perc), m_color_bar1(color_bar1), m_color_bar2(color_bar2) {
  m_tft.drawRect(x,     y,     w,     h,     m_color_bar1);
  m_tft.drawRect(x + 1, y + 1, w - 2, h - 2, m_color_bar1);
}

void TftProgressIndicator::show() {
  if (m_cur_val > m_max_val) return;

  double fraction = (double) m_cur_val / (double) m_max_val;
  uint16_t progress = (m_w - 4) * fraction;

  char text[32];
  snprintf(text, 31, "%d/%d       ", m_cur_val, m_max_val);

  uint16_t ty = m_y + TftCalc::t_center_y(m_h, 2);
  uint16_t tx = m_x + TftCalc::t_center_x(m_w, text, 2);

  m_tft.fillRect(m_x + 2 + progress, ty, m_w - 4 - progress, 16, TftColor::BLACK);
  m_tft.fillRect(m_x + 2, m_y + 2, progress, m_h - 4, m_color_bar2);

  m_tft.drawText(tx, ty, text, m_color_frac);
  tx += (strlen(text) - 6) * 12;
  m_tft.drawText(tx, ty, STRFMT_NOBUF("(%03d%%)", uint8_t(fraction * 100.0)), m_color_perc);
}

void TftProgressIndicator::next() {
  ++m_cur_val;
}

uint8_t ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt, int8_t cols, int32_t btn_height, int16_t initial_choice, uint8_t num, ...
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
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  TftStringMenu menu(tft, T_DEBOUNCE, 10, 10, 50, 10, len);
  menu.draw(tft);

  while (true) { // Loop to get a val
    menu.show_val(tft, 10, 240, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == menu.get_num_btns() - 1) break;

    menu.handle_key(btn_pressed);
  }

  menu.get_val(buf, len);
}

bool ask_yesno(TftCtrl &tft, TouchCtrl &tch, const char *prompt, int16_t initial_choice) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  TftYesNoMenu menu(tft, 10, 10, 50, 10, true, (initial_choice < 0 ? 0 : initial_choice));
  uint8_t btn_pressed = menu.wait_for_value(tch, tft);

  return btn_pressed == 0;
}