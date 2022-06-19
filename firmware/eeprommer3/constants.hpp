#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// To silence clangd errors

#include "HardwareSerial.h"

extern HardwareSerial Serial;

// End silence clangd errors

#include "strfmt.hpp"

/*****************************************/
/** Compile-Time Constants ***************/
/*****************************************/

#define DEBUG_MODE

//#define LOGGING

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

// Shorthand for bottom button in TftBtn ctor
// `tft`: TftCtrl object to get dimensions
// `text`: text of button
#define BOTTOM_BTN(tft, text)            \
10, TftCalc::bottom((tft), 24, 10),      \
  TftCalc::fraction_x((tft), 10, 1), 24, \
  (text)

inline char ser_log_print_fname[128];

// Prints text over Serial if logging is enabled
// - SER_DEBUG_PRINT(foo, 's') where foo = "bar" => prints "SER_DEBUG_PRINT: foo = bar"
#ifdef LOGGING
#define SER_DEBUG_PRINT(var, type) PRINTF_NOBUF(Serial, STRFMT_NOBUF("SER_DEBUG_PRINT: %%s = %%%c\n", type), #var, var)
#define SER_LOG_PRINT(text, ...) (                                  \
  STRNCPY(ser_log_print_fname, __FILE__),                           \
                                                                    \
  PRINTF_NOBUF(                                                     \
    Serial, "*!* - SER_LOG_PRINT: ...%-25.25s :%-4d || " text,      \
    strrchr(ser_log_print_fname, '/') - 10, __LINE__, ##__VA_ARGS__ \
  )                                                                 \
)
#define SER_LOG_PRINTN(text, ...) PRINTF_NOBUF(Serial, text, ##__VA_ARGS__)
#else
#define SER_DEBUG_PRINT(var, type)
#define SER_LOG_PRINT(text, ...)
#define SER_LOG_PRINTN(text, ...)
#endif

#define TYPED_CONTROLLERS     TftCtrl &tft, TouchCtrl &tch, EepromCtrl &ee, SdCtrl &sd
#define INIT_LIST_CONTROLLERS m_tft(tft),   m_tch(tch),     m_ee(ee),       m_sd(sd)
#define CONTROLLERS           tft,          tch,            ee,             sd

/*****************************************/
/** Constants ****************************/
/*****************************************/

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

#define MCP_GPA(n) (n)
#define MCP_GPB(n) (8 + (n))

#define MCP_EE_WE MCP_GPB(7)   // I2C addr 0x21
#define MCP_EE_OE MCP_GPB(7)   // I2C addr 0x20

#define MCP_EE_WE_PORT    1    // I2C addr 0x21
#define MCP_EE_DATA_PORT  0    // I2C addr 0x21
#define MCP_EE_DATA(n)    (n)  // I2C addr 0x21
#define MCP_EE_ADDRL_PORT 0    // I2C addr 0x20
#define MCP_EE_ADDRH_PORT 1    // I2C addr 0x20
#define MCP_EE_ADDR(n)    (n)  // I2C addr 0x20

#endif
