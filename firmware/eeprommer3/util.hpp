#ifndef UTIL_HPP
#define UTIL_HPP

#ifdef __arm__
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif

namespace Util {
  int get_free_memory();

  template<typename T>
  void swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
  }
};

#endif
