#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "strfmt.hpp"

/*****************************************/
/** Global Objects ***********************/
/*****************************************/

#include "HardwareSerial.h"

extern HardwareSerial Serial;

/*****************************************/
/** Compile-Time Constants ***************/
/*****************************************/

#define DEBUG_MODE

// #define LOGGING

/*****************************************/
/** Macros *******************************/
/*****************************************/

#define BYTE_FMT "%c%c%c%c%c%c%c%c"
#define BYTE_FMT_VAL(n)    \
  ((n) & 0x80 ? '1' : '0'), \
  ((n) & 0x40 ? '1' : '0'), \
  ((n) & 0x20 ? '1' : '0'), \
  ((n) & 0x10 ? '1' : '0'), \
  ((n) & 0x08 ? '1' : '0'), \
  ((n) & 0x04 ? '1' : '0'), \
  ((n) & 0x02 ? '1' : '0'), \
  ((n) & 0x01 ? '1' : '0')

// Returns size of `t` but in bits instead of bytes
#define BIT_WIDTH(t) (sizeof(t) * 8)

// Returns `addr` but with 2nd and above most significant bytes equal to `page`
// - PAGE_ADJUSTED(0x123456FF, 0x00654321) => 0x654321FF
#define PAGE_ADJUSTED(addr, page) (((page) << 8) + ((addr) & 0xFF))

// Silences "unused variable" warnings
#define UNUSED_VAR(name) (void) name

// Returns number of elements in array `a`
#define ARR_LEN(a) (sizeof((a)) / sizeof((a)[0]))

// Checks if `n` is in range [`a`, `b`)
#define IN_RANGE(n, a, b) ((a) <= (n) && (n) < (b))

// Returns the smaller of `a` and `b`, or `b` if they are equal
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// Returns the larger of `a` and `b`, or `b` if they are equal
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Returns a 565-color representation of 24-bit color (`r`, `g`, `b`)
#define TO_565(r, g, b) (((r) >> 3 << 11) | ((g) >> 2 << 5) | ((b) >> 3))

// These return the individual RGB components of a 565 color

#define RED_565(c) ((c) >> 11)
#define GRN_565(c) (((c) >> 5) & 0x3F)
#define BLU_565(c) ((c) & 0x1F)

// Shorthand for bottom button in Btn ctor
// `tft`: TftCtrl object to get dimensions
// `text`: text of button
#define BOTTOM_BTN(text)            \
  10, TftCalc::bottom(tft, 24, 10),    \
  TftCalc::fraction_x(tft, 10, 1), 24, \
  (text)

// Prints text over Serial if logging is enabled
// - SER_DEBUG_PRINT(foo, 's') where foo = "bar" => prints "SER_DEBUG_PRINT: foo = bar"
#ifdef LOGGING
#define SER_DEBUG_PRINT(var, type) PRINTF_NOBUF(&Serial, STRFMT_P_NOBUF(PSTR("SER_DEBUG_PRINT: %%s = %%%c\n"), type), #var, var)
#define SER_LOG_PRINT(text, ...) PRINTF_P_NOBUF(&Serial, PSTR("*!* - SER_LOG_PRINT: ...%-12s :%-4d || " text), __FILE__ + max(strlen(__FILE__), 12) - 12, __LINE__, ##__VA_ARGS__)
#else
#define SER_DEBUG_PRINT(var, type)
#define SER_LOG_PRINT(text, ...)
#endif

/*****************************************/
/** Constants ****************************/
/*****************************************/

#define XRAM_8K_BUF 0xE000

#define SD_CS 10
#define SD_EN A15

#define TFT_DRIVER 0x9486

#define TS_XP     8
#define TS_XM     A2
#define TS_YP     A3
#define TS_YM     9
#define TS_RESIST 300

#define TS_MINX 913
#define TS_MAXX 131
#define TS_MINY 944
#define TS_MAXY 90

#define T_DEBOUNCE 200

#endif
