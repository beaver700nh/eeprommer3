#include <cstdint>
#include <cstdlib>

#include "util.hpp"

void free2d(void **ptr, uint32_t end, uint32_t start) {
  for (uint32_t i = start; i < end; ++i) {
    free(ptr[i]);
  }

  free(ptr);
}
