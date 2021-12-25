#include "constants.hpp"

#include "ad_map.hpp"

AddrDataMapPair::AddrDataMapPair(uint16_t addr, uint8_t data)
  : addr(addr), data(data) {
  // Empty
};

AddrDataMapPair::AddrDataMapPair(const AddrDataMapPair &other)
  : AddrDataMapPair(other.addr, other.data) {
  // Empty
};

AddrDataMap::AddrDataMap() {
  // Empty
};

AddrDataMap::~AddrDataMap() {
  purge();
}

bool AddrDataMap::extend(uint16_t capacity) {
  auto *new_arr = (AddrDataMapPair *) malloc((m_len + capacity) * sizeof(*m_data));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, (m_len + capacity) * sizeof(*m_data));

  free(m_data);

  m_data = new_arr;
  m_len += capacity;

  return true;
}

bool AddrDataMap::append(AddrDataMapPair &pair) {
  if (!extend(1)) return false;

  set_pair(m_len - 1, pair);
  return true;
}

bool AddrDataMap::remove(uint16_t idx) {
  if (idx >= m_len) return false;

  auto new_arr = (AddrDataMapPair *) malloc((m_len - 1) * sizeof(*m_data));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, idx * sizeof(*m_data));
  memcpy(new_arr + idx, m_data + idx + 1, (m_len - idx - 1) * sizeof(*m_data));

  free(m_data);

  m_data = new_arr;
  --m_len;

  return true;
}

bool AddrDataMap::set_pair(uint16_t idx, AddrDataMapPair &pair) {
  if (idx >= m_len) return false;

  m_data[idx] = pair;
  return true;
}

bool AddrDataMap::get_pair(uint16_t idx, AddrDataMapPair *pair) {
  if (idx >= m_len) return false;

  uint32_t temp;
  get_24bit(idx, &temp);

  *pair = AddrDataMapPair(temp >> 8, temp & 0xFF);
  return true;
}

bool AddrDataMap::get_24bit(uint16_t idx, uint32_t *val) {
  if (idx >= m_len) return false;

  *val = (m_data[idx].addr << 8) | (m_data[idx].data);
  return true;
}

bool AddrDataMap::purge(uint8_t idx) {
  if (idx >= m_len) return false;

  AddrDataMapPair *to_del = m_data[idx];

  if (!remove(idx)) return false;
  delete to_del;

  return true;
}

void AddrDataMap::purge() {
  while (m_len > 0) {
    purge_btn(m_len - 1); // Purge last pair in array
  }

  m_len = 0;
}

uint16_t AddrDataMap::get_len() {
  return m_len;
}
