#include <Arduino.h>
#include "constants.hpp"

#include <TouchScreen.h>

#include "comm.hpp"
#include "eeprom.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

void mainprog();

void check_packet();

TftCtrl         tft(TFT_CS, TFT_RS, TFT_WR, TFT_RD, TFT_RESET);
SdCtrl          sd(SD_CS, SD_EN);
TouchscreenCtrl ts(TS_XP, TS_YP, TS_XM, TS_YM, TS_RESIST);

bool using_sd = false;

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.init(TFT_DRIVER, 3);
  tft.fillScreen(TftColor::BLACK);

  uint8_t res = sd.init();
  using_sd = (res == 0);

  if      (res == 0) tft.drawText(5, 216, "SD has been initialized!", TftColor::GREEN,   2);
  else if (res == 1) tft.drawText(5, 216, "SD support was disabled!", TftColor::ORANGE,  2);
  else if (res == 2) tft.drawText(5, 216, "SD failed to initialize!", TftColor::RED,     2);
  else               tft.drawText(5, 216, "SD init -> bad err code!", TftColor::MAGENTA, 2);

  mainprog();
}

void mainprog() {
  EepromCtrl ee;

  if (using_sd) {
    TftMenu menu;

    menu.add_btn(new TftBtn(10, 10, 100, 20, "Hello :)"));
    menu.add_btn(new TftBtn(10, 30, 100, 20, "World :)"));

//    TftBtn b(10, 10, 100, 20, "A BUTTON");

    while (true) {
//      b.draw(tft);
//
//      if (b.is_pressed(ts)) {
//        tft.drawText(10, 50, "Press!", TftColor::PINKK, 3);
//      }

      menu.draw(tft);

      uint8_t btn_pressed = menu.wait_any_btn_down(ts);
      menu.wait_all_btn_up(ts);

      char buf[50];
      sprintf(buf, "Press: btn #%d", btn_pressed);

      tft.drawText(10, 100, buf, TftColor::CYAN, 3);

      // draw btns to select settings

      // prog.<action>();
    }
  }
  else {
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

void loop() {}
