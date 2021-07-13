#include <Arduino.h>
#include "constants.hpp"

#include "comm.hpp"

PacketState state = PacketState::NONE;

bool read_packet(Packet *buf) {
  static uint16_t idx = 0;
  static PacketType type = PacketType::WAITING;

  char ch = Serial.read();

#ifdef DEBUG_MODE
  char str[100];
  sprintf(
    str,
    "read_packet():\n"
    "\tstate:\t%d\n"
    "\ttype: \t%d\n"
    "\tchar: \t%c\n",
    (uint8_t) state,
    (uint8_t) type,
    (char)    ch
  );

  Serial.print(str);
#endif

  if (state == PacketState::NONE) {
    if (ch == (char) PacketMarker::DSA) {
      state = PacketState::DATA_START;
    }
    else if (ch == (char) PacketMarker::CSA) {
      state = PacketState::CMD_START;
    }
  }
  else if (state == PacketState::DATA_START) {
    if (ch == (char) PacketMarker::DSB) {
      state = PacketState::DATA;
      type = PacketType::DATA;
    }
    else {
      state = PacketState::NONE;
    }
  }
  else if (state == PacketState::DATA) {
    if (ch == (char) PacketMarker::ESC) {
      state = PacketState::ESCAPED;
    }
    else if (ch == (char) PacketMarker::DEA) {
      state = PacketState::DATA_END;
    }
    else {
      buf->contents[idx++] = ch;
    }
  }
  else if (state == PacketState::ESCAPED) {
    state = (type == PacketType::DATA ? PacketState::DATA : PacketState::CMD);
    buf->contents[idx++] = ch;
  }
  else if (state == PacketState::DATA_END) {
    if (ch == (char) PacketMarker::DEB) {
      state = PacketState::NONE;
      type = PacketType::NONE;

      buf->contents[idx] = '\0';
      idx = 0;
      buf->type = PacketType::DATA;

      return true;
    }
    else {
      state = PacketState::DATA;
      buf->contents[idx++] = (char) PacketMarker::DEA;

      if (ch == (char) PacketMarker::ESC) {
        state = PacketState::ESCAPED;
      }
      else {
        buf->contents[idx++] = ch;
      }
    }
  }
  else if (state == PacketState::CMD_START) {
    if (ch == (char) PacketMarker::CSB) {
      state = PacketState::CMD;
      type = PacketType::CMD;
    }
    else {
      state = PacketState::NONE;
    }
  }
  else if (state == PacketState::CMD) {
    if (ch == (char) PacketMarker::ESC) {
      state = PacketState::ESCAPED;
    }
    else if (ch == (char) PacketMarker::CEA) {
      state = PacketState::CMD_END;
    }
    else {
      buf->contents[idx++] = ch;
    }
  }
  else if (state == PacketState::ESCAPED) {
    state = PacketState::CMD;
    buf->contents[idx++] = ch;
  }
  else if (state == PacketState::CMD_END) {
    if (ch == (char) PacketMarker::CEB) {
      state = PacketState::NONE;
      type = PacketType::NONE;

      buf->contents[idx] = '\0';
      idx = 0;
      buf->type = PacketType::CMD;

      return true;
    }
    else {
      state = PacketState::CMD;
      buf->contents[idx++] = (char) PacketMarker::CEA;

      if (ch == (char) PacketMarker::ESC) {
        state = PacketState::ESCAPED;
      }
      else {
        buf->contents[idx++] = ch;
      }
    }
  }

  if (idx >= 16) {
    buf->contents[16] = '\0';
    idx = 0;
    buf->type = type;

    read_serial_until_end_marker(state);

    state = PacketState::NONE;
    type = PacketType::NONE;

    return true;
  }

  return false;
}

void copy_packet(Packet *to, Packet *from) {
  strcpy(to->contents, from->contents);
  to->type = from->type;
}

void read_serial_until_end_marker(PacketState state) {
  PacketState ps = state;

  while (ps != PacketState::NONE) {
    while (Serial.available() <= 0); /* wait for character */

    char c = Serial.read();

    if (ps == PacketState::DATA) {
      if (c == (char) PacketMarker::DEA) {
        ps = PacketState::DATA_END;
      }
    }
    else if (ps == PacketState::DATA_END) {
      if (c == (char) PacketMarker::DEB) {
        ps = PacketState::NONE;
      }
      else {
        ps = PacketState::DATA;
      }
    }
    else if (ps == PacketState::CMD) {
      if (c == (char) PacketMarker::CEA) {
        ps = PacketState::CMD_END;
      }
    }
    else if (ps == PacketState::CMD_END) {
      if (c == (char) PacketMarker::CEB) {
        ps = PacketState::NONE;
      }
      else {
        ps = PacketState::CMD;
      }
    }
  }
}
