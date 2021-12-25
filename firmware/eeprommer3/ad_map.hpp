#ifndef AD_MAP_HPP
#define AD_MAP_HPP

// In order to allow this file and ad_map.cpp to be used and tested
// outside of the Arduino environment, we use <Arduino.h> with Arduino
// and normal C++ libraries otherwise.

#ifdef ARDUINO
#  include <Arduino.h>
#else
#  include <cstdint>
#  include <cstdlib>
#  include <cstring>
#endif

#include "constants.hpp"

// A key-value pair data structure for storage of addr-data pairs
struct AddrDataMapPair {
  uint16_t addr;
  uint8_t data;
};

// A map data structure specialized for addr-data pairs
class AddrDataMap {
public:
  AddrDataMap() {};
  virtual ~AddrDataMap();

  bool extend(uint16_t capacity);
  bool append(const AddrDataMapPair &pair);
  bool remove(uint16_t idx);

  bool set_pair(uint16_t idx, const AddrDataMapPair &pair);
  bool get_pair(uint16_t idx, AddrDataMapPair *pair);
  bool get_24bit(uint16_t idx, uint32_t *val);

  void purge();

  uint16_t get_len();

private:
  AddrDataMapPair *m_data = nullptr;
  uint16_t m_len = 0;
};

#endif
