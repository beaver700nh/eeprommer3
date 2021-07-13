#include <Arduino.h>
#include "constants.hpp"

#define MEGA_SOFT_SPI 1
#include <SD.h>

#include "comm.hpp"
#include "eeprom.hpp"
#include "sd.hpp"
#include "tft.hpp"

TftCtrl tft(A3, A2, A1, A0, A4);
SdCtrl sd(10, A15);

void setup() {
  delay(1000);

  tft.init(0x9341, 3);
  tft.fillScreen(TftColor::BLACK);

  if (!sd.init()) {
    tft.drawText(5, 216, "SD failed/disabled!", TftColor::ORANGE, 2);
  }
  else {
    tft.drawText(5, 216, "SD successful!", TftColor::GREEN, 2);
  }

  Serial.begin(115200);

  Packet this_packet;

  while (true) {
    if (Serial.available() > 0) {
      if (read_packet(&this_packet)) {
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
  }
}

void loop() {}
