#include <Arduino.h>
#include "constants.hpp"

void *operator new[](size_t size);
void *operator new(size_t size);

void *operator new[](size_t size, void *ptr) noexcept;
void *operator new(size_t size, void *ptr) noexcept;

void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr) noexcept;

void operator delete[](void *ptr, size_t size) noexcept;
void operator delete(void *ptr, size_t size) noexcept;
