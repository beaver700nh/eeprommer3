#ifndef COMM_HPP
#define COMM_HPP

#include <Arduino.h>
#include "constants.hpp"

#define PKT_PING     0x00
#define PKT_FILEOPEN 0x10
#define PKT_FILECONF 0x11
#define PKT_FILESIZE 0x12
#define PKT_FILESEEK 0x13
#define PKT_FILEREAD 0x14
#define PKT_FILEWRIT 0x15
#define PKT_FILEFLUS 0x16
#define PKT_FILECLOS 0x17

#define TIMEOUT_PING      200
#define TIMEOUT_FILEREAD 1000

namespace Comm {

struct Packet {
  uint8_t end;
  uint8_t buffer[256];

  static void copy(Packet *dst, Packet *src);
  static void copy_str(Packet *pkt, const char *str);
  static void copy_str_P(Packet *pkt, const char *str);
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
