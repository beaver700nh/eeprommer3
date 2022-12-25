#include <Arduino.h>

#define LE 39

#define DATA(x) PORTA = x
#define ADDR(x) PORTC = x

void data(uint8_t x) {
  DATA(x);
  Serial.print("Set D0-D7 to ");
  Serial.println(x, HEX);
}

void addh(uint8_t x) {
  ADDR(x);
  Serial.print("Set A8-A15 to ");
  Serial.println(x, HEX);
}

void addl(uint8_t x) {
  DATA(x);
  digitalWrite(LE, HIGH);
  delayMicroseconds(1);
  digitalWrite(LE, LOW);
  Serial.print("Set A0-A7 to ");
  Serial.println(x, HEX);
}

void (*ops[])(uint8_t) {data, addh, addl};

void read_command(char *buf) {
  uint8_t i = 0;
  char c = '\0';

  while (c != '\r') {
    while (!Serial.available());

    c = Serial.read();
    Serial.print(c);

    buf[i++] = c;
  }

  Serial.println();

  while (Serial.available()) {
    Serial.read();
  }
}

uint8_t fromhex(char c) {
  return c <= '9' ? c - '0' : c - 'A' + 10;
}

uint8_t fromhex(char *s) {
  return 16 * fromhex(s[0]) + fromhex(s[1]);
}

void setup() {
  Serial.begin(115200);

  DDRA = 0xFF;
  DDRC = 0xFF;

  pinMode(LE, OUTPUT);

  digitalWrite(LE, LOW);

  delay(5000);
}

void loop() {
  Serial.print("> ");
  char buf[4];
  read_command(buf);
  buf[3] = '\0';

  ops[buf[0] - '0'](fromhex(buf + 1));
}
