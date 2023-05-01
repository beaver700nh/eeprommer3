#include "constants.hpp"

#ifdef ARDUINO
#include "new_delete.hpp"
#endif

#include "ad_array.hpp"

AddrDataArray::~AddrDataArray() {
  purge();
}

bool AddrDataArray::extend(uint16_t capacity) {
  auto *new_arr = (AddrDataArrayPair *) malloc((m_len + capacity) * sizeof(AddrDataArrayPair));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, m_len * sizeof(AddrDataArrayPair));

  free(m_data);

  m_data = new_arr;
  m_len += capacity;

  return true;
}

bool AddrDataArray::append(const AddrDataArrayPair &pair) {
  if (!extend(1)) return false;

  set_pair(m_len - 1, pair);
  return true;
}

bool AddrDataArray::remove(uint16_t idx) {
  if (idx >= m_len) return false;

  auto new_arr = (AddrDataArrayPair *) malloc((m_len - 1) * sizeof(AddrDataArrayPair));

  if (new_arr == nullptr) return false;

  memcpy(new_arr, m_data, idx * sizeof(AddrDataArrayPair));
  memcpy(new_arr + idx, m_data + idx + 1, (m_len - idx - 1) * sizeof(AddrDataArrayPair));

  free(m_data);

  m_data = new_arr;
  --m_len;

  return true;
}

bool AddrDataArray::set_pair(uint16_t idx, const AddrDataArrayPair &pair) {
  if (idx >= m_len) return false;

  m_data[idx] = pair;
  return true;
}

bool AddrDataArray::get_pair(uint16_t idx, AddrDataArrayPair *pair) {
  if (idx >= m_len) return false;

  uint32_t temp;
  get_24bit(idx, &temp);

  *pair = (AddrDataArrayPair) {(uint16_t) (temp >> 8), (uint8_t) (temp & 0xFF)};
  return true;
}

bool AddrDataArray::get_24bit(uint16_t idx, uint32_t *val) {
  if (idx >= m_len) return false;

  *val = (((uint32_t) m_data[idx].addr) << 8) | m_data[idx].data;
  return true;
}

void AddrDataArray::purge() {
  if (m_data != nullptr) {
    free(m_data);
    m_data = nullptr;
  }

  m_len = 0;
}

uint16_t AddrDataArray::get_len() {
  return m_len;
}
