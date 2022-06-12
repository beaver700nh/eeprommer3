#ifndef STRFMT_HPP
#define STRFMT_HPP

extern char custom_printf_buf[128];
extern char custom_strfmt_buf[128];

#define SNPRINTF(buf, fmt, ...) snprintf(buf, ARR_LEN(buf), fmt, ##__VA_ARGS__)
#define STRNCPY(dst, src)       strncpy(dst, src, ARR_LEN(dst))

#define PRINTF_NOBUF(obj, fmt, ...) ( \
  SNPRINTF(custom_printf_buf, fmt, ##__VA_ARGS__), \
  obj.print(custom_printf_buf) \
)

#define PRINTF(buf, obj, fmt, ...) ( \
  SNPRINTF(buf, fmt, ##__VA_ARGS__), \
  obj.print(buf) \
)

#define PRINTF_BUFLEN(buf, len, obj, fmt, ...) ( \
  snprintf(buf, len, fmt, ##__VA_ARGS__), \
  obj.print(buf) \
)

#define STRFMT_NOBUF(fmt, ...) ( \
  SNPRINTF(custom_strfmt_buf, fmt, ##__VA_ARGS__), \
  custom_strfmt_buf \
)

#define STRFMT(buf, fmt, ...) ( \
  SNPRINTF(buf, fmt, ##__VA_ARGS__), \
  buf \
)

#define STRFMT_BUFLEN(buf, len, fmt, ...) ( \
  snprintf(buf, len, fmt, ##__VA_ARGS__), \
  buf \
)

#endif
