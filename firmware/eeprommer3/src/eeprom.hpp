#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "ad_array.hpp"

#define PORT_A 0
#define PORT_B 1

// A class to interface with MCP23017 I/O expanders
class IoExpCtrl {
public:
  IoExpCtrl() {};

  void init(uint8_t addr);

  void set_iodir(uint8_t port, uint8_t mode);

  uint8_t read_port(uint8_t port);
  void write_port(uint8_t port, uint8_t value);

  bool read_bit(uint8_t port, uint8_t which);
  void write_bit(uint8_t port, uint8_t which, bool value);

  enum Regs : uint8_t {
    IODIR   = 0x00,
    IPOL    = 0x02,
    GPINTEN = 0x04,
    DEFVAL  = 0x06,
    INTCON  = 0x08,
    IOCON   = 0x0A,
    GPPU    = 0x0C,
    INTF    = 0x0E,
    INTCAP  = 0x10,
    GPIO    = 0x12,
    OLAT    = 0x14,
  };

private:
  uint8_t m_addr;
};

// This is a class with functions with low and high level controls of an EEPROM connected
// on two MCP23X17 I2C IO expanders, defaulting to I2C addresses 0x20 and 0x21
class EepromCtrl {
public:
  void init(uint8_t addr_exp_0 = 0x20, uint8_t addr_exp_1 = 0x21);

  void set_addr_and_oe(uint16_t addr_and_oe);

  void set_data(uint8_t data);
  uint8_t get_data();
  void set_ddr(bool dir);

  void set_we(bool we);
  void set_oe(bool oe);

  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t data, bool quick = false);

  void read(uint16_t addr1, uint16_t addr2, uint8_t *buf);
  void write(uint16_t addr, uint8_t *buf, uint16_t len);
  void write(AddrDataArray *buf);

#ifdef DEBUG_MODE
  IoExpCtrl *get_io_exp(bool which) {
    return &(which ? m_exp_1 : m_exp_0);
  }
#endif

private:
  enum Timing : uint8_t {
    ADDR_SETUP = 0,   // in microseconds (must be more than 15ns)
    ADDR_HOLD  = 1,   // in microseconds (actually 50ns)
    WE_PULSE   = 1,   // in microseconds (actually 100ns)
    WE_HOLD    = 1,   // in microseconds (actually 50ns)
    WRITE_TIME = 11,  // in milliseconds (actually 10ms)
  };

  IoExpCtrl m_exp_0, m_exp_1;
};

#endif
