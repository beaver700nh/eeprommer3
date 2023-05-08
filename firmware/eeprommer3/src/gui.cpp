#include <Arduino.h>
#include "constants.hpp"

#include "ad_array.hpp"
#include "dialog.hpp"
#include "strfmt.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "gui.hpp"

extern TftCtrl tft;
extern TouchCtrl tch;

namespace Gui {

Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty, const char *text, uint16_t fg, uint16_t bg) :
  m_x(x), m_y(y), m_w(w), m_h(h), m_tx(tx), m_ty(ty), m_fg(fg), m_bg(bg), m_text(text) {
}

Btn::Btn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text, uint16_t fg, uint16_t bg) :
  Btn(x, y, w, h, 0, 0, text, fg, bg) {
  flags.auto_center = true;

  // Pass `tx/ty` as (0, 0) temporarily in init list
  // Overwrite dummy (0, 0) with real center here
  do_auto_center();
}

void Btn::draw() {
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

void Btn::erase() {
  if (!appearance.was_visible) return;  // If it was invisible, there is nothing to erase.

  if (appearance.was_highlighted) {
    tft.fillRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, TftColor::BLACK);
  }
  else {
    tft.fillRect(m_x, m_y, m_w, m_h, TftColor::BLACK);
  }
}

void Btn::draw_highlight() {
  uint16_t color = (
    !appearance.is_highlighted     ? TftColor::BLACK :
    !flags.operational             ? TftColor::LGRAY :
    ((m_bg >> 11) > (m_bg & 0x1F)) ? TftColor::CYAN :
                                     TftColor::YELLOW
  );

  tft.drawThickRect(m_x - 3, m_y - 3, m_w + 6, m_h + 6, color, 3);
}

void Btn::wait_for_press() {
  while (!is_pressed()) {
    /* wait for press */;
  }
}

bool Btn::is_pressed() {
  if (!flags.operational) return false;  // Ignore presses if non-operational.

  TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft.width(), tft.height());

  return (
    tch.is_valid_pressure(p.z) &&
    IN_RANGE(p.x, (int32_t) m_x, (int32_t) (m_x + m_w)) &&
    IN_RANGE(p.y, (int32_t) m_y, (int32_t) (m_y + m_h))
  );
}

Menu::~Menu() {
  purge_btns();
  SER_LOG_PRINT("[Menu destructor called.]\n");
}

Btn *Menu::add_btn(Btn *btn) {
  auto new_arr = (Btn **) malloc((m_num_btns + 1) * sizeof(Btn *));

  if (new_arr == nullptr) return nullptr;

  memcpy(new_arr, m_btns, m_num_btns * sizeof(Btn *));
  new_arr[m_num_btns] = btn;

  free(m_btns);

  m_btns = new_arr;
  ++m_num_btns;

  return btn;
}

bool Menu::rm_btn(uint8_t btn_idx) {
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

bool Menu::set_btn(uint8_t btn_idx, Btn *btn) {
  if (btn_idx >= m_num_btns) return false;

  m_btns[btn_idx] = btn;
  return true;
}

Btn *Menu::get_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return nullptr;

  return m_btns[btn_idx];
}

bool Menu::purge_btn(uint8_t btn_idx) {
  if (btn_idx >= m_num_btns) return false;

  Btn *to_del = m_btns[btn_idx];

  if (!rm_btn(btn_idx)) return false;
  delete to_del;

  return true;
}

void Menu::purge_btns() {
  while (m_num_btns > 0) {
    purge_btn(m_num_btns - 1);  // Purge last button in array
  }

  m_num_btns = 0;
}

uint8_t Menu::get_num_btns() {
  return m_num_btns;
}

void Menu::draw() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->draw();
  }
}

void Menu::erase() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->erase();
  }
}

uint8_t Menu::wait_for_press() {
  int16_t btn = 0;

  do {
    btn = get_pressed();
  }
  while (btn < 0);

  return btn;
}

int16_t Menu::get_pressed() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    if (m_btns[i]->is_pressed()) {
      return i;
    }
  }

  return -1;
}

void Menu::deselect_all() {
  for (uint8_t i = 0; i < m_num_btns; ++i) {
    m_btns[i]->highlight(false);
  }
}

const uint8_t *KeyboardLayout::get_layout() {
  return m_layout;
}

uint8_t KeyboardLayout::get_width() {
  return m_width;
}

uint8_t KeyboardLayout::get_height() {
  return ceil((float) m_length / (float) m_width);
}

const char *KeyboardLayout::get_ptr_char(uint8_t x, uint8_t y) {
  return (char *) m_layout + ptrdiff_t(2 * (y * m_width + x));
}

char KeyboardLayout::get_char(uint8_t x, uint8_t y) {
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

KeyboardLayout glob_kbd_hex_layout((const uint8_t *) glob_kbd_hex_layout_data, 16, 8);

static const uint16_t glob_kbd_str_layout_data[] PROGMEM {
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x11,
  0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7e,
  0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x5f, 0x2d,
  0x7f, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0xb0, 0x2e, 0x2f,
};

KeyboardLayout glob_kbd_str_layout((const uint8_t *) glob_kbd_str_layout_data, 44, 11);

MenuKeyboard::MenuKeyboard(
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

MenuKeyboard::~MenuKeyboard() {
  free(m_val);
};

void MenuKeyboard::update_val(char c) {
  uint8_t len = strlen(m_val);

  if (len >= BUF_LEN()) return;

  m_val[len]     = c;
  m_val[len + 1] = '\0';
}

void MenuKeyboard::show_val(uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg) {
  tft.fillRect(x, y, tft.width() - x, 8 * size, bg);

  char fmt_str[16];
  sprintf_P(fmt_str, PSTR("[%%-%d.%ds]"), len, len);
  tft.drawText(x, y, STRFMT_NOBUF(fmt_str, m_val), fg, size);
}

void MenuKeyboard::get_val(char *buf, uint8_t len) {
  strncpy(buf, m_val, len);
}

char *MenuKeyboard::get_ptr_val() {
  return m_val;
}

void MenuKeyboard::set_val(const char *buf, uint8_t len) {
  strncpy(m_val, buf, len);
}

bool MenuKeyboard::handle_key(uint8_t key) {
  UNUSED_VAR(key);

  unsigned long t_since_last_press = millis() - m_t_last_press;

  if (t_since_last_press > m_t_debounce) {
    m_t_last_press = millis();
    return true;
  }

  return false;
}

KeyboardLayout &MenuKeyboard::get_layout() {
  return m_layout;
}

MenuStrInput::MenuStrInput(
  uint16_t debounce, uint16_t pad_v, uint16_t pad_h,
  uint16_t marg_v, uint16_t marg_h, uint8_t buf_len
) :
  MenuKeyboard(debounce, pad_v, pad_h, marg_v, marg_h, glob_kbd_str_layout), m_buf_len(buf_len) {
  m_val    = (char *) malloc((BUF_LEN() + 1) * sizeof(char));
  m_val[0] = '\0';
}

void MenuStrInput::update_val(char c) {
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

void MenuStrInput::show_val(uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
  // Show caps indicator
  tft.drawTextBg_P(TftCalc::right(tft, 10, 10), 10, (m_capitalize ? Strings::L_INDIC_MAJ : Strings::L_INDIC_MIN));

  uint8_t char_width = 6 * (size == 0 ? 1 : size);  // Prevent divide-by-zero
  uint8_t maxfit_len = (tft.width() - size - 2 * m_marg_h) / char_width - 2;

  uint8_t working_text_len = MIN(maxfit_len, m_buf_len);

  MenuKeyboard::show_val(x, y, working_text_len, size, fg, bg);
}

bool MenuStrInput::handle_key(uint8_t key) {
  if (!MenuKeyboard::handle_key(key)) return false;

  auto w  = get_layout().get_width();
  char ch = get_layout().get_char(key % w, key / w);

  update_val(ch);
  return true;
}

char MenuStrInput::capitalize(char c) {
  return (m_capitalize ? toupper : tolower)(c);
}

void MenuChoice::set_callback(Callback callback) {
  m_callback = callback;
}

MenuChoice::Callback MenuChoice::get_callback() {
  return m_callback;
}

Btn *MenuChoice::add_btn(Btn *btn) {
  Btn *added = Menu::add_btn(btn);

  if (added == nullptr) return nullptr;

  if (m_num_btns - 1 == m_cur_choice) {
    added->highlight(true);
  }

  return added;
}

Btn *MenuChoice::add_btn_calc(const char *text, uint16_t fg, uint16_t bg) {
  uint16_t col = m_num_btns % m_num_cols;
  uint16_t row = m_num_btns / m_num_cols;

  uint16_t w = TftCalc::fraction(tft.width() + 2 * (m_pad_h - m_marg_h), m_pad_h, m_num_cols);
  uint16_t h = (m_btn_height_px ? (uint16_t) m_btn_height : (float) w * m_btn_height);

  uint16_t x = m_marg_h + col * (w + m_pad_h);
  uint16_t y = m_marg_v + row * (h + m_pad_v);

  return add_btn(new Btn(x, y, w, h, text, fg, bg));
}

Btn *MenuChoice::add_btn_confirm(bool force_bottom, uint16_t fg, uint16_t bg) {
  set_confirm_btn(m_num_btns);

  uint16_t y = TftCalc::bottom(tft, 24, (force_bottom ? 10 : m_marg_v));
  uint16_t w = TftCalc::fraction_x(tft, m_marg_h, 1);

  return add_btn(new Btn(m_marg_h, y, w, 24, Strings::L_CONFIRM, fg, bg));
}

void MenuChoice::set_confirm_btn(uint8_t btn_id) {
  m_confirm_btn = btn_id;
  select(m_cur_choice);
}

uint8_t MenuChoice::wait_for_value() {
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

void MenuChoice::set_choice(uint8_t btn) {
  m_old_choice = m_cur_choice;
  m_cur_choice = btn;
}

uint8_t MenuChoice::get_choice() {
  return m_cur_choice;
}

void MenuChoice::select(uint8_t choice) {
  set_choice(choice);
  get_btn(m_old_choice)->highlight(false);
  get_btn(choice)->highlight(true);
}

void MenuChoice::update() {
  get_btn(m_old_choice)->draw_highlight();
  get_btn(m_cur_choice)->draw_highlight();
}

MenuYesNo::MenuYesNo(
  uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, bool force_bottom, uint8_t initial_choice
) :
  MenuChoice(pad_v, pad_h, marg_v, marg_h, 2, 0.7, false, initial_choice) {
  add_btn_calc(Strings::L_YES, TftColor::BLACK, TftColor::GREEN);
  add_btn_calc(Strings::L_NO, TftColor::WHITE, TftColor::RED);
  add_btn_confirm(force_bottom);
}

MenuPairs::MenuPairs(uint8_t marg_u, uint8_t marg_d, uint8_t marg_s, uint8_t pair_height, uint8_t pair_pad, uint8_t num_pairs, AddrDataArray *buf)
  : m_marg_u(marg_u), m_marg_d(marg_d), m_marg_s(marg_s), m_pair_height(pair_height), m_pair_pad(pair_pad), m_num_pairs(num_pairs), m_buf(buf) {
  const uint16_t w1 = TftCalc::fraction_x(tft, marg_s, 1);
  const uint16_t w2 = TftCalc::fraction_x(tft, marg_s, 2);
  const uint16_t _h = tft.height() - marg_u - marg_d - 2 * (24 + 10);
  const uint16_t x1 = marg_s;
  const uint16_t x2 = 2 * marg_s + w2;
  const uint16_t x3 = TftCalc::right(tft, 24, marg_s);
  const uint16_t y1 = marg_u;
  const uint16_t y2 = y1 + 24 + 10;
  const uint16_t y3 = TftCalc::bottom(tft, 24, marg_d);

  add_btn(new Btn(x1, y2, 24, _h, Strings::L_ARROW_U,  TftColor::WHITE, TftColor::DGRAY ));
  add_btn(new Btn(x3, y2, 24, _h, Strings::L_ARROW_D,  TftColor::WHITE, TftColor::DGRAY ));
  add_btn(new Btn(x1, y1, w1, 24, Strings::L_ADD_PAIR, TftColor::WHITE, TftColor::PURPLE));
  add_btn(new Btn(x1, y3, w2, 24, Strings::L_CONFIRM,  TftColor::PINKK, TftColor::RED   ));
  add_btn(new Btn(x2, y3, w2, 24, Strings::L_CANCEL,   TftColor::CYAN,  TftColor::BLUE  ));

  for (uint8_t i = 0; i < num_pairs; ++i) {
    const uint16_t right_space = marg_s + 24 + 10;
    const uint16_t x = TftCalc::right(tft, 18, right_space);
    const uint16_t y = y2 + 2 + i * (pair_height + pair_pad);
    const uint16_t w = pair_height - 4;
    m_deleters.add_btn(new Btn(x, y, w, w, Strings::L_X_CLOSE, TftColor::YELLOW, TftColor::RED));
  }
}

void MenuPairs::draw() {
  Menu::draw();
  draw_pairs();
  m_deleters.draw();
}

MenuPairs::Status MenuPairs::poll() {
  uint16_t max_scroll = (m_num_pairs > m_buf->get_len()) ? 0 : (m_buf->get_len() - m_num_pairs);
  int16_t pressed, deleted;

  while (true) {
    deleted = m_deleters.get_pressed();

    if (deleted >= 0) {
      m_buf->remove(deleted + m_scroll);
      break;
    }

    pressed = get_pressed();

    if (pressed >= 0) {
      switch (pressed) {
      case 0:  if (m_scroll > 0)          --m_scroll; break;
      case 1:  if (m_scroll < max_scroll) ++m_scroll; break;
      case 2:  add_pair_from_user();                  break;
      case 3:  return Status::DONE;
      default: return Status::CANCELED;
      }

      break;
    }
  }

  return Status::RUNNING;
}

void MenuPairs::draw_pairs() {
  // Clear the pairs area
  const uint16_t x = m_marg_s + 24 + 10;
  const uint16_t y = m_marg_u + 24 + 10;
  const uint16_t w = TftCalc::fraction(tft.width(), x, 1);
  const uint16_t h = tft.height() - m_marg_u - m_marg_d - 2 * (24 + 10);
  tft.fillRect(x, y, w, h, TftColor::BLACK);

  for (uint8_t i = 0; i < m_num_pairs; ++i) {
    // Hide all the delete buttons here, later show the ones that are needed
    m_deleters.get_btn(i)->visibility(false)->operation(false);
  }

  if (m_buf->get_len() == 0) {
    tft.drawText_P(x, y +  0, Strings::L_NO_PAIRS1, TftColor::LGRAY);
    tft.drawText_P(x, y + 30, Strings::L_NO_PAIRS2, TftColor::LGRAY);
    return;
  }

  uint16_t this_pair = m_scroll;
  uint16_t last_pair = MIN(m_num_pairs + m_scroll, m_buf->get_len()) - 1U;

  do {
    const uint16_t _y = y + (this_pair - m_scroll) * (m_pair_height + m_pair_pad);
    const uint16_t _h = m_pair_height;
    const uint16_t tx = x + 3;
    const uint16_t ty = _y + TftCalc::t_center_y(_h, 2);

    AddrDataArrayPair pair {0, 0};
    m_buf->get_pair(this_pair, &pair);

    tft.fillRect(x, _y, w, _h, TftColor::ORANGE);
    tft.drawText(tx, ty, STRFMT_P_NOBUF(Strings::L_FMT_PAIR, this_pair, pair.addr, pair.data), TftColor::BLACK, 2);

    // Show all buttons that have pairs
    m_deleters.get_btn(this_pair - m_scroll)->visibility(true)->operation(true);
  }
  while (this_pair++ != last_pair);
}

void MenuPairs::add_pair_from_user() {
  tft.fillScreen(TftColor::BLACK);
  auto addr = Dialog::ask_addr(Strings::P_ADDR_GEN);
  tft.fillScreen(TftColor::BLACK);
  auto data = Dialog::ask_int<uint8_t>(Strings::P_DATA_GEN);
  tft.fillScreen(TftColor::BLACK);

  m_buf->append((AddrDataArrayPair) {addr, data});
}

ProgressIndicator::ProgressIndicator(uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
  : m_max_val(max_val ? max_val : 1), m_x(x), m_y(y), m_w(w), m_h(h) {
  tft.drawThickRect(x, y, w, h, TftColor::DGRAY, 2);
}

void ProgressIndicator::show() {
  const double fraction = (double) m_cur_val / m_max_val;

  if (fraction > 1.0) return;

  if (m_cur_val > 0) {
    tft.drawText(m_tx + 1, m_ty + 1, m_buffer, TftColor::WHITE);
    tft.drawText(m_tx, m_ty, m_buffer, TftColor::WHITE);
  }

  const uint16_t old_progress = m_progress;
  m_progress = (m_w - 4) * fraction;

  const uint8_t length = snprintf_P_sz(m_buffer, PSTR("%d/%d (%03d%%)"), m_cur_val, m_max_val, (uint8_t) (fraction * 100.0));

  m_tx = m_x + TftCalc::t_center_x(m_w, length, 2);
  m_ty = m_y + TftCalc::t_center_y(m_h, 2);

  tft.fillRect(m_x + 2 + old_progress, m_y + 2, m_progress - old_progress, m_h - 4, TftColor::WHITE);

  const uint16_t text_end = m_tx + TftCalc::t_width(length, 2);

  if (m_progress <= text_end) {
    tft.fillRect(m_x + 2 + m_progress, m_y + 2, text_end - m_progress, m_h - 4, TftColor::BLACK);
  }

  tft.drawText(m_tx + 1, m_ty + 1, m_buffer, TftColor::LGRAY);
  tft.drawText(m_tx, m_ty, m_buffer, TftColor::BLACK);
}

void ProgressIndicator::next() {
  ++m_cur_val;
}

PageDisplay::PageDisplay(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr)
  : m_data(data), m_addr1(addr1), m_addr2(addr2), m_repr(repr) {
  // Nothing
}

inline PageDisplay::ByteRepr PageDisplay::repr_hex(uint8_t input_byte) {
  ByteRepr repr;

  repr.offset = 0;
  repr.color  = TftColor::WHITE;
  sprintf_P(repr.text, PSTR("%02X"), input_byte);

  return repr;
}

inline PageDisplay::ByteRepr PageDisplay::repr_chars(uint8_t input_byte) {
  const bool printable = isprint(input_byte);
  ByteRepr repr;

  repr.offset = 3;
  repr.color  = (printable ? TftColor::WHITE : TftColor::GRAY);
  sprintf_P(repr.text, PSTR("%c"), (printable ? input_byte : '?'));

  return repr;
}

void PageDisplay::show_range() {
  // Draw frame for the data
  tft.drawText(10, 10, STRFMT_P_NOBUF(Strings::T_N_BYTES, m_addr2 - m_addr1 + 1), TftColor::CYAN, 3);
  tft.drawThickRect(tft.width() / 2 - 147, 55, 295, 166, TftColor::WHITE, 2);
  tft.drawFastVLine(tft.width() / 2, 57, 162, TftColor::GRAY);

  draw_page_axis_labels();

  const uint16_t x2 = tft.width() - 55;

  Gui::Menu menu;
  menu.add_btn(new Gui::Btn(15, 60, 40, 150, 15, 68, Strings::L_ARROW_L));
  menu.add_btn(new Gui::Btn(x2, 60, 40, 150, 15, 68, Strings::L_ARROW_R));
  menu.add_btn(new Gui::Btn(BOTTOM_BTN(Strings::L_CLOSE)));
  menu.draw();

  const uint8_t max_page = (addr2 >> 8) - (addr1 >> 8);
  uint8_t cur_page = 0;

  while (true) {
    show_page(cur_page, max_page);

    switch (menu.wait_for_press()) {
    case 0: cur_page = (cur_page == 0 ? max_page : cur_page - 1); break;  // Left
    case 1: cur_page = (cur_page == max_page ? 0 : cur_page + 1); break;  // Right
    case 2: return;                                                       // Close
    }
  }
}

void PageDisplay::show_page(uint8_t cur_page, uint8_t max_page) {
  tft.fillRect(tft.width() / 2 - 145, 57, 145, 162, TftColor::DGRAY);
  tft.fillRect(tft.width() / 2 +   1, 57, 145, 162, TftColor::DGRAY);

  tft.drawTextBg(
    10, 256, STRFMT_P_NOBUF(Strings::L_PAGE_N_N, cur_page, max_page),
    TftColor::PURPLE, TftColor::BLACK
  );

  uint16_t glob_range_start = m_addr1 >> 8;
  uint16_t glob_page_start  = MAX(((cur_page + glob_range_start + 0) << 8) + 0, m_addr1);
  uint16_t glob_page_end    = MIN(((cur_page + glob_range_start + 1) << 8) - 1, m_addr2);

  tft.drawTextBg(
    TftCalc::right(tft, 130, 12), 12, STRFMT_P_NOBUF(PSTR("%04X - %04X"), glob_page_start, glob_page_end),
    TftColor::ORANGE, TftColor::BLACK
  );

  for (uint16_t i = glob_page_start; i <= glob_page_end; ++i) {
    uint8_t tft_byte_col = (i & 0x0F);
    uint8_t tft_byte_row = (i & 0xFF) >> 4;

    ByteRepr br          = (*m_repr)(m_data[i - m_addr1]);

    uint8_t split_offset = (tft_byte_col < 8 ? 0 : 3);
    uint16_t tft_byte_x  = tft.width() / 2 - 141 + 18 * tft_byte_col + br.offset + split_offset;
    uint16_t tft_byte_y  = tft.height() / 2 - 100 + 10 * tft_byte_row;

    tft.drawText(tft_byte_x, tft_byte_y, br.text, br.color, 1);
  }
}

void PageDisplay::draw_page_axis_labels() {
  for (uint8_t row = 0x00; row < 0x10; ++row) {
    const uint16_t x1 = tft.width() / 2 - 161;
    const uint16_t x2 = tft.width() / 2 + 151;

    const uint16_t y = tft.height() / 2 - 100 + 10 * row;

    char label[3];
    STRFMT_P_sz(label, PSTR("%1Xx"), row);

    tft.drawText(x1, y, label, TftColor::DCYAN, 1);
    tft.drawText(x2, y, label, TftColor::DCYAN, 1);
  }

  for (uint8_t col = 0x00; col < 0x10; ++col) {
    const uint8_t split_offset = (col < 8 ? 0 : 3);
    const uint16_t x = tft.width() / 2 - 141 + 18 * col + split_offset;

    tft.drawText(x, tft.height() / 2 + 64, STRFMT_P_NOBUF(PSTR("x%1X"), col), TftColor::DCYAN, 1);
  }
}

};
