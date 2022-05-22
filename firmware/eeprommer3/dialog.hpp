#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "touch.hpp"

/*
 * This file contains functions for asking the user for input using Gui::* classes.
 */

namespace Dialog {

/*
 * This is a helper function to ask the user for an arbitrarily-sized integer using a `MenuHexInput`.
 * Type `T` is an integer type that determines the kind of integer for which the user is asked.
 */
template<typename T>
T ask_int(TftCtrl &tft, TouchCtrl &tch, const char *prompt) {
  tft.drawText(10, 10, prompt, TftColor::CYAN, 3);

  Gui::MenuHexInput<T> menu(tft, T_DEBOUNCE, 10, 10, 50, 17);
  menu.draw(tft);

  while (true) { // Loop to get a val
    menu.show_val(tft, 10, 170, 3, TftColor::ORANGE, TftColor::BLACK);

    uint8_t btn_pressed = menu.wait_for_press(tch, tft);

    if (btn_pressed == 16) break;

    menu.handle_key(btn_pressed);
  }

  return menu.get_int_val();
}

/*
 * This is a function to ask the user to pick from one of `num` choices using a `MenuChoice`.
 *
 * This function is variadic, so be careful that you provide a valid `num`.
 * Vararg format: text, fg, bg
 */
uint8_t ask_choice(
  TftCtrl &tft, TouchCtrl &tch, const char *prompt, int8_t cols, int32_t btn_height, int16_t initial_choice, uint8_t num, ...
);

/*
 * This is a function to ask the user a yes or no question, using a `MenuYesNo`.
 */
bool ask_yesno(TftCtrl &tft, TouchCtrl &tch, const char *prompt, int16_t initial_choice = -1);

/*
 * This is a helper function to ask the user for an arbitrarily-sized string, using a `MenuStrInput`.
 */
void ask_str(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *buf, uint8_t len);

};

#endif
