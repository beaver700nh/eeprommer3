#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "gui.hpp"

Gui::Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg)
  : m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg) {
  m_text = text;
}

Gui::Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg)
  : Btn(x, y, w, h, 0, 0, text, fg, bg) { // Passes `tx/ty` as (0, 0) temporarily
  m_auto_center = true;
  auto_center(); // Overwrites dummy (0, 0) `tx/ty` with real center
}

void Gui::Btn::draw(TftCtrl &tft) {
  m_was_highlighted = m_is_highlighted;
  m_was_visible = m_is_visible;

  if (!m_is_visible) return; // If it is invisible, there is nothing to draw.

  uint16_t fg = (m_is_operational ? m_fg : TftColor::DGRAY);
  uint16_t bg = (m_is_operational ? m_bg : TftColor::GRAY);

  tft.fillRect(m_x, m_y, m_w, m_h, bg);
  tft.drawText(m_x + m_tx, m_y + m_ty, m_text, fg, m_font_size);

  draw_highlight(tft);
}

void Gui::Btn::erase(TftCtrl &tft) {
  if (!m_was_visible) return; // If it was invisible, there is nothing to erase.

  if (m_was_highlighted) {
    tft.fillRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::BLACK);
  }
  else {
    tft.fillRect(m_x, m_y, m_w, m_h, TftColor::BLACK);
  }
}

void Gui::Btn::draw_highlight(TftCtrl &tft) {
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

void Gui::Btn::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  while (!is_pressed(tch, tft)) {
    /* wait for press */;
  }
}

bool Gui::Btn::is_pressed(TouchCtrl &tch, TftCtrl &tft) {
  if (!m_is_operational) return false; // Ignore presses if non-operational.

  TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

  return (
    tch.is_valid_pressure(p.z) &&
    IN_RANGE(p.x, (int32_t) m_x, (int32_t) m_x + m_w) &&
    IN_RANGE(p.y, (int32_t) m_y, (int32_t) m_y + m_h)
  );
}

Gui::Menu::~Menu() {
  purge_btns();
  SER_LOG_PRINT("[Menu destructor called.]\n");
}

bool Gui::Menu::add_btn(Btn *btn) {
  auto new_arr = (Btn **) malloc((m_num_btns + 1) * sizeof(Btn *));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_btns, m_num_btns * sizeof(Btn *));
  new_arr[m_num_btns] = btn;

  free(m_btns);

  m_btns = new_arr;
  ++m_num_btns;

  return true;
}

bool Gui::Menu::rm_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  auto new_arr = (Btn **) malloc((m_num_btns - 1) * sizeof(Btn *));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_btns, btn_idx * sizeof(Btn *));
  memcpy(new_arr + btn_idx, m_btns + btn_idx + 1, (m_num_btns - btn_idx - 1) * sizeof(Btn *));

  free(m_btns);

  m_btns = new_arr;
  --m_num_btns;

  return true;
}

bool Gui::Menu::set_btn(uint8_t btn_idx, Btn *btn) {
  if (btn_idx >= m_num_btns) return false;

  m_btns[btn_idx] = btn;
  return true;
}

Gui::Btn *Gui::Menu::get_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return nullptr;

  return m_btns[btn_idx];
}

bool Gui::Menu::purge_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  Btn *to_del = m_btns[btn_idx];

  if (!rm_btn(btn_idx)) return false;
  delete to_del;

  return true;
}

void Gui::Menu::purge_btns() {
  while (m_num_btns > 0) {
    purge_btn(m_num_btns - 1); // Purge last button in array
  }

  m_num_btns = 0;
}

uint8_t Gui::Menu::get_num_btns() {
  return m_num_btns;
}

void Gui::Menu::draw(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw(tft);
  }
}

void Gui::Menu::erase(TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->erase(tft);
  }
}

uint8_t Gui::Menu::wait_for_press(TouchCtrl &tch, TftCtrl &tft) {
  int16_t btn = 0;

  do {
    btn = get_pressed(tch, tft);
  }
  while (btn < 0);

  return btn;
}

int16_t Gui::Menu::get_pressed(TouchCtrl &tch, TftCtrl &tft) {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    if (m_btns[i]->is_pressed(tch, tft)) {
      return i;
    }
  }

  return -1;
}

void Gui::Menu::deselect_all() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->highlight(false);
  }
}

const uint8_t *Gui::KeyboardLayout::get_layout() {
  return m_layout;
}

uint8_t Gui::KeyboardLayout::get_width() {
  return m_width;
}

uint8_t Gui::KeyboardLayout::get_height() {
  return ceil((float) m_length / (float) m_width);
}

const char *const Gui::KeyboardLayout::get_ptr_char(uint8_t x, uint8_t y) {
  return (char *) m_layout + 2 * (y * m_width + x);
}

char Gui::KeyboardLayout::get_char(uint8_t x, uint8_t y) {
  return *get_ptr_char(x, y);
}

Gui::KeyboardLayout &Gui::get_glob_kbd_hex_layout() {
  static Gui::KeyboardLayout layout(
    (const uint8_t *)
    "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
    "\x38\x00\x39\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00",
    16, 8
  );

  return layout;
}

Gui::KeyboardLayout &Gui::get_glob_kbd_str_layout() {
  static Gui::KeyboardLayout layout(
    (const uint8_t *)
    "\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00\x38\x00\x39\x00\x30\x00\x7e\x00"
    "\x51\x00\x57\x00\x45\x00\x52\x00\x54\x00\x59\x00\x55\x00\x49\x00\x4f\x00\x50\x00\x11\x00"
    "\x41\x00\x53\x00\x44\x00\x46\x00\x47\x00\x48\x00\x4a\x00\x4b\x00\x4c\x00\x5f\x00\x2d\x00"
    "\x7f\x00\x5a\x00\x58\x00\x43\x00\x56\x00\x42\x00\x4e\x00\x4d\x00\xb0\x00\x2c\x00\x2e\x00",
    44, 11
  );

  return layout;
}

Gui::MenuKeyboard::MenuKeyboard(
  TftCtrl &tft, uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, KeyboardLayout &layout, float btn_height
)
  : m_t_debounce(t_debounce), m_layout(layout),
  m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h) {
  SER_LOG_PRINT("Hey there\n");
  const uint16_t cell_width = TftCalc::fraction(tft.width() - 2 * marg_h + 2 * pad_h, pad_h, layout.get_width());
  const uint16_t cell_height = (float) cell_width * btn_height;
  uint16_t x, y;

  for (uint8_t row = 0; row < layout.get_height(); ++row) {
    for (uint8_t col = 0; col < layout.get_width(); ++col) {
      x = marg_h + col * (cell_width  + pad_h);
      y = marg_v + row * (cell_height + pad_v);

      SER_LOG_PRINT("MenuKeyboard::MenuKeyboard(): r%dc%d\n", row, col);
      Memory::print_ram_analysis();
      add_btn(new Btn(x, y, cell_width, cell_height, layout.get_ptr_char(col, row), TftColor::WHITE, TftColor::BLUE));
      Memory::print_ram_analysis();
    }
  }

  add_btn(new Btn(BOTTOM_BTN(tft, Strings::L_CONTINUE)));
}

Gui::MenuKeyboard::~MenuKeyboard() {
  free(m_val);
};

void Gui::MenuKeyboard::update_val(char c) {
  uint8_t len = strlen(m_val);

  if (len >= BUF_LEN()) return;

  m_val[len]     = c;
  m_val[len + 1] = '\0';
}

void Gui::MenuKeyboard::show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg) {
  tft.fillRect(x, y, tft.width() - x, 8 * size, bg);

  char fmt_str[16];
  sprintf(fmt_str, "[%%-%d.%ds]", len, len);
  tft.drawText(x, y, STRFMT_NOBUF(fmt_str, m_val), fg, size);
}

void Gui::MenuKeyboard::get_val(char *buf, uint8_t len) {
  strncpy(buf, m_val, len);
}

char *Gui::MenuKeyboard::get_ptr_val() {
  return m_val;
}

void Gui::MenuKeyboard::set_val(const char *buf, uint8_t len) {
  strncpy(m_val, buf, len);
}

bool Gui::MenuKeyboard::handle_key(uint8_t key) {
  UNUSED_VAR(key);

  unsigned long t_since_last_press = millis() - m_t_last_press;

  if (t_since_last_press > m_t_debounce) {
    m_t_last_press = millis();
    return true;
  }

  return false;
}

Gui::KeyboardLayout &Gui::MenuKeyboard::get_layout() {
  return m_layout;
}

Gui::MenuStrInput::MenuStrInput(
  TftCtrl &tft, uint16_t debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, uint8_t buf_len
)
  : MenuKeyboard(tft, debounce, pad_v, pad_h, marg_v, marg_h, get_glob_kbd_str_layout()), m_buf_len(buf_len) {
  SER_DEBUG_PRINT((BUF_LEN() + 1) * sizeof(char), 'd');
  m_val = (char *) malloc((BUF_LEN() + 1) * sizeof(char));
  m_val[0] = '\0';
}

void Gui::MenuStrInput::update_val(char c) {
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

void Gui::MenuStrInput::show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
  // Show caps indicator
  tft.drawTextBg(TftCalc::right(tft, 10, 10), 10, (m_capitalize ? "A" : "a"));

  uint8_t char_width = 6 * (size == 0 ? 1 : size); // Prevent divide-by-zero
  uint8_t maxfit_len = (tft.width() - size - 2 * m_marg_h) / char_width - 2;

  uint8_t working_text_len = MIN(maxfit_len, m_buf_len);

  MenuKeyboard::show_val(tft, x, y, working_text_len, size, fg, bg);
}

bool Gui::MenuStrInput::handle_key(uint8_t key) {
  if (!MenuKeyboard::handle_key(key)) return false;

  auto w = get_layout().get_width();
  char ch = get_layout().get_char(key % w, key / w);

  update_val(ch);
  return true;
}

char Gui::MenuStrInput::capitalize(char c) {
  return (m_capitalize ? toupper : tolower)(c);
}

void Gui::MenuChoice::set_callback(Callback callback) {
  m_callback = callback;
}

Gui::MenuChoice::Callback Gui::MenuChoice::get_callback() {
  return m_callback;
}

bool Gui::MenuChoice::add_btn(Btn *btn) {
  bool retval = Menu::add_btn(btn);

  if (m_num_btns - 1 == m_cur_choice) {
    get_btn(m_num_btns - 1)->highlight(true);
  }

  return retval;
}

bool Gui::MenuChoice::add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg) {
  uint8_t col = m_num_btns % m_num_cols;
  uint8_t row = m_num_btns / m_num_cols;

  uint16_t w = TftCalc::fraction(tft.width() - 2 * m_marg_h + 2 * m_pad_h, m_pad_h, m_num_cols);
  uint16_t h = (m_btn_height_px ? (uint16_t) m_btn_height : (float) w * m_btn_height);

  uint16_t x = m_marg_h + col * (w + m_pad_h);
  uint16_t y = m_marg_v + row * (h + m_pad_v);

  return add_btn(new Btn(x, y, w, h, text, fg, bg));
}

bool Gui::MenuChoice::add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg, uint16_t bg) {
  set_confirm_btn(m_num_btns);

  uint16_t y = TftCalc::bottom(tft, 24, (force_bottom ? 10 : m_marg_v));
  uint16_t w = TftCalc::fraction_x(tft, m_marg_h, 1);

  return add_btn(new Btn(m_marg_h, y, w, 24, Strings::L_CONFIRM, fg, bg));
}

void Gui::MenuChoice::set_confirm_btn(uint8_t btn_id) {
  m_confirm_btn = btn_id;
}

uint8_t Gui::MenuChoice::wait_for_value(TouchCtrl &tch, TftCtrl &tft) {
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

void Gui::MenuChoice::select(uint8_t choice) {
  set_choice(choice);
  get_btn(m_old_choice)->highlight(false);
  get_btn(choice)->highlight(true);
}

void Gui::MenuChoice::set_choice(uint8_t btn) {
  m_old_choice = m_cur_choice;
  m_cur_choice = btn;
}

void Gui::MenuChoice::update(TftCtrl &tft) {
  get_btn(m_old_choice)->draw_highlight(tft);
  get_btn(m_cur_choice)->draw_highlight(tft);
}

Gui::MenuYesNo::MenuYesNo(
  TftCtrl &tft,
  uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h,
  bool force_bottom, uint8_t initial_choice
)
  : Gui::MenuChoice(pad_v, pad_h, marg_v, marg_h, 2, 0.7, false, initial_choice) {
  add_btn_calc(tft, Strings::L_YES, TftColor::BLACK, TftColor::GREEN);
  add_btn_calc(tft, Strings::L_NO,  TftColor::WHITE, TftColor::RED);
  add_btn_confirm(tft, force_bottom);
}

Gui::ProgressIndicator::ProgressIndicator(
  TftCtrl &tft, uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color_frac, uint16_t color_perc, uint16_t color_bar1, uint16_t color_bar2
)
  : m_tft(tft), m_max_val(max_val), m_x(x), m_y(y), m_w(w), m_h(h),
  m_color_frac(color_frac), m_color_perc(color_perc), m_color_bar1(color_bar1), m_color_bar2(color_bar2) {
  m_tft.drawRect(x,     y,     w,     h,     m_color_bar1);
  m_tft.drawRect(x + 1, y + 1, w - 2, h - 2, m_color_bar1);
}

void Gui::ProgressIndicator::show() {
  if (m_cur_val > m_max_val) return;

  double fraction = (double) m_cur_val / (double) m_max_val;
  uint16_t progress = (m_w - 4) * fraction;

  const char *text = STRFMT_NOBUF("%d/%d       ", m_cur_val, m_max_val);

  uint16_t ty = m_y + TftCalc::t_center_y(m_h, 2);
  uint16_t tx = m_x + TftCalc::t_center_x(m_w, text, 2);

  m_tft.fillRect(m_x + 2 + progress, ty, m_w - 4 - progress, 16, TftColor::BLACK);
  m_tft.fillRect(m_x + 2, m_y + 2, progress, m_h - 4, m_color_bar2);

  m_tft.drawText(tx, ty, text, m_color_frac);
  tx += TftCalc::t_width(strlen(text) - 6, 2); // 6 spaces
  m_tft.drawText(tx, ty, STRFMT_NOBUF("(%03d%%)", uint8_t(fraction * 100.0)), m_color_perc);
}

void Gui::ProgressIndicator::next() {
  ++m_cur_val;
}
