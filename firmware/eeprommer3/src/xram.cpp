#include <Arduino.h>
#include "constants.hpp"

#include "xram.hpp"

static bool initialized = false;

void xram::init(uint8_t waits, uint8_t portc_mask) {
  // Enable SRAM
  XMCRA |= (1 << SRE);

  // Set 2 bits at SRW10 to `waits`
  XMCRA &= ~(0x03 << SRW10);
  XMCRA |= ((waits & 0x03) << SRW10);

  // Set XMMn bits to `portc_mask` (zero all else)
  XMCRB = (portc_mask & 0x07);

  // Mark as initialized
  initialized = true;
}

bool xram::is_initialized() {
  return initialized;
}

volatile uint8_t *xram::access(uint16_t addr) {
  return initialized ? reinterpret_cast<volatile uint8_t *>(addr) : nullptr;
}

xram::TestResults xram::test() {
  auto buf = access(0x8000);
  TestResults res {0, millis()};

  constexpr const auto seed_pin = A14;

  pinMode(seed_pin, INPUT);
  auto seed = analogRead(seed_pin);

  srand(seed);

  for (uint16_t i = 0x0000; i < 0x8000; ++i) {
    buf[i] = (rand() & 0xFF);
  }

  srand(seed);

  for (uint16_t i = 0x0000; i < 0x8000; ++i) {
    if (buf[i] == (rand() & 0xFF)) {
      ++(res.successes);
    }
  }

  res.time = millis() - res.time;
  return res;
}
