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
namespace TftColor {
  enum : uint16_t {
    RED     = 0xF800,
    ORANGE  = 0xFCE3,
    YELLOW  = 0xFFE0,
    DGREEN  = 0x03E0,
    GREEN   = 0x07E0,
    CYAN    = 0x07FF,
    BLUE    = 0x001F,
    PURPLE  = 0xE31D,
    MAGENTA = 0xF81F,
    PINKK   = 0xFEF7,
    BLACK   = 0x0000,
    DGRAY   = 0x39E7,
    GRAY    = 0x7BEF,
    LGRAY   = 0xBDF7,
    WHITE   = 0xFFFF,
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
  TftBtn() {};
  TftBtn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t tx, uint16_t ty,
    const char *text, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  TftBtn(
    uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *text,
    uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE
  );

  uint16_t get_x();
  void     set_x(uint16_t x);
  uint16_t get_y();
  void     set_y(uint16_t y);

  uint16_t get_w();
  void     set_w(uint16_t w);
  uint16_t get_h();
  void     set_h(uint16_t h);

  uint16_t get_tx();
  void     set_tx(uint16_t tx);
  uint16_t get_ty();
  void     set_ty(uint16_t ty);

  uint16_t get_fg();
  void     set_fg(uint16_t fg);
  uint16_t get_bg();
  void     set_bg(uint16_t bg);

  const char *get_text();
  void        set_text(const char *text);

  void draw(TftCtrl &tft);
  void erase(TftCtrl &tft);
  void draw_highlight(TftCtrl &tft);

  // Controls whether a yellow outline appears around the button.
  void highlight(bool highlight);
  // Default is unhighlighted.
  bool is_highlighted();

  // Controls whether the button is visible.
  void visibility(bool invisibility);
  // Default is true.
  bool is_visible();

  // Controls whether the button responds to presses.
  void operation(bool operation);
  // Default is true.
  bool is_operational();

  void wait_for_press(TouchCtrl &tch, TftCtrl &tft);
  bool is_pressed(TouchCtrl &tch, TftCtrl &tft);

private:
  uint16_t m_x, m_y;
  uint16_t m_w, m_h;
  uint16_t m_tx, m_ty;
  uint16_t m_fg, m_bg;

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

protected:
  TftBtn **m_btns = nullptr;
  uint8_t m_num_btns = 0;
};

/*
 * TftKeyboardMenu is a TftMenu but specialized
 * for getting aggregate values from the user
 * such as strings and numbers via the use of a
 * keyboard.
 * 
 * This is a basecClass and is designed to be
 * inherited from, not used directly
 */
class TftKeyboardMenu : public TftMenu {
public:
  // height of button = width of button * param btn_height
  TftKeyboardMenu(
    TftCtrl &tft, uint8_t t_debounce,
    uint16_t pad_v, uint16_t pad_h,
    uint16_t marg_v, uint16_t marg_h,
    uint8_t num_cols, float btn_height = 1.2
  );

  void update_val(char c);
  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t size, uint16_t fg, uint16_t bg);

  void get_val(char *buf, uint8_t len);
  char *get_ptr_val();
  void set_val(const char *buf);

  // REMEMBER TO OVERRIDE IN CHILD CLASS!
  inline static const uint8_t BUF_LEN = 0;
  // REMEMBER TO OVERRIDE IN CHILD CLASS!
  inline static const uint8_t KBD_WIDTH = 0;
  // REMEMBER TO OVERRIDE IN CHILD CLASS!
  inline static const uint8_t KBD_HEIGHT = 0;
  // REMEMBER TO OVERRIDE IN CHILD CLASS!
  inline static const uint8_t *KBD_LAYOUT;

private:
  char val[BUF_LEN + 1] = {'\0'};

  unsigned long long t_last_press = 0;
  uint8_t t_debounce;
};

/*
 * TftHexSelMenu is a TftMenu but specialized
 * for inputting numbers in hexadecimal.
 *
 * Type T is an integer type and determines what
 * numbers can be inputted.
 */
template<typename T>
class TftHexSelMenu : public TftMenu {
public:
  TftHexSelMenu(TftCtrl &tft, uint16_t top_margin, uint16_t side_margin) {
    const uint16_t cell_margin = 10;
    const uint16_t cell_size = (tft.width() - 7 * cell_margin - 2 * side_margin) / 8;
    const uint16_t cell_dist = cell_size + cell_margin;
    const uint16_t text_margin = (cell_size - 10) / 2;

    for (uint8_t i = 0x00; i < 0x10; ++i) {
      uint16_t x = side_margin + cell_dist * (i % 8);
      uint16_t y = (i < 8 ? top_margin : top_margin + cell_size + cell_margin);
  
      add_btn(new TftBtn(x, y, cell_size, cell_size, text_margin, text_margin, STRFMT_NOBUF("%1X", i), TftColor::WHITE, TftColor::BLUE));
    }
  }

  void update_val(uint8_t k) {
    m_val = (m_val << 4) + k;
  }

  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t font_size, uint16_t fg, uint16_t bg) {
    tft.fillRect(x, y, tft.width() - x, 8 * font_size, bg);

    char strfmt_buf[50];
    sprintf(strfmt_buf, "Val: [%%0%dX]", BIT_WIDTH(T) / 4);

    tft.drawText(x, y, STRFMT_NOBUF(strfmt_buf, m_val), fg, font_size);
  }

  T    get_val()      { return m_val; }
  void set_val(T val) { m_val = val;  }

private:
  T m_val = 0;
};

/*
 * TftStringMenu is a TftMenu but specialized
 * for inputting strings.
 */
class TftStringMenu : public TftMenu {
public:
  TftStringMenu(TftCtrl &tft, uint16_t top_margin, uint8_t side_margin, uint8_t padding);

  void update_val(char k);
  void show_val(TftCtrl &tft, uint16_t x, uint16_t y, uint8_t len, uint8_t font_size, uint16_t fg, uint16_t bg);

  void get_val(char *buf, uint8_t len);
  char *get_ptr_val();
  void set_val(const char *buf);

  static const char *const get_ptr_char(uint8_t x, uint8_t y);
  static char get_char(uint8_t x, uint8_t y);

  inline static const uint8_t LEN = 255;

  inline static const uint8_t LAYOUT_WIDTH = 11;
  inline static const uint8_t LAYOUT_HEIGHT = 4;

  inline static const char *const LAYOUT = (
    "\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00\x38\x00\x39\x00\x30\x00\x7e\x00"
    "\x51\x00\x57\x00\x45\x00\x52\x00\x54\x00\x59\x00\x55\x00\x49\x00\x4f\x00\x50\x00\x11\x00"
    "\x41\x00\x53\x00\x44\x00\x46\x00\x47\x00\x48\x00\x4a\x00\x4b\x00\x4c\x00\x5f\x00\x2d\x00"
    "\x7f\x00\x5a\x00\x58\x00\x43\x00\x56\x00\x42\x00\x4e\x00\x4d\x00\xb0\x00\x2c\x00\x2e\x00"
  );

private:
  char m_val[LEN + 1] = {'\0'};

  bool m_capitalize = false;
  unsigned long long m_t_press;
};

/*
 * TftChoiceMenu is a specialized TftMenu to
 * choose one option from a selection of choices.
 */
class TftChoiceMenu : public TftMenu {
public:
  TftChoiceMenu(
    uint8_t v_margin, uint8_t h_margin,
    uint8_t v_padding, uint8_t h_padding,
    uint8_t num_cols, uint16_t btn_height,
    uint8_t initial_choice = 0
  );

  typedef void (*Callback)(TftCtrl &tft, uint8_t btn_id, bool is_confirm);

  void set_callback(Callback callback);
  Callback get_callback();

  void update(TftCtrl &tft);

  bool add_btn(TftBtn *btn);
  bool add_btn_calc(TftCtrl &tft, const char *text, uint16_t fg, uint16_t bg);
  bool add_btn_confirm(TftCtrl &tft, bool force_bottom, uint16_t fg = TftColor::BLACK, uint16_t bg = TftColor::WHITE);
  uint8_t wait_for_value(TouchCtrl &tch, TftCtrl &tft);

protected:
  uint8_t m_v_margin, m_h_margin, m_v_padding, m_h_padding;
  uint8_t m_num_cols, m_btn_height;
  uint8_t m_confirm_btn = 0;
  uint8_t m_cur_choice = 0;
  uint8_t m_old_choice = 0;

  Callback m_callback = [](TftCtrl &tft, uint8_t btn_id, bool is_confirm) -> void {
    (void) tft; // Silence unused parameter
    (void) btn_id; // Silence unused parameter
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
    TftCtrl &tft,
    uint8_t v_margin, uint8_t h_margin,
    uint8_t v_padding, uint8_t h_padding,
    bool force_bottom, uint8_t initial_choice = 0
  );
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
  tft.drawText(10, 10, prompt, TftColor::CYAN, 4);

  TftHexSelMenu<T> menu(tft, 50, 17);
  menu.add_btn(new TftBtn(BOTTOM_BTN(tft, "Continue")));
  menu.draw(tft);

  while (true) { // Loop to get a val
    menu.show_val(tft, 10, 170, 4, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == 16) break;

    menu.update_val(btn_pressed);
  }

  return menu.get_val();
}

/*
 * This is a function to ask the user to pick
 * from one of `num` choices.
 * 
 * This function is variadic, so be careful
 * that you provide a valid `num`.
 */
uint8_t ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt,
  int8_t cols, int32_t btn_height, int16_t initial_choice,
  uint8_t num, ...
);

/*
 * This is a helper function to ask the user
 * for an arbitrarily-sized string
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
 * TFT, to help identify special characters.
 */
void tft_print_chars(TftCtrl &tft);

#endif

#endif
