#ifndef TFT_HPP
#define TFT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>

#include "input.hpp"

/*
 * This file contains classes for various
 * front-end purposes.
 */

// Removes color constants from MCUFRIEND_kbv library
#undef TFT_BLACK
#undef TFT_NAVY
#undef TFT_DARKGREEN
#undef TFT_DARKCYAN
#undef TFT_MAROON
#undef TFT_PURPLE
#undef TFT_OLIVE
#undef TFT_LIGHTGREY
#undef TFT_DARKGREY
#undef TFT_BLUE
#undef TFT_GREEN
#undef TFT_CYAN
#undef TFT_RED
#undef TFT_MAGENTA
#undef TFT_YELLOW
#undef TFT_WHITE
#undef TFT_ORANGE
#undef TFT_GREENYELLOW
#undef TFT_PINK

// Makes specifying TFT colors easier
// Note: adjusted for my personal TFT - might need tweaking to work with others
namespace TftColor {
  enum : uint16_t {
    // Standard full-intensity colors (except gray, which is half-intensity)

    RED     = 0xF800,
    YELLOW  = 0xFFE0,
    GREEN   = 0x07E0,
    CYAN    = 0x07FF,
    BLUE    = 0x001F,
    MAGENTA = 0xF81F,
    BLACK   = 0x0000,
    GRAY    = 0x7BEF,
    WHITE   = 0xFFFF,

    // Other miscellaneous colors
    // Some of these are thanks to prenticedavid/MCUFRIEND_kbv (GitHub)

    DRED   = 0x7800,
    ORANGE = 0xFCE3,
    LIME   = 0xB7E0,
    LGREEN = 0x7FEF,
    DGREEN = 0x03E0,
    OLIVE  = 0x7BE0,
    DCYAN  = 0x03EF,
    DBLUE  = 0x000F,
    PURPLE = 0xE31D,
    PINKK  = 0xFEF7,
    LGRAY  = 0xBDF7,
    DGRAY  = 0x39E7,
  };
};

/*
 * TftCtrl is the main class to interface
 * with the TFT; it is a wrapper around the
 * 3rd-party class MCUFRIEND_kbv.
 */
class TftCtrl : public MCUFRIEND_kbv {
public:
  TftCtrl() {};

  void init(uint16_t driver_id, uint8_t orientation);

  void drawText(uint16_t x, uint16_t y, const char *text, uint16_t color = TftColor::WHITE, uint8_t size = 2);
  void drawTextBg(uint16_t x, uint16_t y, const char *text, uint16_t color = TftColor::WHITE, uint16_t bg = TftColor::BLACK, uint8_t size = 2);
  void drawText(const char *text);

  bool drawRGBBitmapFromFile(
    uint16_t x, uint16_t y, const char *file, uint16_t width, uint16_t height,
    bool swap_endian, bool (*check_skip)()
  );
};

/*
 * A set of functions to help calculate the
 * positions of elements on a TFT screen
 */
namespace TftCalc {
  uint16_t t_center_x(uint16_t box, const char *text, uint8_t size);
  uint16_t t_center_x(TftCtrl &tft, const char *text, uint8_t size);

  uint16_t t_center_x_l(uint16_t box, uint8_t len, uint8_t size);
  uint16_t t_center_x_l(TftCtrl &tft, uint8_t len, uint8_t size);

  uint16_t t_center_y(uint16_t box, uint8_t size);
  uint16_t t_center_y(TftCtrl &tft, uint8_t size);

  uint16_t fraction(uint16_t box, uint16_t margin, uint8_t denom);
  uint16_t fraction_x(TftCtrl &tft, uint16_t margin, uint8_t denom);
  uint16_t fraction_y(TftCtrl &tft, uint16_t margin, uint8_t denom);

  uint16_t bottom(uint16_t box, uint16_t height, uint16_t margin);
  uint16_t bottom(TftCtrl &tft, uint16_t height, uint16_t margin);

  uint16_t right(uint16_t box, uint16_t width, uint16_t margin);
  uint16_t right(TftCtrl &tft, uint16_t width, uint16_t margin);
};

// Classes from here on are for GUI

/*
 * Forward declaration because some of the
 * GUI classes' methods depend on TouchCtrl
 */
class TouchCtrl;

/*
 * TftBtn: stores its own data,
 * can detect if it is pressed,
 * and can draw itself to the
 * TFT screen.
 */
class TftBtn {
public:
  // Default constructor; does nothing.
  TftBtn() {};

  // Full constructor.
  TftBtn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty,
    const char *text, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  // Partial constructor, enables auto centering eliminating need for tx and ty.
  TftBtn(
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

  inline uint8_t get_font_size()                  { return m_font_size; }
  inline void    set_font_size(uint8_t font_size) {
    m_font_size = font_size;
    auto_center();
  }

  inline const char *get_text()                 { return m_text; }
  inline void        set_text(const char *text) {
    m_text = text;
    auto_center();
  }

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);
  void draw_highlight(TftCtrl &tft);

  // Controls whether a yellow outline appears around the button.
  inline void highlight(bool highlight) { m_is_highlighted = highlight; }
  // Default is false/unhighlighted.
  inline bool is_highlighted() { return m_is_highlighted; }

  // Controls whether the button is visible.
  inline void visibility(bool visibility) { m_is_visible = visibility; }
  // Default is true/visible.
  inline bool is_visible() { return m_is_visible; }

  // Controls whether the button responds to presses.
  inline void operation(bool operation) { m_is_operational = operation; }
  // Default is true/operational.
  inline bool is_operational() { return m_is_operational; }

  // Controls whether the button automatically centers text. Only call this to override auto detection.
  inline void auto_centering(bool auto_center) { m_auto_center = auto_center; }
  // Default is false / manual centering; if constructed without tx/ty parameters, default is true / auto centering.
  inline bool is_auto_centering() { return m_auto_center; }

  // Sets tx and ty to position text at center of button, but only if in auto_center mode.
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
  uint16_t m_x, m_y;
  uint16_t m_w, m_h;
  uint16_t m_tx, m_ty;
  uint16_t m_fg, m_bg;

  bool m_auto_center = false;

  bool m_is_highlighted = false;
  bool m_was_highlighted = false;

  bool m_is_visible = true;
  bool m_was_visible = true;

  bool m_is_operational = true;

  const char *m_text;
};

/*
 * TftMenu makes creating menus easier;
 * it's basically just a group of buttons
 * with some helpful functions included
 */
class TftMenu {
public:
  TftMenu() {};
  virtual ~TftMenu();

  bool add_btn(TftBtn *btn);
  bool rm_btn(uint8_t btn_idx);
  bool set_btn(uint8_t btn_idx, TftBtn *btn);
  TftBtn *get_btn(uint8_t btn_idx);
  bool purge_btn(uint8_t btn_idx);
  void purge_btns();

  uint8_t get_num_btns();

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);

  uint8_t wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  int16_t get_pressed(TouchCtrl &tch, TftCtrl &tft);

  void deselect_all();

protected:
  TftBtn **m_btns = nullptr;
  uint8_t m_num_btns = 0;
};

class TftKeyboardLayout {
public:
  TftKeyboardLayout(const uint8_t *layout, uint8_t length, uint8_t width);

  const uint8_t *get_layout();

  uint8_t get_width();
  uint8_t get_height();

  const char *const get_ptr_char(uint8_t x, uint8_t y);
  char get_char(uint8_t x, uint8_t y);

private:
  const uint8_t *m_layout;

  uint8_t m_length;
  uint8_t m_width;
};

TftKeyboardLayout &get_glob_kbd_hex_layout();
TftKeyboardLayout &get_glob_kbd_str_layout();

/*
 * TftKeyboardMenu is an ABC specialization of
 * TftMenu for getting aggregate values such as
 * strings and numbers from the user via the
 * use of a keyboard.
 * 
 * THIS IS A BASE CLASS AND CANNOT BE USED
 * DIRECTLY -- USE A SUBCLASS INSTEAD!
 */
class TftKeyboardMenu : public TftMenu {
public:
  // IMPORTANT - Does not initialize internal value string, do not forget to do so
  // height of button = calculated width of button * param btn_height
  TftKeyboardMenu(
    TftCtrl &tft, uint8_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, TftKeyboardLayout &layout, float btn_height = 1.2
  );

  ~TftKeyboardMenu();

  void update_val(char c);
  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t len, uint8_t size, uint16_t fg, uint16_t bg);

  void get_val(char *buf, uint8_t len);
  char *get_ptr_val();
  void set_val(const char *buf, uint8_t len);

  TftKeyboardLayout &get_layout();

  // REMEMBER TO OVERRIDE BUFFER LENGTH IN CHILD CLASS!
  inline virtual const uint8_t BUF_LEN() {
    return 0;
  }

protected:
  char *m_val;

  unsigned long m_t_last_press = 0;
  uint8_t m_t_debounce;

  TftKeyboardLayout &m_layout;

  uint16_t m_pad_v, m_pad_h, m_marg_v, m_marg_h;
};

/*
 * TftHexSelMenu is a TftMenu but specialized
 * for inputting numbers in hexadecimal.
 *
 * Type T is an integer type and determines what
 * numbers can be inputted.
 */
template <typename T>
class TftHexSelMenu : public TftKeyboardMenu {
public:
  // param val_size: 1 = 8 bits, 2 = 16 bits, etc
  TftHexSelMenu(TftCtrl &tft, uint8_t t_debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h)
    : TftKeyboardMenu(tft, t_debounce, pad_v, pad_h, marg_v, marg_h, get_glob_kbd_hex_layout(), 1) {
    m_val = (char *) malloc(BUF_LEN() * sizeof(char));

    for (uint8_t i = 0; i < BUF_LEN(); ++i) {
      m_val[i] = '0';
    }

    m_val[BUF_LEN()] = '\0';
  }

  void update_val(char c) {
    uint8_t len = strlen(m_val);

    if (len >= BUF_LEN()) {
      for (uint8_t i = 1; i < BUF_LEN(); ++i) {
        m_val[i - 1] = m_val[i];
      }

      m_val[BUF_LEN() - 1] = c;

      return;
    }

    TftKeyboardMenu::update_val(c);
  }

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
    TftKeyboardMenu::show_val(tft, x, y, BUF_LEN(), size, fg, bg);
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

  inline virtual const uint8_t BUF_LEN() override {
    return BIT_WIDTH(T) / 4;
  }
};

/*
 * TftStringMenu is a TftKeyboardMenu but specialized
 * for inputting strings.
 */
class TftStringMenu : public TftKeyboardMenu {
public:
  TftStringMenu(TftCtrl &tft, uint8_t debounce, uint16_t pad_v, uint16_t pad_h, uint16_t marg_v, uint16_t marg_h, uint8_t buf_len);

  int16_t get_pressed(TouchCtrl &tch, TftCtrl &tft);

  void update_val(char c);

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
    uint8_t char_width = 6 * (size == 0 ? 1 : size); // Prevent divide-by-zero
    uint8_t maxfit_len = (tft.width() - size - 2 * m_marg_h) / char_width - 2;

    TftKeyboardMenu::show_val(tft, x, y, MIN(maxfit_len, m_buf_len), size, fg, bg);
  }

  char capitalize(char c);

  inline virtual const uint8_t BUF_LEN() override {
    return m_buf_len;
  }

private:
  uint8_t m_buf_len;

  bool m_capitalize = false;
};

/*
 * TftChoiceMenu is a specialized TftMenu to
 * choose one option from a selection of choices.
 * It also supports having button layouts be
 * automatically calculated based on the parameters
 * of padding and margin passed in the constructor.
 */
class TftChoiceMenu : public TftMenu {
public:
  // btn_height_px: true = btn_height uses px, false = multiplied with btn width
  TftChoiceMenu(
    uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t num_cols,
    float btn_height, bool btn_height_px, uint8_t initial_choice = 0
  );

  typedef void (*Callback)(TftCtrl &tft, uint8_t btn_id, bool is_confirm);

  void set_callback(Callback callback);
  Callback get_callback();

  void set_choice(uint8_t choice);
  void select(uint8_t btn);

  void update(TftCtrl &tft);

  bool add_btn(TftBtn *btn);
  bool add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg);
  bool add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE);
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
    (void) tft;        // Silence unused parameter
    (void) btn_id;     // Silence unused parameter
    (void) is_confirm; // Silence unused parameter
  };
};

/*
 * TftYesNoMenu is a specialized TftChoiceMenu
 * which only has the options "Yes" and "No".
 * It also automatically adds the "Confirm" button
 * upon construction.
 */
class TftYesNoMenu : public TftChoiceMenu {
public:
  TftYesNoMenu(
    TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, bool force_bottom, uint8_t initial_choice = 0
  );
};

/*
 * TftProgressIndicator is a class to show a progress
 * amount on a TftCtrl as a fraction, a percentange,
 * and a progress bar.
 *
 * It will try to only take up as much space as it is
 * allotted in the constructor.
 */
class TftProgressIndicator {
public:
  TftProgressIndicator(
    TftCtrl &tft, uint16_t max_val, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
    uint16_t color_frac = TftColor::DGREEN, uint16_t color_perc = TftColor::BLUE,
    uint16_t color_bar1 = TftColor::DRED, uint16_t color_bar2 = TftColor::WHITE
  );

// Should return true to request cancel of loop.
#define TFT_PROGRESS_INDICATOR_LAMBDA (uint8_t progress) -> bool

  // Is a template func because lambdas without libstdc++ are annoying.
  // Returns true if loop completed succesfully, false if canceled.
  template<typename Func>
  bool for_each(Func action) {
    while (m_cur_val <= m_max_val) {
      show();
      bool should_quit = action(m_cur_val);
      next();

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

/*
 * This is a helper function to ask the user for
 * an arbitrarily-sized integer.
 * 
 * Type T is the integer type that determines
 * the kind of integer for which the user is asked.
 */
template<typename T>
T ask_val(TftCtrl &tft, TouchCtrl &tch, const char *prompt) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  TftHexSelMenu<T> menu(tft, T_DEBOUNCE, 10, 10, 50, 17);
  menu.draw(tft);

  while (true) { // Loop to get a val
    menu.show_val(tft, 10, 170, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == 16) break;

    menu.update_val(
      (IN_RANGE(btn_pressed, 0, 10) ? btn_pressed + '0' : btn_pressed + 'A' - 10)
    );
  }

  return menu.get_int_val();
}

/*
 * This is a function to ask the user to pick
 * from one of `num` choices.
 * 
 * This function is variadic, so be careful
 * that you provide a valid `num`.
 */
uint8_t ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt, int8_t cols, int32_t btn_height, int16_t initial_choice, uint8_t num, ...
);

/*
 * This is a function to ask the user a
 * yes or no question, using a TftYesNoMenu.
 */
bool ask_yesno(TftCtrl &tft, TouchCtrl &tch, const char *prompt, int16_t initial_choice = -1);

/*
 * This is a helper function to ask the user
 * for an arbitrarily-sized string, using
 * a TftStringMenu.
 */
void ask_str(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *buf, uint8_t len);

/*
 * Debug function for testing touchscreen and TFT screen.
 * This function contains an infinite loop, and will never
 * finish executing.
 */
void tft_draw_test(TouchCtrl &tch, TftCtrl &tft);

#ifdef DEBUG_MODE

/*
 * Quick little function to print the character set of the
 * TFT, to help identify special characters like arrows for
 * use in the GUI.
 */
void tft_print_chars(TftCtrl &tft);

#endif

#endif
