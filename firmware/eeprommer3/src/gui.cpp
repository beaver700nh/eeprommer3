#include <Arduino.h>
#include "constants.hpp"

#include "strfmt.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "gui.hpp"

extern TftCtrl tft;
extern TouchCtrl tch;

Gui::Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg) :
  m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg), m_text(text) {
}

Gui::Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg) :
  Btn(x, y, w, h, 0, 0, text, fg, bg) {
  flags.auto_center = true;

  // Pass `tx/ty` as (0, 0) temporarily in init list
  // Overwrite dummy (0, 0) with real center here
  do_auto_center();
}

void Gui::Btn::draw() {
  appearance.was_highlighted = appearance.is_highlighted;
  appearance.was_visible     = appearance.is_visible;

  if (!appearance.is_visible) return;  // If it is invisible, there is nothing to draw.

  uint16_t fg = (flags.operational ? m_fg : TftColor::DGRAY);
  uint16_t bg = (flags.operational ? m_bg : TftColor::GRAY);

  tft.fillRect(m_x, m_y, m_w, m_h, bg);

  TftCtrl::drawText_t printer = (flags.ram_label ? &TftCtrl::drawText : &TftCtrl::drawText_P);

  (tft.*printer)(m_x + m_tx, m_y + m_ty, m_text, fg, m_font_size);

  draw_highlight();
}

void Gui::Btn::erase() {
  if (!appearance.was_visible) return;  // If it was invisible, there is nothing to erase.

  if (appearance.was_highlighted) {
    tft.fillRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::BLACK);
  }
  else {
    tft.fillRect(m_x, m_y, m_w, m_h, TftColor::BLACK);
  }
}

void Gui::Btn::draw_highlight() {
  auto color = ([=]() -> uint16_t {
    if (!appearance.is_highlighted) {
      return TftColor::BLACK;
    }

    if (!flags.operational) {
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

void Gui::Btn::wait_for_press() {
  while (!is_pressed()) {
    /* wait for press */;
  }
}

bool Gui::Btn::is_pressed() {
  if (!flags.operational) return false;  // Ignore presses if non-operational.

  TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft.width(), tft.height());

  return (
    tch.is_valid_pressure(p.z) &&
    IN_RANGE(p.x, (int32_t) m_x, (int32_t) (m_x + m_w)) &&
    IN_RANGE(p.y, (int32_t) m_y, (int32_t) (m_y + m_h))
  );
}

Gui::Menu::~Menu() {
  purge_btns();
  SER_LOG_PRINT("[Menu destructor called.]\n");
}

Gui::Btn *Gui::Menu::add_btn(Btn *btn) {
  auto new_arr = (Btn **) malloc((m_num_btns + 1) * sizeof(Btn *));

  if (new_arr == nullptr) return nullptr;

  memcpy(new_arr, m_btns, m_num_btns * sizeof(Btn *));
  new_arr[m_num_btns] = btn;

  free(m_btns);

  m_btns = new_arr;
  ++m_num_btns;

  return btn;
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
    purge_btn(m_num_btns - 1);  // Purge last button in array
  }

  m_num_btns = 0;
}

uint8_t Gui::Menu::get_num_btns() {
  return m_num_btns;
}

void Gui::Menu::draw() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw();
  }
}

void Gui::Menu::erase() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->erase();
  }
}

uint8_t Gui::Menu::wait_for_press() {
  int16_t btn = 0;

  do {
    btn = get_pressed();
  }
  while (btn < 0);

  return btn;
}

int16_t Gui::Menu::get_pressed() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    if (m_btns[i]->is_pressed()) {
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

const char *Gui::KeyboardLayout::get_ptr_char(uint8_t x, uint8_t y) {
  return (char *) m_layout + ptrdiff_t(2 * (y * m_width + x));
}

char Gui::KeyboardLayout::get_char(uint8_t x, uint8_t y) {
  return pgm_read_byte_near(get_ptr_char(x, y));
}

/*
 * Layout data is uint16_t[] to insert NULs between the elements when casting back to uint8_t[]
 * Makes for simpler code but it's an ugly hack relying on the system being little-endian
 */

static const uint16_t glob_kbd_hex_layout_data[] PROGMEM {
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
};

Gui::KeyboardLayout Gui::glob_kbd_hex_layout((const uint8_t *) glob_kbd_hex_layout_data, 16, 8);

static const uint16_t glob_kbd_str_layout_data[] PROGMEM {
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x11,
  0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7e,
  0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x5f, 0x2d,
  0x7f, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0xb0, 0x2e, 0x2f,
};

Gui::KeyboardLayout Gui::glob_kbd_str_layout((const uint8_t *) glob_kbd_str_layout_data, 44, 11);

Gui::MenuKeyboard::MenuKeyboard(
  uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, KeyboardLayout &layout, float btn_height
) :
  m_t_debounce(t_debounce), m_layout(layout),
  m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h) {
  const uint16_t cell_width  = TftCalc::fraction(tft.width() - 2 * marg_h + 2 * pad_h, pad_h, layout.get_width());
  const uint16_t cell_height = (float) cell_width * btn_height;

  for (uint8_t row = 0; row < layout.get_height(); ++row) {
    for (uint8_t col = 0; col < layout.get_width(); ++col) {
      const uint16_t x = marg_h + col * (cell_width + pad_h);
      const uint16_t y = marg_v + row * (cell_height + pad_v);

      add_btn(new Btn(x, y, cell_width, cell_height, layout.get_ptr_char(col, row), TftColor::WHITE, TftColor::BLUE));
    }
  }

  add_btn(new Btn(BOTTOM_BTN(Strings::L_CONTINUE)));
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

void Gui::MenuKeyboard::show_val(uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg) {
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
  uint16_t debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, uint8_t buf_len
) :
  MenuKeyboard(debounce, pad_v, pad_h, marg_v, marg_h, glob_kbd_str_layout), m_buf_len(buf_len) {
  m_val    = (char *) malloc((BUF_LEN() + 1) * sizeof(char));
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

void Gui::MenuStrInput::show_val(uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
  // Show caps indicator
  tft.drawTextBg(TftCalc::right(tft, 10, 10), 10, (m_capitalize ? Strings::L_INDIC_MAJ : Strings::L_INDIC_MIN));

  uint8_t char_width = 6 * (size == 0 ? 1 : size);  // Prevent divide-by-zero
  uint8_t maxfit_len = (tft.width() - size - 2 * m_marg_h) / char_width - 2;

  uint8_t working_text_len = MIN(maxfit_len, m_buf_len);

  MenuKeyboard::show_val(x, y, working_text_len, size, fg, bg);
}

bool Gui::MenuStrInput::handle_key(uint8_t key) {
  if (!MenuKeyboard::handle_key(key)) return false;

  auto w  = get_layout().get_width();
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

Gui::Btn *Gui::MenuChoice::add_btn(Btn *btn) {
  Btn *added = Menu::add_btn(btn);

  if (added == nullptr) return nullptr;

  if (m_num_btns - 1 == m_cur_choice) {
    added->highlight(true);
  }

  return added;
}

Gui::Btn *Gui::MenuChoice::add_btn_calc(const char *text, uint16_t fg, uint16_t bg) {
  uint16_t col = m_num_btns % m_num_cols;
  uint16_t row = m_num_btns / m_num_cols;

  uint16_t w = TftCalc::fraction((uint16_t) tft.width() - 2 * (uint16_t)m_marg_h + 2 * (uint16_t)m_pad_h, (uint16_t)m_pad_h, (uint8_t)m_num_cols);
  uint16_t h = (m_btn_height_px ? (uint16_t) m_btn_height : (float) w * m_btn_height);

  uint16_t x = m_marg_h + col * (w + m_pad_h);
  uint16_t y = m_marg_v + row * (h + m_pad_v);

  return add_btn(new Btn(x, y, w, h, text, fg, bg));
}

Gui::Btn *Gui::MenuChoice::add_btn_confirm(bool force_bottom, uint16_t fg, uint16_t bg) {
  set_confirm_btn(m_num_btns);

  uint16_t y = TftCalc::bottom(tft, 24, (force_bottom ? 10 : m_marg_v));
  uint16_t w = TftCalc::fraction_x(tft, m_marg_h, 1);

  return add_btn(new Btn(m_marg_h, y, w, 24, Strings::L_CONFIRM, fg, bg));
}

void Gui::MenuChoice::set_confirm_btn(uint8_t btn_id) {
  m_confirm_btn = btn_id;
}

uint8_t Gui::MenuChoice::wait_for_value() {
  draw();

  while (true) {
    uint8_t btn_pressed = wait_for_press();
    (*m_callback)(btn_pressed, btn_pressed == m_confirm_btn);

    if (btn_pressed == m_confirm_btn) {
      if (get_btn(m_cur_choice)->flags.operational) {
        break;
      }
    }
    else {
      select(btn_pressed);
      update();
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

void Gui::MenuChoice::update() {
  get_btn(m_old_choice)->draw_highlight();
  get_btn(m_cur_choice)->draw_highlight();
}

Gui::MenuYesNo::MenuYesNo(
  uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h,
  bool force_bottom, uint8_t initial_choice
) :
  Gui::MenuChoice(pad_v, pad_h, marg_v, marg_h, 2, 0.7, false, initial_choice) {
  add_btn_calc(Strings::L_YES, TftColor::BLACK, TftColor::GREEN);
  add_btn_calc(Strings::L_NO, TftColor::WHITE, TftColor::RED);
  add_btn_confirm(force_bottom);
}

Gui::ProgressIndicator::ProgressIndicator(
  uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color_frac, uint16_t color_perc, uint16_t color_bar1, uint16_t color_bar2
) :
  m_max_val(max_val), m_x(x), m_y(y), m_w(w), m_h(h),
  m_color_frac(color_frac), m_color_perc(color_perc), m_color_bar1(color_bar1), m_color_bar2(color_bar2) {
  tft.drawRect(x, y, w, h, m_color_bar1);
  tft.drawRect(x + 1, y + 1, w - 2, h - 2, m_color_bar1);
}

void Gui::ProgressIndicator::show() {
  if (m_cur_val > m_max_val) return;

  double fraction   = (double) m_cur_val / (double) m_max_val;
  uint16_t progress = (m_w - 4) * fraction;

  const char *text = STRFMT_NOBUF("%d/%d       ", m_cur_val, m_max_val);

  uint16_t ty = m_y + TftCalc::t_center_y(m_h, 2);
  uint16_t tx = m_x + TftCalc::t_center_x_l(m_w, strlen(text), 2);

  tft.fillRect(m_x + 2 + progress, ty, m_w - 4 - progress, 16, TftColor::BLACK);
  tft.fillRect(m_x + 2, m_y + 2, progress, m_h - 4, m_color_bar2);

  tft.drawText(tx, ty, text, m_color_frac);
  tx += TftCalc::t_width(strlen(text) - 6, 2);  // 6 spaces
  tft.drawText(tx, ty, STRFMT_NOBUF("(%03d%%)", uint8_t(fraction * 100.0)), m_color_perc);
}

void Gui::ProgressIndicator::next() {
  ++m_cur_val;
}
