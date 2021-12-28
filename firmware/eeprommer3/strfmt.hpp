#ifndef STRFMT_HPP
#define STRFMT_HPP

extern char custom_printf_buf[100];
extern char custom_strfmt_buf[100];

#define PRINTF_NOBUF(obj, fmt, ...) \
  ( \
    snprintf(custom_printf_buf, sizeof(custom_printf_buf), fmt, ##__VA_ARGS__), \
    obj.print(custom_printf_buf) \
  )

#define PRINTF(buf, obj, fmt, ...) \
  ( \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__), \
    obj.print(buf) \
  )

#define PRINTF_BUFLEN(buf, len, obj, fmt, ...) \
  ( \
    snprintf(buf, len, fmt, ##__VA_ARGS__), \
    obj.print(buf) \
  )

#define STRFMT_NOBUF(fmt, ...) \
  ( \
    snprintf(custom_strfmt_buf, sizeof(custom_strfmt_buf), fmt, ##__VA_ARGS__), \
    custom_strfmt_buf \
  )

#define STRFMT(buf, fmt, ...) \
  ( \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__), \
    buf \
  )

#define STRFMT_BUFLEN(buf, len, fmt, ...) \
  ( \
    snprintf(buf, len, fmt, ##__VA_ARGS__), \
    buf \
  )

#endif
