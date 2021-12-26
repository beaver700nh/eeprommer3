#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"

#include "ad_map.hpp"

void EepromCtrl::init(uint8_t addr_exp_0, uint8_t addr_exp_1) {
  m_exp_0.begin_I2C(addr_exp_0);
  m_exp_1.begin_I2C(addr_exp_1);

  m_exp_1.pinMode(MCP_EE_WE, OUTPUT);
  set_we(true);

  // Set address pins + OE to outputs
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
  set_we(true);
  set_addr_and_oe(addr & ~0x8000); // ~OE is off to enable output
  delayMicroseconds(10);
  uint8_t data = get_data();
  delayMicroseconds(1);
  return data;
}

void EepromCtrl::write(uint16_t addr, uint8_t data) {
  set_addr_and_oe(addr | 0x8000); // ~OE is on to disable output
  set_data(data);

  set_we(false);
  delayMicroseconds(10);
  set_we(true);
  delay(10);
}

uint8_t *EepromCtrl::read(uint16_t addr1, uint16_t addr2) {
  uint8_t *data = (uint8_t *) malloc((addr2 - addr1 + 1) * sizeof(*data));

  for (uint16_t i = addr1; /* condition is in loop body */; ++i) {
    data[i - addr1] = read(i);

    if (i == addr2) break;
  }

  return data;
}

void EepromCtrl::write(AddrDataMap *buf) {
  if (buf->get_len() < 1) return;

  AddrDataMapPair pair;

  uint16_t addr0;

  {
    uint32_t temp;
    buf->get_24bit(0, &temp);
    addr0 = temp >> 8;
  }

  uint16_t i = 0;

  while (buf->get_pair(i++, &pair) == true) {
    // Skip pairs that are not in the same page as pair #0
    if ((0x7FC0 & ~(pair.addr ^ addr0)) != 0x7FC0) continue;

    set_addr_and_oe(pair.addr | 0x8000); // ~OE is on to disable output
    set_data(pair.data);

    set_we(false);
    delayMicroseconds(10);
    set_we(true);
    delayMicroseconds(2);
  }

  // Delay to be sure that the next operation is treated
  // as a different one than this operation
  delay(10);
}
