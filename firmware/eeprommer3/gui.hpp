#ifndef GUI_HPP
#define GUI_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "touch.hpp"
#include "util.hpp"

/*
 * This file contains classes for GUI elements such as buttons and menus.
 */

namespace Gui {

/*
 * Button class that stores some metadata, can detect if it is pressed, and can draw itself to the TFT screen.
 */
class Btn {
public:
  // Default constructor; does nothing.
  Btn() {};

  // Full constructor.
  Btn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty,
    const char *text, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  // Partial constructor; enables auto centering, eliminating need for `tx` and `ty`.
  Btn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text,
    uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  inline void init_bit_fields() {
    flags.auto_center = 0;
    flags.operational = 1;
    flags.ram_label   = 0;

    appearance.is_highlighted  = 0;
    appearance.was_highlighted = 0;
    appearance.is_visible      = 1;
    appearance.was_visible     = 1;
  }

  void draw();
  void erase();
  void draw_highlight();

  inline uint8_t calc_center_y() { return TftCalc::t_center_y(m_h, m_font_size); }
  inline uint8_t calc_center_x() {
    auto len_fn = (flags.ram_label ? strlen : strlen_P);
    return TftCalc::t_center_x_l(m_w, len_fn(m_text), m_font_size);
  }

  inline uint16_t get_x() { return m_x; }
  inline Btn *set_x(uint16_t val) {
    m_x = val;
    return this;
  }

  inline uint16_t get_y() { return m_y; }
  inline Btn *set_y(uint16_t val) {
    m_y = val;
    return this;
  }

  inline uint16_t get_w() { return m_w; }
  inline Btn *set_w(uint16_t val) {
    m_w = val;
    return this;
  }

  inline uint16_t get_h() { return m_h; }
  inline Btn *set_h(uint16_t val) {
    m_h = val;
    return this;
  }

  inline uint16_t get_tx() { return m_tx; }
  inline Btn *set_tx(uint16_t val) {
    m_tx              = val;
    flags.auto_center = false;
    return this;
  }

  inline uint16_t get_ty() { return m_ty; }
  inline Btn *set_ty(uint16_t val) {
    m_ty              = val;
    flags.auto_center = false;
    return this;
  }

  inline uint16_t get_fg() { return m_fg; }
  inline Btn *set_fg(uint16_t val) {
    m_fg = val;
    return this;
  }

  inline uint16_t get_bg() { return m_bg; }
  inline Btn *set_bg(uint16_t val) {
    m_bg = val;
    return this;
  }

  inline uint8_t get_font_size() { return m_font_size; }
  inline Btn *set_font_size(uint8_t val) {
    m_font_size = val;
    do_auto_center();
    return this;
  }

  inline const char *get_text() { return m_text; }
  inline Btn *set_text(const char *val) {
    m_text = val;
    do_auto_center();
    return this;
  }

  // Controls whether an outline appears around the button. Default is false/unhighlighted.
  inline Btn *highlight(bool val) {
    appearance.is_highlighted = val;
    return this;
  }

  // Controls whether the button is visible. Default is true/visible.
  inline Btn *visibility(bool val) {
    appearance.is_visible = val;
    return this;
  }

  // Controls whether the button responds to presses. Default is true/operational.
  inline Btn *operation(bool val) {
    flags.operational = val;
    return this;
  }

  // Controls whether the button treats its label text as stored in RAM (true) or PROGMEM (false). Default is false/PROGMEM.
  inline Btn *ram_label(bool val) {
    flags.ram_label = val;
    do_auto_center();
    return this;
  }

  // Controls whether the button automatically centers text. Only call this to override auto detection.
  // Default is false / manual centering; if constructed without `tx/ty` parameters, default is true / auto centering.
  inline Btn *auto_center(bool val) {
    flags.auto_center = val;
    return this;
  }

  // Sets `tx` and `ty` to position text at center of button, but only if in `auto_center` mode.
  inline Btn *do_auto_center() {
    if (flags.auto_center) {
      m_tx = calc_center_x();
      m_ty = calc_center_y();
    }

    return this;
  }

  // Test if button is being pressed (There is a press on `tch` at x to x+w, y to y+h).
  bool is_pressed();
  // Wait for a press on the button, blocks until `is_pressed()` returns true.
  void wait_for_press();

  struct {
    uint8_t auto_center : 1;
    uint8_t operational : 1;
    uint8_t ram_label   : 1;
  } flags;

  struct {
    uint8_t is_highlighted  : 1;
    uint8_t was_highlighted : 1;
    uint8_t is_visible      : 1;
    uint8_t was_visible     : 1;
  } appearance;

private:
  uint8_t m_font_size = 2;
  uint16_t m_x, m_y, m_w, m_h;
  uint16_t m_tx, m_ty;
  uint16_t m_fg, m_bg;

  const char *m_text;
} __attribute__((packed, aligned(1)));

/*
 * `Menu` makes creating menus easier; it's basically just a collection of `Btn`s with some helpful functions included.
 */
class Menu {
public:
  Menu() {};
  ~Menu();

  Btn *add_btn(Btn *btn);
  bool set_btn(uint8_t btn_idx, Btn *btn);
  Btn *get_btn(uint8_t btn_idx);
  bool purge_btn(uint8_t btn_idx);
  void purge_btns();

  uint8_t get_num_btns();

  void draw();
  void erase();

  uint8_t wait_for_press();
  int16_t get_pressed();

  void deselect_all();

protected:
  bool rm_btn(uint8_t btn_idx);  // Not exposed because unsafe: does not `delete` the button

  Btn **m_btns       = nullptr;
  uint8_t m_num_btns = 0;
};

/*
 * `KeyboardLayout` is a class for storing layouts of `MenuKeyboard`s.
 */
class KeyboardLayout {
public:
  KeyboardLayout(const uint8_t *layout, uint8_t length, uint8_t width) : m_layout(layout), m_length(length), m_width(width) {};

  const uint8_t *get_layout();

  uint8_t get_width();
  uint8_t get_height();

  const char *get_ptr_char(uint8_t x, uint8_t y);
  char get_char(uint8_t x, uint8_t y);

private:
  const uint8_t *m_layout;

  uint8_t m_length;
  uint8_t m_width;
};

// Pre-set layouts of the different `MenuKeyboard`s

extern KeyboardLayout glob_kbd_hex_layout;
extern KeyboardLayout glob_kbd_str_layout;

/*
 * `MenuKeyboard` is an ABC specialization of `Menu` for getting aggregate values such as
 * strings and numbers from the user via the use of a keyboard.
 * 
 * THIS IS A BASE CLASS AND CANNOT BE USED DIRECTLY -- USE A SUBCLASS INSTEAD!
 */
class MenuKeyboard : public Menu {
public:
  // IMPORTANT - Does not initialize internal value string, do not forget to do so
  // height of button = calculated width of button * param `btn_height`
  MenuKeyboard(
    uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, KeyboardLayout &layout, float btn_height = 1.2
  );

  ~MenuKeyboard();

  void update_val(char c);
  void show_val(uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg);

  void get_val(char *buf, uint8_t len);
  char *get_ptr_val();
  void set_val(const char *buf, uint8_t len);

  // Placeholder: only checks for debounce
  bool handle_key(uint8_t key);

  KeyboardLayout &get_layout();

  // ***** REMEMBER TO OVERRIDE BUFFER LENGTH IN CHILD CLASS! *****
  inline virtual uint8_t BUF_LEN() {
    return 0;
  }

protected:
  char *m_val;

  unsigned long m_t_last_press = 0;
  uint16_t m_t_debounce;

  KeyboardLayout &m_layout;

  uint16_t m_pad_v, m_pad_h, m_marg_v, m_marg_h;
};

/*
 * `MenuHexInput` is a `MenuKeyboard` but specialized for inputting numbers in hexadecimal.
 * Type `T` is an integer type and determines what numbers can be inputted.
 */
template<typename T>
class MenuHexInput : public MenuKeyboard {
public:
  // param `val_size`: 1 = 8 bits, 2 = 16 bits, etc
  MenuHexInput(uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h) :
    MenuKeyboard(t_debounce, pad_v, pad_h, marg_v, marg_h, glob_kbd_hex_layout, 1) {
    m_val = (char *) malloc(BUF_LEN() * sizeof(char));  // NOLINT(cppcoreguidelines-prefer-member-initializer): init list taken by delegated ctor

    for (uint8_t i = 0; i < BUF_LEN(); ++i) {
      m_val[i] = '0';
    }

    m_val[BUF_LEN()] = '\0';
  }

  ~MenuHexInput() {}  // Ensure that base destructor(s) are called.

  void update_val(char c) {
    uint8_t len = strlen(m_val);

    if (len >= BUF_LEN()) {
      // Scroll on reaching length limit, discard overflow
      for (uint8_t i = 1; i < BUF_LEN(); ++i) {
        m_val[i - 1] = m_val[i];
      }

      m_val[BUF_LEN() - 1] = c;

      return;
    }

    MenuKeyboard::update_val(c);
  }

  void show_val(uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
    MenuKeyboard::show_val(x, y, BUF_LEN(), size, fg, bg);
  }

  T get_int_val() {
    T result    = 0;
    uint8_t len = strlen(m_val);

    for (uint8_t i = len - BUF_LEN(); i < len; ++i) {
      result = (result << 4);

      if (m_val[i] > '9') {
        result += m_val[i] - 'A' + 0xA;
      }
      else {
        result += m_val[i] - '0';
      }
    }

    return result;
  }

  bool handle_key(uint8_t key) {
    if (!MenuKeyboard::handle_key(key)) return false;

    update_val(IN_RANGE(key, 0, 10) ? key + '0' : key + 'A' - 10);
    return true;
  }

  inline uint8_t BUF_LEN() override {
    return BIT_WIDTH(T) / 4;
  }
};

/*
 * `MenuStrInput` is a `MenuKeyboard` but specialized for inputting strings.
 */
class MenuStrInput : public MenuKeyboard {
public:
  MenuStrInput(uint16_t debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, uint8_t buf_len);
  ~MenuStrInput() {}  // Ensure that base destructor(s) are called.

  void update_val(char c);

  void show_val(uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg);

  bool handle_key(uint8_t key);

  char capitalize(char c);

  inline uint8_t BUF_LEN() override {
    return m_buf_len;
  }

private:
  uint8_t m_buf_len;

  bool m_capitalize = false;
};

/*
 * `MenuChoice` is a `Menu` specialized to choose one option from a selection of choices. It also supports
 * having button layouts be automatically calculated based on the parameters of padding and margin passed in the constructor.
 */
class MenuChoice : public Menu {
public:
  // `btn_height_px`: true = `btn_height` uses px, false = multiplied with btn width
  MenuChoice(
    uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t num_cols,
    float btn_height, bool btn_height_px, uint8_t initial_choice = 0
  ) :
    m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h), m_num_cols(num_cols),
    m_btn_height(btn_height), m_btn_height_px(btn_height_px) {
    select(initial_choice);
  }

  ~MenuChoice() {}  // Ensure that base destructor(s) are called.

  // Function that gets run when user presses a button
  typedef void (*Callback)(uint8_t btn_id, bool is_confirm);

  void set_callback(Callback callback);
  Callback get_callback();

  void set_choice(uint8_t choice);
  void select(uint8_t btn);

  void update();

  Btn *add_btn(Btn *btn);
  Btn *add_btn_calc(const char *text, uint16_t fg, uint16_t bg);
  Btn *add_btn_confirm(bool force_bottom, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE);

  void set_confirm_btn(uint8_t btn_id);

  uint8_t wait_for_value();

protected:
  uint8_t m_pad_v, m_pad_h, m_marg_v, m_marg_h;
  uint8_t m_num_cols;

  float m_btn_height;
  bool m_btn_height_px;

  uint8_t m_confirm_btn = 0;
  uint8_t m_cur_choice  = 0;
  uint8_t m_old_choice  = 0;

  Callback m_callback = [](uint8_t btn_id, bool is_confirm) -> void {
    UNUSED_VAR(btn_id);
    UNUSED_VAR(is_confirm);
  };
};

/*
 * `MenuYesNo` is a `MenuChoice` specialized to only have the options "Yes" and "No".
 * It also automatically adds the "Confirm" button upon construction.
 */
class MenuYesNo : public MenuChoice {
public:
  MenuYesNo(
    uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, bool force_bottom, uint8_t initial_choice = 0
  );

  ~MenuYesNo() {}  // Ensure that base destructor(s) are called.
};

// Should return true to request cancel of loop.
#define GUI_PROGRESS_INDICATOR_LAMBDA (uint8_t progress) -> bool

/*
 * `ProgressIndicator` is a class to show a progress amount on a `TftCtrl` as a fraction, a percentange, and a progress bar.
 * It will try to only take up as much space as it is allotted in the constructor.
 */
class ProgressIndicator {
public:
  ProgressIndicator(
    uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
    uint16_t color_frac = TftColor::DGREEN, uint16_t color_perc = TftColor::BLUE,
    uint16_t color_bar1 = TftColor::DRED, uint16_t color_bar2 = TftColor::WHITE
  );

  // Is a template func because lambdas without libstdc++ are annoying.
  // Returns true if loop completed succesfully, false if canceled.
  // `action` is a lambda like TFT_PROGRESS_INDICATOR_LAMBDA
  template<typename Func>
  bool for_each(Func action) {
    show();

    while (m_cur_val < m_max_val) {
      bool should_quit = action(m_cur_val);
      next();
      show();

      if (should_quit) return false;
    }

    return true;
  }

  void show();
  void next();

private:
  uint16_t m_max_val, m_cur_val = 0;
  uint16_t m_x, m_y, m_w, m_h;
  uint16_t m_color_frac, m_color_perc, m_color_bar1, m_color_bar2;
};

};

#endif
