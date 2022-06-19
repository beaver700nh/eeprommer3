#ifndef TFT_HPP
#define TFT_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <MCUFRIEND_kbv.h>
#include <SD.h>

#include "touch.hpp"

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
 * `TftCtrl` is the main class to interface with the TFT; it is a wrapper
 * around the 3rd-party class `MCUFRIEND_kbv` - prenticedavid/MCUFRIEND_kbv (GitHub).
 */
class TftCtrl : public MCUFRIEND_kbv {
public:
  TftCtrl() {};

  void init(uint16_t driver_id, uint8_t orientation);

  void drawThickRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint8_t thickness);

  void drawText(uint16_t x, uint16_t y, const char *text, uint16_t color = TftColor::WHITE, uint8_t size = 2);
  void drawTextBg(uint16_t x, uint16_t y, const char *text, uint16_t color = TftColor::WHITE, uint16_t bg = TftColor::BLACK, uint8_t size = 2);
  void drawText(const char *text);

  template<typename Func>
  bool drawRGBBitmapFromFile(uint16_t x, uint16_t y, const char *file, uint16_t width, uint16_t height, bool swap_endian, Func check_skip) {
    bool success = true;

    File f = SD.open(file);
    if (!f) return false;

    const size_t row_size_bytes = width * sizeof(uint16_t);
    auto *buf = (uint16_t *) malloc(row_size_bytes);

    if (buf == nullptr) {
      success = false;
    }
    else {
      for (uint16_t j = 0; j < height; ++j) {
        int16_t res = f.read((uint8_t *) buf, row_size_bytes);

        if (res < 0) {
          success = false;
          break;
        }

        if (swap_endian) {
          for (uint16_t i = 0; i < width; ++i) {
            buf[i] = (buf[i] << 8) | (buf[i] >> 8);
          }
        }

        setAddrWindow(x, y + j, x + width, y + j);
        pushColors(buf, width, true);

        if (check_skip()) break;
      }
    }

    f.close();
    free(buf);
    return success;
  }
};

// Forward declaration because of circular dependency
class TouchCtrl;

/*
 * Debug function for testing touchscreen and TFT screen. Press left edge of screen to quit.
 */
void tft_draw_test(TouchCtrl &tch, TftCtrl &tft);

/*
 * Quick little function to print the character set of the TFT, to help identify special characters like arrows for use in the GUI.
 */
void tft_print_chars(TftCtrl &tft);

/*
 * Function to test showing colors on the TFT screen.
 */
void tft_show_colors(TftCtrl &tft);

#endif
