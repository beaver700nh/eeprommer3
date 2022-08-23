#include <Arduino.h>
#include "constants.hpp"

#include <stdarg.h>

#include "gui.hpp"
#include "util.hpp"

#include "dialog.hpp"

uint8_t Dialog::ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt, int8_t cols, int32_t btn_height, int16_t initial_choice, uint8_t num, ...
) {
  va_list args;
  va_start(args, num);

  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  if (cols < 0) {
    cols = (int8_t) floor(sqrt(num));

    // param num...   |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | etc...
    // ---------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+------~
    // calc'd cols... |  1 |  1 |  1 |  2 |  2 |  2 |  2 |  2 |  3 |  3 |  3 |  3 |  3 |  3 |  3 |  4 | etc...
  }

  Gui::MenuChoice menu(10, 10, 50, 10, cols, (btn_height < 0 ? 24 : btn_height), true, (initial_choice < 0 ? 0 : initial_choice));

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

bool Dialog::ask_yesno(TftCtrl &tft, TouchCtrl &tch, const char *prompt, int16_t initial_choice) {
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuYesNo menu(tft, 10, 10, 50, 10, true, (initial_choice < 0 ? 0 : initial_choice));
  uint8_t btn_pressed = menu.wait_for_value(tch, tft);

  return btn_pressed == 0;
}

void Dialog::ask_str(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *buf, uint8_t len) {
  SER_LOG_PRINT("Hello from ask_str()!\n");
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Memory::print_ram_analysis();
  Gui::MenuStrInput menu(tft, T_DEBOUNCE, 10, 10, 50, 10, len);
  Memory::print_ram_analysis();
  menu.draw(tft);

  while (true) {  // Loop to get a val
    menu.show_val(tft, 10, 240, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == menu.get_num_btns() - 1) break;

    menu.handle_key(btn_pressed);
  }

  menu.get_val(buf, len);
}
