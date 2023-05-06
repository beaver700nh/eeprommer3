#include <Arduino.h>
#include "constants.hpp"

#include <Wire/src/Wire.h>

#include "comm.hpp"
#include "eeprom.hpp"
#include "gui.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_util.hpp"
#include "util.hpp"
#include "xram.hpp"

SdCtrl::Status initialize();
void draw_intro(uint16_t x, uint16_t y, Gui::Btn *skip_btn);
void mainprog();

void check_packet();

// Global

TftCtrl tft;
TouchCtrl tch(TS_XP, TS_XM, TS_YP, TS_YM, TS_RESIST);
SdCtrl sd(SD_CS, SD_EN);
EepromCtrl ee;

void setup() {
  delay(1000); // Stabilization delay or else stuff like Serial can be glitchy

  SdCtrl::Status sd_status = initialize();

  const uint16_t intro_x = (tft.width() - 320) / 2;
  const uint16_t intro_y = (tft.height() - 240) / 2 - 17;

  Gui::Btn skip_btn(80, intro_y + 250, 320, 24, Strings::L_SKIP, TftColor::WHITE, TftColor::DGREEN);
  skip_btn.draw();

  draw_intro(intro_x, intro_y, &skip_btn);

  if (sd_status == SdCtrl::Status::OK) {
    SER_LOG_PRINT("... success!\n");

    tft.drawText_P(90, 241, Strings::L_SD_GOOD, TftColor::GREEN, 2);
  }
  else {
    SER_LOG_PRINT("... failed!\n");

    switch (sd_status) {
    case SdCtrl::Status::DISABLED: tft.drawText_P(90, 241, Strings::L_SD_DISAB, TftColor::ORANGE,  2); break;
    case SdCtrl::Status::FAILED:   tft.drawText_P(90, 241, Strings::L_SD_FAIL,  TftColor::RED,     2); break;
    default:                       tft.drawText_P(90, 241, Strings::L_SD_INVAL, TftColor::MAGENTA, 2); break;
    }
  }

  skip_btn.set_text(Strings::L_CONTINUE);
  skip_btn.draw();
  delay(1000);  // Delay so button isn't accidentally pressed too early
  skip_btn.wait_for_press();

  SER_LOG_PRINT("\n");

  mainprog();
}

SdCtrl::Status initialize() {
  Wire.begin();

  Serial.begin(115200);
  Serial.println(F("=== EEPROMMER3 ==="));
  Serial.println(F("> Starting up... <"));
  Serial.println(F(""));

  SER_LOG_PRINT(
    "Heap configured at addresses 0x%X-0x%X.\n",  // Using %X because the output is more customizable than %p
    reinterpret_cast<uintptr_t>(__malloc_heap_start),
    reinterpret_cast<uintptr_t>(__malloc_heap_end)
  );

  xram::init(0x00, 0x01);
  auto xr = xram::test();

  char percentage[10];
  dtostrf(lround(xr.successes / 32768.0 * 1000.0 * 100.0) / 1000.0, 0, 3, percentage);

  SER_LOG_PRINT("Initialized XRAM!\n");
  SER_LOG_PRINT("- Verified %u/32768 bytes (%s%%) in %lums.\n", xr.successes, percentage, xr.time);

  tft.init(TFT_DRIVER, 1);
  tft.fillScreen(TftColor::BLACK);
  SER_LOG_PRINT("Initialized TFT!\n");

  ee.init();
  SER_LOG_PRINT("Initialized EEPROM!\n");

  SdCtrl::Status res = sd.init();
  SER_LOG_PRINT("Initialized SD...\n");

  return res;
}

void draw_intro(uint16_t x, uint16_t y, Gui::Btn *skip_btn) {
  if (sd.is_enabled()) {
    tft.drawRGBBitmapFromFile(x, y, "startup.bin", 320, 240, true, TftUtil::Lambdas::is_tching_btn(*skip_btn));
  }
  else {
    tft.drawThickRect(x, y, 320, 240, TftColor::CYAN, 4);
    tft.drawText_P(TftCalc::t_center_x(tft, 10, 4), y + 10, Strings::L_PROJ_NAME, TftColor::ORANGE, 4);
    tft.drawText_P(TftCalc::t_center_x(tft, 15, 2), y + 50, Strings::L_INTRO1,    TftColor::BLUE,   2);
    tft.drawText_P(TftCalc::t_center_x(tft, 18, 2), y + 90, Strings::L_INTRO2,    TftColor::CYAN,   2);

    tft.fillCircle   (x + 135, y + 140, 20,     TftColor::CYAN);
    tft.fillRect     (x + 175, y + 120, 40, 40, TftColor::DGRAY);
    tft.drawThickRect(x + 175, y + 120, 40, 40, TftColor::LGRAY, 2);

    tft.drawText_P(x + 10, y + 190, Strings::W_LOAD, TftColor::PURPLE, 3);

    Util::skippable_delay(2000, TftUtil::Lambdas::is_tching_btn(*skip_btn));
  }
}

void mainprog() {
  tft.fillScreen(TftColor::BLACK);

  SER_LOG_PRINT("> HELLO WORLD <\n");

  Programmer prog;
  prog.init();
  prog.run();
}

void check_packet() {
  Packet this_packet;

  if (Serial.available() > 0 && read_packet(&this_packet)) {
    uint16_t color = TftColor::RED;

    switch (this_packet.type) {
    case PacketType::NONE:    color = TftColor::MAGENTA; break;
    case PacketType::WAITING: color = TftColor::YELLOW;  break;
    case PacketType::DATA:    color = TftColor::CYAN;    break;
    case PacketType::CMD:     color = TftColor::GREEN;   break;
    }

    tft.fillScreen(TftColor::BLACK);
    tft.drawText(0, 0, this_packet.contents, color);
  }
}

// Empty and unused loop function
void loop() {}
