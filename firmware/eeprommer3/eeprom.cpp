#include <Arduino.h>
#include "constants.hpp"

#include <util/delay.h>

#include <Adafruit_MCP23X17.h>
#include <Adafruit_BusIO_Register.h>

#include "new_delete.hpp"
#include "ad_array.hpp"

#include "eeprom.hpp"

void EepromCtrl::init(uint8_t addr_exp_0, uint8_t addr_exp_1) {
  m_exp_0.begin(addr_exp_0);
  m_exp_1.begin(addr_exp_1);

  m_exp_1.set_iodir(MCP_EE_WE_PORT, OUTPUT);
  set_we(true);

  m_exp_0.set_iodir(MCP_EE_ADDRL_PORT, OUTPUT);
  m_exp_0.set_iodir(MCP_EE_ADDRH_PORT, OUTPUT);
}

void EepromCtrl::set_addr_and_oe(uint16_t addr_and_oe) {
  m_exp_0.write_port(MCP_EE_ADDRL_PORT,  addr_and_oe       & 0xFF);
  m_exp_0.write_port(MCP_EE_ADDRH_PORT, (addr_and_oe >> 8) & 0xFF);
}

void EepromCtrl::set_data(uint8_t data) {
  set_ddr(true);
  m_exp_1.write_port(MCP_EE_DATA_PORT, data);
}

uint8_t EepromCtrl::get_data() {
  set_ddr(false);
  return m_exp_1.read_port(MCP_EE_DATA_PORT);
}

void EepromCtrl::set_ddr(bool dir) {
  uint8_t _dir = (dir ? OUTPUT : INPUT_PULLUP);
  m_exp_1.set_iodir(MCP_EE_DATA_PORT, _dir);
}

void EepromCtrl::set_we(bool we) {
  m_exp_1.write_port(MCP_EE_WE_PORT, (we ? 0xFF : 0x00));
}

void EepromCtrl::set_oe(bool oe) {
  m_exp_0.digitalWrite(MCP_EE_OE, (oe ? HIGH : LOW));
}

uint8_t EepromCtrl::read(uint16_t addr) {
  set_we(true);
  set_addr_and_oe(addr & ~0x8000); // ~OE is off to enable output

  _delay_us(Timing::ADDR_SETUP);
  uint8_t data = get_data();
  _delay_us(Timing::ADDR_HOLD);

  return data;
}

void EepromCtrl::write(uint16_t addr, uint8_t data, bool quick) {
  set_addr_and_oe(addr | 0x8000); // ~OE is on to disable output
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

  set_addr_and_oe(0x8000); // ~OE is on to disable output

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

IoExpCtrl::~IoExpCtrl() {
  delete m_reg_iodir_a;
  delete m_reg_iodir_b;
  delete m_reg_gppu_a;
  delete m_reg_gppu_b;
  delete m_reg_gpio_a;
  delete m_reg_gpio_b;
}

bool IoExpCtrl::begin(uint8_t addr) {
  bool status = begin_I2C(addr);

  m_reg_iodir_a = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_IODIR, 0));
  m_reg_iodir_b = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_IODIR, 1));
  m_reg_gppu_a  = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_GPPU,  0));
  m_reg_gppu_b  = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_GPPU,  1));
  m_reg_gpio_a  = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_GPIO,  0));
  m_reg_gpio_b  = new Adafruit_BusIO_Register(i2c_dev, getRegister(MCP23XXX_GPIO,  1));

  return status;
}

void IoExpCtrl::set_iodir(uint8_t port, uint8_t mode) {
  (port == 0 ? m_reg_iodir_a : m_reg_iodir_b)->write((mode == OUTPUT      ) ? 0x00 : 0xFF);
  (port == 0 ? m_reg_gppu_a  : m_reg_gppu_b )->write((mode == INPUT_PULLUP) ? 0xFF : 0x00);
}

uint8_t IoExpCtrl::read_port(uint8_t port) {
  return (port == 0 ? m_reg_gpio_a : m_reg_gpio_b)->read() & 0xFF;
}

void IoExpCtrl::write_port(uint8_t port, uint8_t value) {
  (port == 0 ? m_reg_gpio_a : m_reg_gpio_b)->write(value);
}
