#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "util.hpp"

#include "error.hpp"

extern TftCtrl tft;

void Dialog::show_error(uint8_t lvl, uint8_t str_types, const char *title, const char *msg) {
  TftCtrl::drawText_t printer = (str_types & 0x1 ? &TftCtrl::drawText_P : &TftCtrl::drawText);
  
  (tft.*printer)(10, 10, title, TftColor::CYAN, 3);

  const char *name = Util::strdup_P(ErrorLevel::NAMES[lvl]);
  uint16_t color = ErrorLevel::COLORS[lvl];

  char tag[16];
  SNPRINTF(tag, "[%s]", name);

  free((void *) name);

  const uint16_t x = TftCalc::right(tft, TftCalc::t_width(strlen(tag), 2), 10);
  tft.drawText(x, 10, tag, color, 2);

  char *const _msg = (str_types & 0x2 ? Util::strdup_P : strdup)(msg);
  char *cur_line   = _msg;
  uint16_t y       = 50;

  while (cur_line != nullptr) {
    char *end_line = strchr(cur_line, '\n');

    if (end_line != nullptr) *end_line = '\0';
    // if it is null (reached end of string) then the end of the line is already null

    tft.drawText(10, y, cur_line, TftColor::PURPLE, 2);
    y += 35;

    cur_line = (end_line == nullptr ? nullptr : end_line + 1);
  }

  free(_msg);

  TftUtil::wait_bottom_btn(lvl >= ErrorLevel::WARNING ? Strings::L_OK : Strings::L_CONTINUE);
}
