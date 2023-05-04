#ifndef STRFMT_HPP
#define STRFMT_HPP

#include <stdarg.h>
#include <stddef.h>

extern char custom_printf_buf[128];
extern char custom_strfmt_buf[128];

#define snprintf_sz(buf, fmt, ...) snprintf(buf, ARR_LEN(buf), fmt, ##__VA_ARGS__)
#define vsnprintf_sz(buf, fmt, args) vsnprintf(buf, ARR_LEN(buf), fmt, args)
#define strncpy_sz(dst, src) strncpy(dst, src, ARR_LEN(dst))

#define snprintf_P_sz(buf, fmt, ...) snprintf_P(buf, ARR_LEN(buf), fmt, ##__VA_ARGS__)
#define vsnprintf_P_sz(buf, fmt, args) vsnprintf_P(buf, ARR_LEN(buf), fmt, args)
#define strncpy_P_sz(dst, src) strncpy_P(dst, src, ARR_LEN(dst))

class Print;

int PRINTF(char *buf, size_t len, Print *obj, const char *fmt, ...);
int PRINTF_NOBUF(Print *obj, const char *fmt, ...);

#define PRINTF_sz(buf, obj, fmt, ...) PRINTF(buf, ARR_LEN(buf), obj, fmt, ##__VA_ARGS__)

int PRINTF_P(char *buf, size_t len, Print *obj, const char *fmt, ...);
int PRINTF_P_NOBUF(Print *obj, const char *fmt, ...);

#define PRINTF_P_sz(buf, obj, fmt, ...) PRINTF_P(buf, ARR_LEN(buf), obj, fmt, ##__VA_ARGS__)

const char *STRFMT(char *buf, size_t len, const char *fmt, ...);
const char *STRFMT_NOBUF(const char *fmt, ...);

#define STRFMT_sz(buf, fmt, ...) STRFMT(buf, ARR_LEN(buf), fmt, ##__VA_ARGS__)

const char *STRFMT_P(char *buf, size_t len, const char *fmt, ...);
const char *STRFMT_P_NOBUF(const char *fmt, ...);

#define STRFMT_P_sz(buf, fmt, ...) STRFMT_P(buf, ARR_LEN(buf), fmt, ##__VA_ARGS__)

#endif
