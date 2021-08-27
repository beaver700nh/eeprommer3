#include <Arduino.h>
#include "constants.hpp"

#include <IoAbstractionWire.h>
#include <LiquidCrystalIO.h>
#include <TouchScreen.h>
#include <Wire.h>

#include "comm.hpp"
#include "eeprom.hpp"
#include "prog.hpp"
#include "sd.hpp"
#include "tft.hpp"

void mainprog();

void check_packet();

TftCtrl tft;
TouchCtrl tch(TS_XP, TS_XM, TS_YP, TS_YM, TS_RESIST);

SdCtrl sd(SD_CS, SD_EN);

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.init(TFT_DRIVER, 1);
  tft.fillScreen(TftColor::BLACK);

  uint8_t res = sd.init();

  if      (res == 0) tft.drawText(5, tft.height() - 24, "SD was initialized successfully!",   TftColor::GREEN,   2);
  else if (res == 1) tft.drawText(5, tft.height() - 24, "SD card support has been disabled!", TftColor::ORANGE,  2);
  else if (res == 2) tft.drawText(5, tft.height() - 24, "SD card failed to initialize!",      TftColor::RED,     2);
  else               tft.drawText(5, tft.height() - 24, "SD init returned bad error code!",   TftColor::MAGENTA, 2);

  delay(1000);

  mainprog();
}

void mainprog() {
  tft.fillScreen(TftColor::BLACK);

  while (true) {
    TSPoint p = tch.get_tft_point(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY, tft);

    if (TouchCtrl::is_valid_pressure(p.z)) {
      tft.fillCircle(p.x, p.y, 3, TftColor::RED);
    }
  }

  EepromCtrl ee;

  if (sd.is_enabled()) {
    TftMenu menu;

    TftBtn *btn1 = new TftBtn(10, 70, 100, 20, "BLAH - 1");
    TftBtn *btn2 = new TftBtn(10, 70, 100, 20, "BLAH - 2");

    menu.add_btn(new TftBtn(10, 10, 100, 20, "Hello :)"));
    menu.add_btn(new TftBtn(10, 40, 100, 20, "World :)"));
    menu.add_btn(btn1);

    while (true) {
      menu.draw(tft);

      uint8_t btn_pressed = menu.wait_for_press(tch, tft);

      tft.fillScreen(TftColor::BLACK);

      if (btn_pressed == 1) {
        menu.set_btn(2, btn1);
      }
      else if (btn_pressed == 2) {
        menu.set_btn(2, btn2);
      }

      char buf[50];
      sprintf(buf, "Press: '%s'", menu.get_btn(2)->get_text());

      tft.drawText(10, 140, buf, TftColor::CYAN, 3);
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
