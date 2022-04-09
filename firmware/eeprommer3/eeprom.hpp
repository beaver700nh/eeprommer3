#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <Adafruit_MCP23X17.h>
#include <Adafruit_BusIO_Register.h>

#include "ad_array.hpp"

// A wrapper class around Adafruit_MCP23X17 that has optimized methods
class IoExpCtrl : public Adafruit_MCP23X17 {
public:
  ~IoExpCtrl();

  bool begin(uint8_t addr);

  void set_iodir(uint8_t port, uint8_t mode);

  uint8_t read_port(uint8_t port);
  void write_port(uint8_t port, uint8_t value);

protected:
  Adafruit_BusIO_Register \
    *m_reg_iodir_a, *m_reg_iodir_b, \
    *m_reg_gppu_a,  *m_reg_gppu_b, \
    *m_reg_gpio_a,  *m_reg_gpio_b;
};

// This is a class with functions with low and high level controls of an EEPROM connected
// on two MCP23X17 I2C IO expanders, defaulting to I2C addresses 0x20 and 0x21
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

#ifdef DEBUG_MODE
  IoExpCtrl *get_io_exp(bool which) {
    return &(which ? m_exp_1 : m_exp_0);
  }
#endif

private:
  enum Timing : uint8_t {
    ADDR_SETUP = 1,  // in microseconds
    ADDR_HOLD  = 1,  // in microseconds
    WE_PULSE   = 1,  // in microseconds
    WE_HOLD    = 5,  // in microseconds
    WRITE_TIME = 25, // in milliseconds
  };

  IoExpCtrl m_exp_0, m_exp_1;
};

#endif
