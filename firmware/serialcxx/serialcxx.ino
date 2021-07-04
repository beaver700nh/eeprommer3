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
  NONE, START, CONTENT, END, ESCAPED
};

enum class PacketMarker {
  SMA = '\x3c', SMB = '\x3c', EMA = '\x3e', EMB = '\x3e', ESC = '\x5c'
};

PacketState state = PacketState::NONE;

Elegoo_TFTLCD tft(A3, A2, A1, A0, A4);

char **bufs;

bool read_incoming(char *buf);
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
      if (read_incoming(temp)) {
        scroll_bufs(&bufs);
        strcpy(bufs[7], temp);
        update_tft(bufs);
      }
    }
  }
}

bool read_incoming(char *buf) {
  static uint16_t idx = 0;

  if (idx >= 16) {
    Serial.println("Length exceeded!");
    state = PacketState::NONE;
    buf[16] = '\0';
    idx = 0;
    flush_serial_input_until_end();
    Serial.println("Done flushing!");

    return true;
  }

  char data = Serial.read();

  if (state == PacketState::NONE) {
    if (data == (char) PacketMarker::SMA) {
      state = PacketState::START;
    }
  }
  else if (state == PacketState::START) {
    if (data == (char) PacketMarker::SMB) {
      state = PacketState::CONTENT;
    }
    else {
      state = PacketState::NONE;
    }
  }
  else if (state == PacketState::CONTENT) {
    if (data == (char) PacketMarker::ESC) {
      state = PacketState::ESCAPED;
    }
    else if (data == (char) PacketMarker::EMA) {
      state = PacketState::END;
    }
    else {
      buf[idx++] = data;
    }
  }
  else if (state == PacketState::ESCAPED) {
    state = PacketState::CONTENT;
    buf[idx++] = data;
  }
  else if (state == PacketState::END) {
    if (data == (char) PacketMarker::EMB) {
      state = PacketState::NONE;

      buf[idx] = '\0';
      idx = 0;

      return true;
    }
    else {
      state = PacketState::CONTENT;
      buf[idx++] = (char) PacketMarker::EMA;

      if (data == (char) PacketMarker::ESC) {
        while (Serial.available() <= 0); /* wait for a char */
        buf[idx++] = Serial.read();
      }
      else {
        buf[idx++] = data;
      }
    }
  }

  return false;
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
  PacketState ps = PacketState::CONTENT;

  while (ps != PacketState::NONE) {
    while (Serial.available() <= 0); /* wait for character */

    char c = Serial.read();

    if (ps == PacketState::CONTENT) {
      if (c == (char) PacketMarker::EMA) {
        ps = PacketState::END;
      }
    }
    else if (ps == PacketState::END) {
      if (c == (char) PacketMarker::EMB) {
        ps = PacketState::NONE;
      }
      else {
        ps = PacketState::CONTENT;
      }
    }
  }
}

void loop() {}
