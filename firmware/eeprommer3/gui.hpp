#ifndef GUI_HPP
#define GUI_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft_calc.hpp"
#include "tft.hpp"
#include "touch.hpp"

/*
 * This file contains classes for GUI elements such as buttons and menus.
 */

namespace Gui {

/*
 * Class that stores some metadata, can detect if it is pressed, and can draw itself to the TFT screen.
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

  inline uint8_t calc_center_x() { return TftCalc::t_center_x(m_w, m_text, m_font_size); }
  inline uint8_t calc_center_y() { return TftCalc::t_center_y(m_h, m_font_size);         }

  inline uint16_t get_x()           { return m_x; }
  inline void     set_x(uint16_t x) { m_x = x;    }
  inline uint16_t get_y()           { return m_y; }
  inline void     set_y(uint16_t y) { m_y = y;    }

  inline uint16_t get_w()           { return m_w; }
  inline void     set_w(uint16_t w) { m_w = w;    }
  inline uint16_t get_h()           { return m_h; }
  inline void     set_h(uint16_t h) { m_h = h;    }

  inline uint16_t get_tx()            { return m_tx;                      }
  inline void     set_tx(uint16_t tx) { m_tx = tx; m_auto_center = false; }
  inline uint16_t get_ty()            { return m_ty;                      }
  inline void     set_ty(uint16_t ty) { m_ty = ty; m_auto_center = false; }

  inline uint16_t get_fg()            { return m_fg; }
  inline void     set_fg(uint16_t fg) { m_fg = fg;   }
  inline uint16_t get_bg()            { return m_bg; }
  inline void     set_bg(uint16_t bg) { m_bg = bg;   }

  inline uint8_t get_font_size()                  { return m_font_size;                     }
  inline void    set_font_size(uint8_t font_size) { m_font_size = font_size; auto_center(); }

  inline const char *get_text()                 { return m_text;                }
  inline void        set_text(const char *text) { m_text = text; auto_center(); }

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);
  void draw_highlight(TftCtrl &tft);

  // Controls whether a yellow outline appears around the button. Default is false/unhighlighted.
  inline void highlight(bool highlight) { m_is_highlighted = highlight; }
  inline bool is_highlighted() { return m_is_highlighted; }

  // Controls whether the button is visible. Default is true/visible.
  inline void visibility(bool visibility) { m_is_visible = visibility; }
  inline bool is_visible() { return m_is_visible; }

  // Controls whether the button responds to presses. Default is true/operational.
  inline void operation(bool operation) { m_is_operational = operation; }
  inline bool is_operational() { return m_is_operational; }

  // Controls whether the button automatically centers text. Only call this to override auto detection.
  // Default is false / manual centering; if constructed without `tx/ty` parameters, default is true / auto centering.
  inline void auto_centering(bool auto_center) { m_auto_center = auto_center; }
  inline bool is_auto_centering() { return m_auto_center; }

  // Sets `tx` and `ty` to position text at center of button, but only if in `auto_center` mode.
  inline void auto_center() {
    if (!m_auto_center) return;

    m_tx = calc_center_x();
    m_ty = calc_center_y();
  }

  // Wait for a press within the boundaries of the button (x to x+w, y to y+h).
  void wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  // Test if button is being pressed (x to x+w, y to y+h).
  bool is_pressed(TouchCtrl &tch, TftCtrl &tft);

private:
  uint8_t m_font_size = 2;
  uint16_t m_x, m_y, m_w, m_h;
  uint16_t m_tx, m_ty;
  uint16_t m_fg, m_bg;

  bool m_auto_center = false;

  bool m_is_highlighted = false, m_was_highlighted = false;
  bool m_is_visible     =  true, m_was_visible     =  true;

  bool m_is_operational = true;

  const char *m_text;
};

/*
 * `Menu` makes creating menus easier; it's basically just a group of `Btn`s with some helpful functions included.
 */
class Menu {
public:
  Menu() {};
  ~Menu();

  bool add_btn(Btn *btn);
  bool rm_btn(uint8_t btn_idx);
  bool set_btn(uint8_t btn_idx, Btn *btn);
  Btn *get_btn(uint8_t btn_idx);
  bool purge_btn(uint8_t btn_idx);
  void purge_btns();

  uint8_t get_num_btns();

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);

  uint8_t wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  int16_t get_pressed(TouchCtrl &tch, TftCtrl &tft);

  void deselect_all();

protected:
  Btn **m_btns = nullptr;
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

// Functions to get pre-set layouts of the different `MenuKeyboard`s

KeyboardLayout &get_glob_kbd_hex_layout();
KeyboardLayout &get_glob_kbd_str_layout();

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
    TftCtrl &tft, uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, KeyboardLayout &layout, float btn_height = 1.2
  );

  ~MenuKeyboard();

  void update_val(char c);
  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg);

  void get_val(char *buf, uint8_t len);
  char *get_ptr_val();
  void set_val(const char *buf, uint8_t len);

  // Placeholder: only checks for debounce
  bool handle_key(uint8_t key);

  KeyboardLayout &get_layout();

  // REMEMBER TO OVERRIDE BUFFER LENGTH IN CHILD CLASS!
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
template <typename T>
class MenuHexInput : public MenuKeyboard {
public:
  // param `val_size`: 1 = 8 bits, 2 = 16 bits, etc
  MenuHexInput(TftCtrl &tft, uint16_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h)
    : MenuKeyboard(tft, t_debounce, pad_v, pad_h, marg_v, marg_h, get_glob_kbd_hex_layout(), 1) {
    m_val = (char *) malloc(BUF_LEN() * sizeof(char)); // NOLINT(cppcoreguidelines-prefer-member-initializer): init list taken by delegated ctor

    for (uint8_t i = 0; i < BUF_LEN(); ++i) {
      m_val[i] = '0';
    }

    m_val[BUF_LEN()] = '\0';
  }

  ~MenuHexInput() {} // Ensure that base destructor(s) are called.

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

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
    MenuKeyboard::show_val(tft, x, y, BUF_LEN(), size, fg, bg);
  }

  T get_int_val() {
    T result = 0;
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
  MenuStrInput(TftCtrl &tft, uint16_t debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, uint8_t buf_len);
  ~MenuStrInput() {} // Ensure that base destructor(s) are called.

  void update_val(char c);

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg);

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
  )
    : m_pad_v(pad_v), m_pad_h(pad_h), m_marg_v(marg_v), m_marg_h(marg_h), m_num_cols(num_cols),
    m_btn_height(btn_height), m_btn_height_px(btn_height_px) {
    select(initial_choice);
  }

  ~MenuChoice() {} // Ensure that base destructor(s) are called.

  // Function that gets run when user presses a button
  typedef void (*Callback)(TftCtrl &tft, uint8_t btn_id, bool is_confirm);

  void set_callback(Callback callback);
  Callback get_callback();

  void set_choice(uint8_t choice);
  void select(uint8_t btn);

  void update(TftCtrl &tft);

  bool add_btn(Btn *btn);
  bool add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg);
  bool add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE);

  void set_confirm_btn(uint8_t btn_id);

  uint8_t wait_for_value(TouchCtrl &tch, TftCtrl &tft);

protected:
  uint8_t m_pad_v, m_pad_h, m_marg_v, m_marg_h;
  uint8_t m_num_cols;

  float m_btn_height;
  bool m_btn_height_px;

  uint8_t m_confirm_btn = 0;
  uint8_t m_cur_choice = 0;
  uint8_t m_old_choice = 0;

  Callback m_callback = [](TftCtrl &tft, uint8_t btn_id, bool is_confirm) -> void {
    UNUSED_VAR(tft);
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
    TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, bool force_bottom, uint8_t initial_choice = 0
  );

  ~MenuYesNo() {} // Ensure that base destructor(s) are called.
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
    TftCtrl &tft, uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
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
  TftCtrl &m_tft;

  uint16_t m_max_val, m_cur_val = 0;
  uint16_t m_x, m_y, m_w, m_h;
  uint16_t m_color_frac, m_color_perc, m_color_bar1, m_color_bar2;
};

};

#endif
