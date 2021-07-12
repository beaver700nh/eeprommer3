#include <Elegoo_TFTLCD.h>

#define WHITE   0xFFFF
#define RED     0xF800
#define YELLOW  0xFFE0
#define GREEN   0x07E0
#define CYAN    0x07FF
#define BLUE    0x001F
#define MAGENTA 0xF81F
#define BLACK   0x0000

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

PacketState state = PacketState::NONE;

Elegoo_TFTLCD tft(A3, A2, A1, A0, A4);

Packet bufs[8];

bool read_incoming(Packet *buf);
void scroll_bufs(Packet (*bufs)[8]);
void clear_bufs(Packet (*bufs)[8]);
void update_tft(Packet (*bufs)[8]);

void flush_serial_input();
void flush_serial_input_until_end();

void pckt_copy(Packet *to, Packet *from);

void execute_cmd(Packet *pckt);

void setup() {
  for (uint8_t i = 0; i < 8; ++i) {
    bufs[i].type = PacketType::NONE;
  }

  delay(1000);

  Serial.begin(19200);

  tft.reset();
  delay(1000);
  tft.begin(0x9341);
  tft.setRotation(3);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);

  clear_bufs(&bufs);
  update_tft(&bufs);

  Packet this_pckt;

  while (true) {
    if (Serial.available() > 0) {
      if (read_incoming(&this_pckt)) {
        scroll_bufs(&bufs);
        pckt_copy(&(bufs[7]), &this_pckt);
        update_tft(&bufs);

        if (this_pckt.type == PacketType::CMD) {
          execute_cmd(&this_pckt);
        }
      }
    }
  }
}

bool read_incoming(Packet *buf) {
  static uint16_t idx = 0;
  static PacketType type = PacketType::WAITING;

  if (idx >= 16) {
    state = PacketState::NONE;
    PacketType temp = type;
    type = PacketType::NONE;

    buf->contents[16] = '\0';
    idx = 0;
    buf->type = temp;

    flush_serial_input_until_end();

    return true;
  }

  char ch = Serial.read();

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

  return false;
}

void scroll_bufs(Packet (*bufs)[8]) {
  for (uint8_t i = 1; i < 8; ++i) {
    pckt_copy(&((*bufs)[i - 1]), &((*bufs)[i]));
  }
}

void clear_bufs(Packet (*bufs)[8]) {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);

  for (uint8_t i = 0; i < 8; ++i) {
    sprintf((*bufs)[i].contents, "Buffer #%d.......", i);
    (*bufs)[i].type = PacketType::NONE;
  }
}

void update_tft(Packet (*bufs)[8]) {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);

  for (uint8_t i = 0; i < 8; ++i) {
    uint16_t color = YELLOW;

    switch ((*bufs)[i].type) {
    case PacketType::NONE:    color = MAGENTA; break;
    case PacketType::WAITING: color = RED;     break;
    case PacketType::DATA:    color = CYAN;    break;
    case PacketType::CMD:     color = GREEN;   break;
    }

    tft.setTextColor(color);
    tft.println((*bufs)[i].contents);
  }
}

void flush_serial_input() {
  while (Serial.available() > 0) Serial.read();
}

void flush_serial_input_until_end() {
  PacketState ps = PacketState::DATA;

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
  }
}

void pckt_copy(Packet *to, Packet *from) {
  strcpy(to->contents, from->contents);
  to->type = from->type;
}

void execute_cmd(Packet *pckt) {
  if (pckt->type != PacketType::CMD) return;

  if (strncmp(pckt->contents, "clear", 5) == 0) {
    clear_bufs(&bufs);
    update_tft(&bufs);
  }
  else if (strncmp(pckt->contents, "parrot", 6) == 0) {
    // TODO
    Serial.print("TODO");
  }
}

void loop() {}
