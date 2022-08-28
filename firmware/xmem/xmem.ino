#include "HardwareSerial.h"
#include <Arduino.h>

static auto *extmem = (volatile uint8_t *) 0x8000;

uint16_t test() {
  uint16_t wrong = 0;

  for (uint16_t i = 0; i < 0x8000; ++i) {
    extmem[i] = (i & 0xFF);
  }

  for (uint16_t i = 0; i < 0x8000; ++i) {
    if (extmem[i] != (i & 0xFF)) {
      ++wrong;
    }
  }

  return wrong;
}

void setup() {
  auto run = [&]() {
    unsigned long t1, t2;

    t1 = millis();
    uint16_t right = 0x8000 - test();
    t2 = millis();

    char buf[128];
    sprintf(
      buf, "Test finished in %lu milliseconds with %u/32768 successful verifications (%lu%%).",
      t2 - t1, right, lround((float) right / 327.68)
    );

    Serial.println(buf);
  };

  Serial.begin(115200);

  Serial.println("Enabling XMEM...");

  XMCRA |= (1 << SRE);
  XMCRB |= (1 << XMM0);

  Serial.println("1. Wait cycles: 2 + 1");

  XMCRA |= (1 << SRW11);
  XMCRA |= (1 << SRW10);
  run();

  Serial.println("2. Wait cycles: 2");

  XMCRA |=  (1 << SRW11);
  XMCRA &= ~(1 << SRW10);
  run();

  Serial.println("3. Wait cycles: 1");

  XMCRA &= ~(1 << SRW11);
  XMCRA |=  (1 << SRW10);
  run();

  Serial.println("4. Wait cycles: 0");
  XMCRA &= ~(1 << SRW11);
  XMCRA &= ~(1 << SRW10);
  run();

  Serial.println("Done!");
}

void loop() {
  // All work completed in setup()
}
