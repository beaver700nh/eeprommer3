#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <functional>

#include "wx_dep.hpp"

#define ON_ERR_FUNC        std::function<void(wxString, wxString, int)>
#define ON_ERR_FUNC_LAMBDA (wxString message, wxString title, int style) -> void

template<typename Type>
Type **init2d(uint16_t x, uint16_t y, ON_ERR_FUNC on_err);
void free2d(void **ptr, uint32_t end, uint32_t start = 0);

template<typename Type, uint16_t x, uint16_t y>
void flatten_2d21d(Type *in[x][y], Type **out, uint8_t bit_width = 16);

#include "util.tpp"

#endif
