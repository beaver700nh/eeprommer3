#include <cstdint>
#include <cstdlib>
#include <functional>

#include "dlgbox.hpp"
#include "util.hpp"
#include "wx_dep.hpp"

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

template<typename Type, uint16_t x, uint16_t y>
void flatten_2d21d(Type (*in)[x][y], Type **out, uint8_t bit_width) {
  for (uint16_t i = 0; i < x; ++i) {
    for (uint16_t j = 0; j < y; ++j) {
      uint32_t idx = (
        ((i & ((1 << bit_width) - 1)) << bit_width) | \
        (j & ((1 << bit_width) - 1))
      );

      DlgBox::info(wxString::Format("idx: %d", idx), "YO INFO", wxOK);

      (*out)[idx] = (*in)[i][j];
    }
  }
}
