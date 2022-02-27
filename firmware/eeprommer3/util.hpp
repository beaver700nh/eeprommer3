#ifndef UTIL_HPP
#define UTIL_HPP

namespace Util {
  template<typename T>
  void swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
  }
};

#endif
