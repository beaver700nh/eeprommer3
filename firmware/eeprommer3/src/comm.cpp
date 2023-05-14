#include <Arduino.h>
#include "constants.hpp"

#include "comm.hpp"
#include "tft.hpp"

extern TftCtrl tft;

namespace Comm {

void Packet::copy(Packet *dst, Packet *src) {
  dst->end = src->end;
  memcpy(dst->buffer, src->buffer, (uint16_t) src->end + 1);
}

void Packet::copy_str(Packet *pkt, const char *str) {
  pkt->end = strlen(str) - 1;
  memcpy(pkt->buffer, str, (uint16_t) pkt->end + 1);
}

void Packet::copy_str_P(Packet *pkt, const char *str) {
  pkt->end = strlen_P(str) - 1;
  memcpy_P(pkt->buffer, str, (uint16_t) pkt->end + 1);
}

void send(Packet *pkt) {
  Serial.write(pkt->end);
  Serial.write(pkt->buffer, (uint16_t) pkt->end + 1);
}

bool recv(Packet *pkt, uint16_t timeout_ms) {
  unsigned long t1 = millis();
  bool header = true;
  uint8_t index = 0;

  while (true) {
    while (Serial.available() == 0) {
      if (0 < timeout_ms && timeout_ms < (millis() - t1)) {
        SER_LOG_PRINT("Timeout %dms exceeded while reading packet.\n", timeout_ms);
        return false;
      }
    }

    if (header) {
      pkt->end = Serial.read();
      tft.drawText(0, 170, STRFMT_NOBUF("val %d", pkt->end), TftColor::CYAN, 1);
      header = false;
    }
    else {
      pkt->buffer[index] = Serial.read();
      tft.drawText(0, 180 + (index * 8), STRFMT_NOBUF("val %d", pkt->buffer[index]), TftColor::GREEN, 1);

      if (index++ == pkt->end) {
        break;
      }
    }
  }

  return true;
}

bool ping() {
  Packet pkt = {0x00, {PKT_PING}};
  send(&pkt);
  return recv(&pkt, PING_TIMEOUT);
}

};
