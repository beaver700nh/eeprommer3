#include <Arduino.h>
#include "constants.hpp"

#include <Wire.h>

#include "comm.hpp"
#include "eeprom.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

void draw_intro(uint16_t x, uint16_t y, TftBtn *skip_btn);
void mainprog();

void check_packet();

TftCtrl tft;
TouchCtrl tch(TS_XP, TS_XM, TS_YP, TS_YM, TS_RESIST);
SdCtrl sd(SD_CS, SD_EN);
EepromCtrl ee;

void setup() {
  delay(1000);

  Serial.begin(115200);
  Serial.println(F("=== EEPROMMER3 ==="));
  Serial.println(F("> Starting up... <"));

  tft.init(TFT_DRIVER, 1);
  tft.fillScreen(TftColor::BLACK);
  SER_LOG_PRINT("Initialized TFT!");

  ee.init();
  SER_LOG_PRINT("Initialized EEPROM!");

  SdCtrl::Status res = sd.init();
  SER_LOG_PRINT("Initializing SD...");

  uint16_t intro_x = (tft.width() - 320) / 2;
  uint16_t intro_y = (tft.height() - 240) / 2 - 17;

  TftBtn skip_btn(80, intro_y + 250, 320, 24, "Skip", TftColor::WHITE, TftColor::DGREEN);
  skip_btn.draw(tft);

  draw_intro(intro_x, intro_y, &skip_btn);

  if (res == SdCtrl::Status::OK) SER_LOG_PRINT("... success!");
  else                           SER_LOG_PRINT("... failed!");

  if      (res == SdCtrl::Status::OK)       tft.drawText(90, 241, "SD init success!",   TftColor::GREEN,   2);
  else if (res == SdCtrl::Status::DISABLED) tft.drawText(90, 241, "SD card disabled!",  TftColor::ORANGE,  2);
  else if (res == SdCtrl::Status::FAILED)   tft.drawText(90, 241, "SD init failed!",    TftColor::RED,     2);
  else                                      tft.drawText(90, 241, "SD invalid status!", TftColor::MAGENTA, 2);

  Util::skippable_delay(2000, LAMBDA_IS_TCHING_BTN(&skip_btn, tch, tft));

  Serial.println(F("Hello, world!"));
  Serial.println();

  mainprog();
}

void draw_intro(uint16_t x, uint16_t y, TftBtn *skip_btn) {
  if (sd.is_enabled()) {
    tft.drawRGBBitmapFromFile(x, y, "startup.bin", 320, 240, true, LAMBDA_IS_TCHING_BTN(skip_btn, tch, tft));
  }
  else {
    tft.drawThickRect(x, y, 320, 240, TftColor::CYAN, 4);
    tft.drawText(TftCalc::t_center_x_l(tft, 10, 4), y + 10, "EEPROMMER3",         TftColor::ORANGE, 4);
    tft.drawText(TftCalc::t_center_x_l(tft, 15, 2), y + 50, "by @beaver700nh",    TftColor::BLUE,   2);
    tft.drawText(TftCalc::t_center_x_l(tft, 18, 2), y + 90, "made using Arduino", TftColor::CYAN,   2);

    tft.fillCircle   (x + 135, y + 140, 20,     TftColor::CYAN);
    tft.fillRect     (x + 175, y + 120, 40, 40, TftColor::DGRAY);
    tft.drawThickRect(x + 175, y + 120, 40, 40, TftColor::LGRAY, 2);

    tft.drawText(x + 10, y + 190, "Loading...", TftColor::PURPLE, 3);

    Util::skippable_delay(2000, LAMBDA_IS_TCHING_BTN(skip_btn, tch, tft));
  }
}

void mainprog() {
  tft.fillScreen(TftColor::BLACK);

  if (sd.is_enabled()) {
    SER_LOG_PRINT("> COMMENCE SD PROGRAMMING <");

    ProgrammerFromSd prog(CONTROLLERS);
    prog.init();
    prog.run();
  }
  else {
    SER_LOG_PRINT("> COMMENCE SERIAL PROGRAMMING <");

    while (true) {
      check_packet();
    }
  }
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
