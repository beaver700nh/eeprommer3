#include <Elegoo_TFTLCD.h>

#include "macros.hpp"

#include "comm.hpp"

#define WHITE   0xFFFF
#define RED     0xF800
#define YELLOW  0xFFE0
#define GREEN   0x07E0
#define CYAN    0x07FF
#define BLUE    0x001F
#define MAGENTA 0xF81F
#define BLACK   0x0000

Elegoo_TFTLCD tft(A3, A2, A1, A0, A4);

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.reset();
  delay(1000);
  tft.begin(0x9341);
  tft.setRotation(3);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);

  Packet this_packet;

  while (true) {
    if (Serial.available() > 0) {
      if (read_packet(&this_packet)) {
        uint16_t color = RED;

        switch (this_packet.type) {
        case PacketType::NONE:    color = MAGENTA; break;
        case PacketType::WAITING: color = YELLOW;  break;
        case PacketType::DATA:    color = CYAN;    break;
        case PacketType::CMD:     color = GREEN;   break;
        }

        tft.fillScreen(BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(color);
        tft.println(this_packet.contents);
      }
    }
  }
}

void loop() {}
