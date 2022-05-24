#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "tft.hpp"
#include "touch.hpp"

#include "tft_util.hpp"

namespace TftUtil {
  void wait_bottom_btn(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    static Gui::Btn continue_btn(BOTTOM_BTN(tft, text));
    continue_btn.draw(tft);
    continue_btn.wait_for_press(tch, tft);
  }

  void show_error(TftCtrl &tft, TouchCtrl &tch, const char *text) {
    tft.drawText(10, 10, "Error!",   TftColor::RED,    3);
    tft.drawText(10, 50, "Message:", TftColor::CYAN,   2);

    char *const _text = strdup(text);
    char *cur_line = _text;
    uint8_t msg_line = 0;

    while (cur_line != nullptr) {
      char *end_line = strchr(cur_line, '\n');

      if (end_line != nullptr) *end_line = '\0';
      // if it is null then the end of the line is already null (string null termination)

      const uint16_t y = 85 + (35 * msg_line++);
      tft.drawText(10, y, cur_line, TftColor::PURPLE);

      cur_line = (end_line == nullptr ? nullptr : end_line + 1);
    }

    wait_bottom_btn(tft, tch, "OK");
  }
};
