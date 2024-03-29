#include <Arduino.h>
#include "constants.hpp"

#include <stdarg.h>

#include "gui.hpp"
#include "util.hpp"

#include "dialog.hpp"

extern TftCtrl tft;
extern TouchCtrl tch;

uint16_t Dialog::ask_addr(const char *prompt) {
  uint16_t addr = ask_int<uint16_t>(prompt);
  Util::validate_addr(&addr);

  return addr;
}

uint8_t Dialog::ask_choice(const char *prompt, int8_t cols, int32_t btn_height, int16_t initial_choice, uint8_t num, ...) {
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

    menu.add_btn_calc(text, fg_color, bg_color);
  }

  menu.add_btn_confirm(true);

  va_end(args);

  uint8_t val = menu.wait_for_value();
  return val;
}

bool Dialog::ask_yesno(const char *prompt, int16_t initial_choice) {
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuYesNo menu(10, 10, 50, 10, true, (initial_choice < 0 ? 0 : initial_choice));
  uint8_t btn_pressed = menu.wait_for_value();

  return btn_pressed == 0;
}

void Dialog::ask_str(const char *prompt, char *buf, uint8_t len) {
  tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuStrInput menu(T_DEBOUNCE, 10, 10, 50, 10, len);
  menu.draw();

  while (true) {  // Loop to get a val
    menu.show_val(10, 240, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press();

    if (btn_pressed == menu.get_num_btns() - 1) break;

    menu.handle_key(btn_pressed);
  }

  menu.get_val(buf, len);
}

bool Dialog::ask_pairs(const char *prompt, AddrDataArray *buf) {
  using PStatus = Gui::MenuPairs::Status;

  Gui::MenuPairs menu(40, 10, 10, 22, 8, 7, buf);
  PStatus status = PStatus::RUNNING;

  do {
    tft.drawText_P(10, 10, prompt, TftColor::CYAN, 3);
    menu.draw();

    status = menu.poll();
  }
  while (status == PStatus::RUNNING);

  return status == PStatus::DONE;
}
