#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"

#include <stdlib.h>

EepromAddrCtrl::EepromAddrCtrl(uint8_t data, uint8_t clk, uint8_t latch)
  : m_data(data), m_clk(clk), m_latch(latch) {
  // Empty
}

void EepromAddrCtrl::init() {
  pinMode(m_data,  OUTPUT);
  pinMode(m_clk,   OUTPUT);
  pinMode(m_latch, OUTPUT);

  digitalWrite(m_data,  LOW);
  digitalWrite(m_clk,   LOW);
  digitalWrite(m_latch, LOW);
}

void EepromAddrCtrl::shift(uint16_t data) {
  shiftOut(m_data, m_clk, MSBFIRST, data >> 8);
  shiftOut(m_data, m_clk, MSBFIRST, data & 0xFF);
}

void EepromAddrCtrl::update() {
  digitalWrite(m_latch, HIGH);
  delayMicroseconds(1);
  digitalWrite(m_latch, LOW);
}

void EepromAddrCtrl::write(uint16_t data, bool oe) {
  shift((data & ~0x80) | (oe ? 0x00 : 0x80));
  update();
}

EepromDataCtrl::EepromDataCtrl(uint8_t pins[8]) {
  memcpy(m_pins, pins, 8);
}

void EepromDataCtrl::init() {
  set_ddr(OUTPUT);
}

void EepromDataCtrl::set_ddr(uint8_t dir) {
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(m_pins[i], dir);
  }
}

uint8_t EepromDataCtrl::read() {
  set_ddr(INPUT);

  uint8_t res = 0;

  for (uint8_t i = 0; i < 7; ++i) {
    res = (res >> 1) | (digitalRead(m_pins[i]) == HIGH ? 0x80 : 0x00);
  }

  return res;
}

void EepromDataCtrl::write(uint8_t data) {
  set_ddr(OUTPUT);

  for (uint8_t i = 0; i < 8; ++i) {
    digitalWrite(m_pins[i], (data >> i) & 1);
  }
}

EepromCtrl::EepromCtrl(uint8_t we, EepromAddrCtrl eac, EepromDataCtrl edc)
  : m_we(we), m_eac(eac), m_edc(edc) {
  // Empty
}

void EepromCtrl::init() {
  m_eac.init();
  m_edc.init();

  pinMode(m_we, OUTPUT);
  digitalWrite(m_we, HIGH);
}

uint8_t EepromCtrl::read(uint16_t addr) {
  m_eac.write(addr, true);
  delayMicroseconds(10);
  uint8_t data = m_edc.read();
  delayMicroseconds(1);
  return data;
}

void EepromCtrl::write(uint16_t addr, uint8_t data) {
  m_eac.write(addr, false);
  m_edc.write(data);

  digitalWrite(m_we, LOW);
  delayMicroseconds(10);
  digitalWrite(m_we, HIGH);
  delay(10);
}
