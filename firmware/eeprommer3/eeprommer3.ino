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

TftCtrl tft(A3, A2, A1, A0, A4);
SdCtrl sd(10, A15);
TouchScreen ts(8, A3, A2, 9, 300);

bool using_sd = false;

void setup() {
  delay(1000);

  Serial.begin(115200);

  tft.init(0x9341, 3);
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
    constexpr uint8_t num_btns = 5;

    TftBtn btns[num_btns] = {
      TftBtn(120, 0,   80, 20, "Hello1!"),
      TftBtn(120, 25,  80, 20, "Hello2!"),
      TftBtn(120, 50,  80, 20, "Hello3!"),
      TftBtn(120, 75,  80, 20, "Hello4!"),
      TftBtn(120, 100, 80, 20, "Hello5!"),
    };

    while (true) {
      bool got_press = false;

      for (uint8_t i = 0; i < num_btns; ++i) {
        btns[i].draw(tft);
      }

      while (!got_press) {
        for (uint8_t i = 0; i < num_btns; ++i) {
          if (btns[i].is_pressed(ts)) {
            serial_print_point(btns[i].most_recent_press);
            got_press = true;
            break;
          }
        }
      }

      tft.fillScreen(TftColor::MAGENTA);

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
