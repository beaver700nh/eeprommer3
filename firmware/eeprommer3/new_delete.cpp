#include <Arduino.h>
#include "constants.hpp"

#include "new_delete.hpp"

void *operator new[](size_t size) {
  return malloc(size);
}

void *operator new(size_t size) {
  return malloc(size);
}

void *operator new[](size_t size, void *ptr) noexcept {
  UNUSED_VAR(ptr);
  return malloc(size);
}

void *operator new(size_t size, void *ptr) noexcept {
  UNUSED_VAR(ptr);
  return malloc(size);
}

void operator delete[](void *ptr) noexcept {
  free(ptr);
}

void operator delete(void *ptr) noexcept {
  free(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept {
  UNUSED_VAR(size);
  free(ptr);
}

void operator delete(void *ptr, size_t size) noexcept {
  UNUSED_VAR(size);
  free(ptr);
}
