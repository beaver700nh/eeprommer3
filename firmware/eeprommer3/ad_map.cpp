#include "constants.hpp"

#include "ad_map.hpp"

AddrDataMap::~AddrDataMap() {
  purge();
}

bool AddrDataMap::extend(uint16_t capacity) {
  auto *new_arr = (AddrDataMapPair *) malloc((m_len + capacity) * sizeof(AddrDataMapPair));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, m_len * sizeof(AddrDataMapPair));

  free(m_data);

  m_data = new_arr;
  m_len += capacity;

  return true;
}

bool AddrDataMap::append(const AddrDataMapPair &pair) {
  if (!extend(1)) return false;

  set_pair(m_len - 1, pair);
  return true;
}

bool AddrDataMap::remove(uint16_t idx) {
  if (idx >= m_len) return false;

  auto new_arr = (AddrDataMapPair *) malloc((m_len - 1) * sizeof(AddrDataMapPair));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, idx * sizeof(AddrDataMapPair));
  memcpy(new_arr + idx, m_data + idx + 1, (m_len - idx - 1) * sizeof(AddrDataMapPair));

  free(m_data);

  m_data = new_arr;
  --m_len;

  return true;
}

bool AddrDataMap::set_pair(uint16_t idx, const AddrDataMapPair &pair) {
  if (idx >= m_len) return false;

  m_data[idx] = pair;
  return true;
}

bool AddrDataMap::get_pair(uint16_t idx, AddrDataMapPair *pair) {
  if (idx >= m_len) return false;

  uint32_t temp;
  get_24bit(idx, &temp);

  *pair = (AddrDataMapPair) {(uint16_t) (temp >> 8), (uint8_t) (temp & 0xFF)};
  return true;
}

bool AddrDataMap::get_24bit(uint16_t idx, uint32_t *val) {
  if (idx >= m_len) return false;

  *val = (((uint32_t) m_data[idx].addr) << 8) | m_data[idx].data;
  return true;
}

void AddrDataMap::purge() {
  if (m_data != nullptr) {
    free(m_data);
    m_data = nullptr;
  }

  m_len = 0;
}

uint16_t AddrDataMap::get_len() {
  return m_len;
}
