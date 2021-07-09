#include <cstdint>
#include <cstdlib>
#include <functional>

#include "util.hpp"

template<typename Type>
Type **init2d(uint16_t x, uint16_t y, ON_ERR_FUNC on_err) {
  auto **out = (Type **) malloc(sizeof(Type *) * x);

  if (out == nullptr) {
    on_err("Could not allocate memory.", "Memory Error", wxOK);
    return (Type **) nullptr;
  }

  for (uint16_t i = 0; i < 256; ++i) {
    out[i] = (Type *) malloc(sizeof(Type) * y);

    if (out[i] == nullptr) {
      on_err("Could not allocate memory.", "Memory Error", wxOK);
      free2d((void **) out, i);
      return nullptr;
    }
  }

  return out;
}
