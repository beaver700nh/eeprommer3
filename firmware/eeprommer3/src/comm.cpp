#include <Arduino.h>
#include "constants.hpp"

#include "comm.hpp"

namespace Comm {

void Packet::copy(Packet *dst, Packet *src) {
  dst->end = src->end;
  memcpy(dst->buffer, src->buffer, src->end);
}

bool blocking_recv(Packet *pkt, uint16_t timeout_ms) {
  unsigned long t1 = millis();
  bool header = true;
  uint8_t index = 0;

  do {
    while (Serial.available() == 0) {
      if (0 < timeout_ms && timeout_ms < (millis() - t1)) {
        SER_LOG_PRINT("Timeout %dms exceeded while reading packet.\n", timeout_ms);
        return false;
      }
    }

    if (header) {
      pkt->end = Serial.read();
      header = false;
    }
    else {
      pkt->buffer[index] = Serial.read();
      ++index;
    }
  }
  while (index != pkt->end);

  return true;
}

bool ping() {
  Packet pkt;
  return blocking_recv(&pkt, PING_TIMEOUT);
}

};
