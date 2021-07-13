#ifndef EEPROM_HPP
#define EEPROM_HPP

#include <Arduino.h>
#include "macros.hpp"

class EepromAddrCtrl {
  
};

class EepromDataCtrl {
  
};

class EepromCtrl {
public:
  EepromCtrl() {};
  EepromCtrl(uint8_t we, EepromAddrCtrl eac, EepromDataCtrl edc);

  void init();

private:
  uint8_t m_we;

  EepromAddrCtrl m_eac;
  EepromDataCtrl m_edc;
};

#endif
