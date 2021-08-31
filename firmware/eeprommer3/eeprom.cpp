#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"

void EepromCtrl::init(uint8_t addr_exp_0, uint8_t addr_exp_1) {
  m_exp_0.begin_I2C(addr_exp_0);
  m_exp_1.begin_I2C(addr_exp_1);

  m_exp_1.pinMode(MCP_EE_WE, OUTPUT);
  set_we(true);

  for (uint8_t i = 0; i < 16; ++i) {
    m_exp_0.pinMode(i, OUTPUT);
  }
}

void EepromCtrl::set_addr_and_oe(uint16_t addr_and_oe) {
  m_exp_0.writeGPIO( addr_and_oe       & 0xFF, MCP_EE_ADDRL_PORT);
  m_exp_0.writeGPIO((addr_and_oe >> 8) & 0xFF, MCP_EE_ADDRH_PORT);
}

void EepromCtrl::set_data(uint8_t data) {
  set_ddr(true);
  m_exp_1.writeGPIO(data, MCP_EE_DATA_PORT);
}

uint8_t EepromCtrl::get_data() {
  set_ddr(false);
  return m_exp_1.readGPIO(MCP_EE_DATA_PORT);
}

void EepromCtrl::set_ddr(bool dir) {
  uint8_t _dir = (dir ? OUTPUT : INPUT);

  for (uint8_t i = 0; i < 8; ++i) {
    m_exp_1.pinMode(MCP_EE_DATA(i), _dir);
  }
}

void EepromCtrl::set_we(bool we) {
  m_exp_1.digitalWrite(MCP_EE_WE, (we ? HIGH : LOW));
}

void EepromCtrl::set_oe(bool oe) {
  m_exp_0.digitalWrite(MCP_EE_OE, (oe ? HIGH : LOW));
}

uint8_t EepromCtrl::read(uint16_t addr) {
  set_addr_and_oe(addr | 0x8000);
  delayMicroseconds(10);
  uint8_t data = get_data();
  delayMicroseconds(1);
  return data;
}

void EepromCtrl::write(uint16_t addr, uint8_t data) {
  set_addr_and_oe(addr & ~0x8000);
  set_data(data);

  set_we(false);
  delayMicroseconds(10);
  set_we(true);
  delay(10);
}
