#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"

#include "error.hpp"

void Dialog::show_error(TftCtrl &tft, TouchCtrl &tch, uint8_t lvl, const char *title, const char *msg) {
  tft.drawText(10, 10, title, TftColor::CYAN, 3);

  auto name  = ErrorLevel::NAMES[lvl];
  auto color = ErrorLevel::COLORS[lvl];

  char tag[16];
  snprintf(tag, 15, "[%s]", name);

  tft.drawText(TftCalc::right(tft, TftCalc::t_width(tag, 2), 10), 10, tag, color, 2);

  char *const _msg = strdup(msg);
  char *cur_line = _msg;
  uint16_t y = 50;

  while (cur_line != nullptr) {
    char *end_line = strchr(cur_line, '\n');

    if (end_line != nullptr) *end_line = '\0';
    // if it is null (reached end of string) then the end of the line is already null

    tft.drawText(10, y, cur_line, TftColor::PURPLE);
    y += 35;

    cur_line = (end_line == nullptr ? nullptr : end_line + 1);
  }

  free(_msg);

  TftUtil::wait_bottom_btn(tft, tch, (lvl >= ErrorLevel::WARNING ? "OK" : "Continue"));
}
