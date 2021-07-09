#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <functional>

#include "wx_dep.hpp"

#define OnErrFunc std::function<void(wxString, wxString, int)>
#define OnErrFuncLambda (wxString message, wxString title, int style)

template<typename Type>
Type **init2d(uint16_t x, uint16_t y, OnErrFunc on_err);
void free2d(void **ptr, uint32_t end, uint32_t start = 0);

#include "util.tpp"

#endif
