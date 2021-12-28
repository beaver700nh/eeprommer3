#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <Adafruit_MCP23X17.h>

#include "ad_array.hpp"

class EepromCtrl {
public:
  EepromCtrl() {};

  void init(uint8_t addr_exp_0 = 0x20, uint8_t addr_exp_1 = 0x21);

  void set_addr_and_oe(uint16_t addr_and_oe);

  void set_data(uint8_t data);
  uint8_t get_data();
  void set_ddr(bool dir);

  void set_we(bool we);
  void set_oe(bool oe);

  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data);

  void read(uint16_t addr1, uint16_t addr2, uint8_t *buf);
  void write(uint16_t addr, uint8_t *buf, uint16_t len);
  void write(AddrDataArray *buf);

private:
  Adafruit_MCP23X17 m_exp_0, m_exp_1;
};

#endif
