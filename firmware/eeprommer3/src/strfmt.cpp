#include <Arduino.h>
#include "constants.hpp"

#include <stdarg.h>
#include <stddef.h>

#include <Print.h>

#include "strfmt.hpp"

char custom_printf_buf[128];
char custom_strfmt_buf[128];

int VPRINTF(char *buf, size_t len, Print *obj, const char *fmt, va_list args) {
  int ret = vsnprintf(buf, len, fmt, args);
  obj->print(buf);
  return ret;
}

int PRINTF(char *buf, size_t len, Print *obj, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = VPRINTF(buf, len, obj, fmt, args);

  va_end(args);
  return ret;
}

int PRINTF_NOBUF(Print *obj, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = VPRINTF(custom_printf_buf, ARR_LEN(custom_printf_buf), obj, fmt, args);

  va_end(args);
  return ret;
}

int VPRINTF_P(char *buf, size_t len, Print *obj, const char *fmt, va_list args) {
  int ret = vsnprintf_P(buf, len, fmt, args);
  obj->print(buf);
  return ret;
}

int PRINTF_P(char *buf, size_t len, Print *obj, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = VPRINTF_P(buf, len, obj, fmt, args);

  va_end(args);
  return ret;
}

int PRINTF_P_NOBUF(Print *obj, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = VPRINTF_P(custom_printf_buf, ARR_LEN(custom_printf_buf), obj, fmt, args);

  va_end(args);
  return ret;
}

const char *VSTRFMT(char *buf, size_t len, const char *fmt, va_list args) {
  vsnprintf(buf, len, fmt, args);
  return buf;
}

const char *STRFMT(char *buf, size_t len, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto *ret = VSTRFMT(buf, len, fmt, args);

  va_end(args);
  return ret;
}

const char *STRFMT_NOBUF(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto *ret = VSTRFMT(custom_strfmt_buf, ARR_LEN(custom_strfmt_buf), fmt, args);

  va_end(args);
  return ret;
}

const char *VSTRFMT_P(char *buf, size_t len, const char *fmt, va_list args) {
  vsnprintf_P(buf, len, fmt, args);
  return buf;
}

const char *STRFMT_P(char *buf, size_t len, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto *ret = VSTRFMT_P(buf, len, fmt, args);

  va_end(args);
  return ret;
}

const char *STRFMT_P_NOBUF(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto *ret = VSTRFMT_P(custom_strfmt_buf, ARR_LEN(custom_strfmt_buf), fmt, args);

  va_end(args);
  return ret;
}
