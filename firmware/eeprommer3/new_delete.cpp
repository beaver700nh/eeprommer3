#include <Arduino.h>

#include "constants.hpp"

void *operator new[](size_t size) {
  return malloc(size);
}

void *operator new(size_t size) {
  return malloc(size);
}


void *operator new[](size_t size, void *ptr) {
  (void) ptr; // Silence unused parameter
  return malloc(size);
}


void *operator new(size_t size, void *ptr) {
  (void) ptr; // Silence unused parameter
  return malloc(size);
}

void operator delete[](void *ptr) {
  free(ptr);
}

void operator delete(void *ptr) {
  free(ptr);
}

void operator delete[](void *ptr, size_t size) {
  (void) size; // Silence unused parameter
  free(ptr);
}

void operator delete(void *ptr, size_t size) {
  (void) size; // Silence unused parameter
  free(ptr);
}
