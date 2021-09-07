#include <Arduino.h>
#include "constants.hpp"

#include <IoAbstractionWire.h>
#include <LiquidCrystalIO.h>
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

EepromCtrl ee;
LiquidCrystal lcd(MCP_GPB(4), MCP_GPB(5), MCP_GPB(6), MCP_GPB(0), MCP_GPB(1), MCP_GPB(2), MCP_GPB(3), ioFrom23017(0x21));

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.init(TFT_DRIVER, 1);
  tft.fillScreen(TftColor::BLACK);

  ee.init();

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(" Hello World :) ");
  lcd.setCursor(0, 1);
  lcd.print(" bit.ly/3DCVkbB ");

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

  if (sd.is_enabled()) {
    ProgrammerFromSd prog(ee, sd, tch, tft);
    prog.run();
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
