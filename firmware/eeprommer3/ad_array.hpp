#ifndef AD_ARRAY_HPP
#define AD_ARRAY_HPP

/*
 * In order to allow this file and ad_map.cpp to be used and tested outside of the Arduino environment,
 * we use <Arduino.h> with Arduino and normal C++ libraries otherwise.
 */

#include "constants.hpp"

// A key-value pair data structure for storage of addr-data pairs
struct AddrDataArrayPair {
  uint16_t addr;
  uint8_t data;
};

// A data structure to store addr-data pairs
class AddrDataArray {
public:
  AddrDataArray() {};
  virtual ~AddrDataArray();

  bool extend(uint16_t capacity);
  bool append(const AddrDataArrayPair &pair);
  bool remove(uint16_t idx);

  bool set_pair(uint16_t idx, const AddrDataArrayPair &pair);
  bool get_pair(uint16_t idx, AddrDataArrayPair *pair);
  bool get_24bit(uint16_t idx, uint32_t *val);

  void purge();

  uint16_t get_len();

private:
  AddrDataArrayPair *m_data = nullptr;
  uint16_t m_len = 0;
};

#endif
