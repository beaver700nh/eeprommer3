#ifndef COMM_HPP
#define COMM_HPP

#include <Arduino.h>

#include "macros.hpp"

enum class PacketState {
  NONE, ESCAPED,
  DATA_START, DATA, DATA_END,
  CMD_START,  CMD,  CMD_END,
};

enum class PacketMarker {
  DSA = '\x3c', DSB = '\x3c', DEA = '\x3e', DEB = '\x3e',
  CSA = '\x5b', CSB = '\x5b', CEA = '\x5d', CEB = '\x5d',
  ESC = '\x5c'
};

enum class PacketType {
  NONE, WAITING, DATA, CMD
};

typedef struct {
  PacketType type = PacketType::NONE;
  char contents[17];
} Packet;

bool read_packet(Packet *buf);
void copy_packet(Packet *to, Packet *from);

void read_serial_until_end_marker(PacketState state);

#endif
