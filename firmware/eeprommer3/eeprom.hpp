#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <Arduino.h>
#include "constants.hpp"

class EepromAddrCtrl {
public:
  EepromAddrCtrl() {};
  EepromAddrCtrl(uint8_t data, uint8_t clk, uint8_t latch);

  void init();

  void shift(uint16_t data);
  void update();

  void write(uint16_t addr, bool oe);

private:
  uint8_t m_data, m_clk, m_latch;
};

class EepromDataCtrl {
public:
  EepromDataCtrl() {};
  EepromDataCtrl(uint8_t pins[8]);

  void init();

  void set_ddr(uint8_t dir);

  uint8_t read();
  void write(uint8_t data);

private:
  uint8_t m_pins[8];
};

class EepromCtrl {
public:
  EepromCtrl() {};
  EepromCtrl(uint8_t we, EepromAddrCtrl eac, EepromDataCtrl edc);

  void init();

  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data);

private:
  uint8_t m_we;

  EepromAddrCtrl m_eac;
  EepromDataCtrl m_edc;
};

#endif
