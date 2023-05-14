#ifndef COMM_HPP
#define COMM_HPP

#include <Arduino.h>
#include "constants.hpp"

#define PKT_PING 0x00

#define PING_TIMEOUT 200

namespace Comm {

struct Packet {
  uint8_t end;
  uint8_t buffer[256];

  static void copy(Packet *dst, Packet *src);
};

// Sends a packet over Serial.
void send(Packet *pkt);

// Blocks while reading a packet from Serial.
// Returns true if packet was read successfully, false if timeout occurred.
// Set `timeout_ms` to 0 to disable timeout.
bool recv(Packet *pkt, uint16_t timeout_ms = 0);

// Pings connected computer over Serial, returns whether a response was recieved.
bool ping();

};

#endif
