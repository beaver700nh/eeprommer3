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

PacketState state = PacketState::NONE;

Elegoo_TFTLCD tft(A3, A2, A1, A0, A4);

char **bufs;

PacketType read_incoming(char *buf);
void scroll_bufs(char ***bufs);
void update_tft(char **bufs);

void flush_serial_input();
void flush_serial_input_until_end();

void setup() {
  bufs = (char **) malloc(sizeof(char *) * 8);

  for (uint8_t i = 0; i < 8; ++i) {
    bufs[i] = (char *) malloc(sizeof(char) * (16 + 1));
  }

  delay(1000);

  Serial.begin(19200);

  tft.reset();
  delay(1000);
  tft.begin(0x9341);
  tft.setRotation(3);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);

  for (uint8_t i = 0; i < 8; ++i) {
    sprintf(bufs[i], "Buffer #%d.......", i);
  }

  update_tft(bufs);

  char temp[17];

  while (true) {
    if (Serial.available() > 0) {
      PacketType result = read_incoming(temp);

      if (result == PacketType::DATA) {
        scroll_bufs(&bufs);
        strcpy(bufs[7], temp);
        update_tft(bufs);
      }
      else if (result == PacketType::CMD) {
        scroll_bufs(&bufs);

        char temp2[22];
        sprintf(temp2, "Cmd: %s", temp);
        strcpy(bufs[7], temp2);

        update_tft(bufs);
      }
    }
  }
}

PacketType read_incoming(char *buf) {
  static uint16_t idx = 0;
  static PacketType type = PacketType::WAITING;

  if (idx >= 16) {
    state = PacketState::NONE;
    PacketType temp = type;
    type = PacketType::NONE;

    buf[16] = '\0';
    idx = 0;

    flush_serial_input_until_end();

    return temp;
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
      buf[idx++] = ch;
    }
  }
  else if (state == PacketState::ESCAPED) {
    state = (type == PacketType::DATA ? PacketState::DATA : PacketState::CMD);
    buf[idx++] = ch;
  }
  else if (state == PacketState::DATA_END) {
    if (ch == (char) PacketMarker::DEB) {
      state = PacketState::NONE;
      type = PacketType::NONE;

      buf[idx] = '\0';
      idx = 0;

      return PacketType::DATA;
    }
    else {
      state = PacketState::DATA;
      buf[idx++] = (char) PacketMarker::DEA;

      if (ch == (char) PacketMarker::ESC) {
        state = PacketState::ESCAPED;
      }
      else {
        buf[idx++] = ch;
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
      buf[idx++] = ch;
    }
  }
  else if (state == PacketState::ESCAPED) {
    state = PacketState::CMD;
    buf[idx++] = ch;
  }
  else if (state == PacketState::CMD_END) {
    if (ch == (char) PacketMarker::CEB) {
      state = PacketState::NONE;
      type = PacketType::NONE;

      buf[idx] = '\0';
      idx = 0;

      return PacketType::CMD;
    }
    else {
      state = PacketState::CMD;
      buf[idx++] = (char) PacketMarker::CEA;

      if (ch == (char) PacketMarker::ESC) {
        state = PacketState::ESCAPED;
      }
      else {
        buf[idx++] = ch;
      }
    }
  }

  return PacketType::WAITING;
}

void scroll_bufs(char ***bufs) {
  for (uint8_t i = 1; i < 8; ++i) {
    strcpy((*bufs)[i - 1], (*bufs)[i]);
  }
}

void update_tft(char **bufs) {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);

  for (uint8_t i = 0; i < 8; ++i) {
    tft.println(bufs[i]);
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

void loop() {}
