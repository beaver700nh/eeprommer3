#include <Arduino.h>
#include "constants.hpp"

#include <util/delay.h>

#include <Wire.h>

#include "ad_array.hpp"
#include "new_delete.hpp"
#include "util.hpp"

#include "eeprom.hpp"

void EepromCtrl::init(uint8_t addr_exp_0, uint8_t addr_exp_1) {
  m_exp_0.init(addr_exp_0);
  m_exp_1.init(addr_exp_1);

  m_exp_1.set_iodir(PORT_B, OUTPUT);

  set_we(true);

  m_exp_0.set_iodir(PORT_A, OUTPUT);
  m_exp_0.set_iodir(PORT_B, OUTPUT);
}

void EepromCtrl::set_addr_and_oe(uint16_t addr_and_oe) {
  m_exp_0.write_port(PORT_A,  addr_and_oe       & 0xFF);
  m_exp_0.write_port(PORT_B, (addr_and_oe >> 8) & 0xFF);
}

void EepromCtrl::set_data(uint8_t data) {
  set_ddr(true);
  m_exp_1.write_port(PORT_A, data);
}

uint8_t EepromCtrl::get_data() {
  set_ddr(false);
  return m_exp_1.read_port(PORT_A);
}

void EepromCtrl::set_ddr(bool dir) {
  uint8_t _dir = (dir ? OUTPUT : INPUT_PULLUP);
  m_exp_1.set_iodir(PORT_A, _dir);
}

void EepromCtrl::set_we(bool we) {
  m_exp_1.write_port(PORT_B, (we ? 0xFF : 0x00));
}

void EepromCtrl::set_oe(bool oe) {
  m_exp_0.write_bit(PORT_B, 7, (oe ? HIGH : LOW));
}

uint8_t EepromCtrl::read(uint16_t addr) {
  set_we(true);
  set_addr_and_oe(addr & ~0x8000);  // ~OE is off to enable output

  _delay_us(Timing::ADDR_SETUP);
  uint8_t data = get_data();
  _delay_us(Timing::ADDR_HOLD);

  return data;
}

void EepromCtrl::write(uint16_t addr, uint8_t data, bool quick) {
  set_addr_and_oe(addr | 0x8000);  // ~OE is on to disable output
  set_data(data);

  set_we(false);
  _delay_us(Timing::WE_PULSE);
  set_we(true);
  _delay_us(Timing::WE_HOLD);

  if (!quick) {
    _delay_ms(Timing::WRITE_TIME);
  }
}

void EepromCtrl::read(uint16_t addr1, uint16_t addr2, uint8_t *buf) {
  uint16_t i = addr1;

  do {
    buf[i - addr1] = read(i);
  }
  while (i++ != addr2);
}

void EepromCtrl::write(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint16_t i = addr;

  set_addr_and_oe(0x8000);  // ~OE is on to disable output

  do {
    write(i, buf[i - addr], true);
  }
  while ((i - addr + 1) < len && ++i);

  // Delay to be sure that the next operation is treated as a different operation than this one
  _delay_ms(Timing::WRITE_TIME);
}

void EepromCtrl::write(AddrDataArray *buf) {
  if (buf->get_len() < 1) return;

  AddrDataArrayPair pair;

  uint16_t i = 0;

  while (buf->get_pair(i++, &pair) == true) {
    write(pair.addr, pair.data, true);
  }

  // Delay to be sure that the next operation is treated as a different operation than this one
  _delay_ms(Timing::WRITE_TIME);
}

void IoExpCtrl::init(uint8_t addr) {
  m_addr = addr;
}

void IoExpCtrl::set_iodir(uint8_t port, uint8_t mode) {
  Wire.beginTransmission(m_addr);
  Wire.write(Regs::IODIR | port);
  Wire.write((mode == OUTPUT) ? 0x00 : 0xFF);
  Wire.endTransmission();

  Wire.beginTransmission(m_addr);
  Wire.write(Regs::GPPU | port);
  Wire.write((mode == INPUT_PULLUP) ? 0xFF : 0x00);
  Wire.endTransmission();
}

uint8_t IoExpCtrl::read_port(uint8_t port) {
  Wire.beginTransmission(m_addr);
  Wire.write(Regs::GPIO | port);
  Wire.endTransmission();

  Wire.requestFrom(m_addr, 1U);

  return Wire.read();
}

void IoExpCtrl::write_port(uint8_t port, uint8_t value) {
  Memory::print_ram_analysis();

  Wire.beginTransmission(m_addr);
  Wire.write(Regs::GPIO | port);
  Wire.write(value);
  Wire.endTransmission();
}

bool IoExpCtrl::read_bit(uint8_t port, uint8_t which) {
  return read_port(port) & (1 << which);
}

void IoExpCtrl::write_bit(uint8_t port, uint8_t which, bool value) {
  uint8_t temp = (read_port(port) & ~(1 << which)) | (value << which);
  write_port(port, temp);
}
