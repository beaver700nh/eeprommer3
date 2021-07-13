#include <Arduino.h>
#include "macros.hpp"

#include "comm.hpp"
#include "eeprom.hpp"
#include "tft.hpp"

TftCtrl tft(A3, A2, A1, A0, A4);

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.init(0x9341, 3, TftColor::WHITE, 3);

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
