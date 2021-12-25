#ifndef AD_MAP_HPP
#define AD_MAP_HPP

#include "constants.hpp"

// A key-value pair data structure for storage of addr-data pairs
struct AddrDataMapPair {
  AddrDataMapPair(uint16_t addr, uint8_t data);
  AddrDataMapPair(const AddrDataMapPair &other);

  uint16_t addr;
  uint8_t data;
};

// A map data structure specialized for addr-data pairs
class AddrDataMap {
public:
  AddrDataMap();
  ~AddrDataMap();

  bool extend(uint16_t capacity);
  bool append(AddrDataMapPair &pair);
  bool remove(uint16_t idx);

  bool set_pair(uint16_t idx, AddrDataMapPair &pair);
  bool get_pair(uint16_t idx, AddrDataMapPair *pair);
  bool get_24bit(uint16_t idx, uint32_t *val);

  bool purge(uint16_t idx);
  void purge();

  uint16_t get_len();

private:
  AddrDataMapPair *m_data;
  uint16_t m_len = 0;
};

#endif
