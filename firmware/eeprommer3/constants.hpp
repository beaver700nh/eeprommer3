#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "strfmt.hpp"

/*****************************************/
/** Macros *******************************/
/*****************************************/

#define BYTE_FMT "%c%c%c%c%c%c%c%c"
#define BYTE_FMT_VAL(n)   \
  (n & 0x80 ? '1' : '0'), \
  (n & 0x40 ? '1' : '0'), \
  (n & 0x20 ? '1' : '0'), \
  (n & 0x10 ? '1' : '0'), \
  (n & 0x08 ? '1' : '0'), \
  (n & 0x04 ? '1' : '0'), \
  (n & 0x02 ? '1' : '0'), \
  (n & 0x01 ? '1' : '0')

#define BIT_WIDTH(t) sizeof(t) * 8

#define PAGE_ADJUSTED(addr, page) (((page) << 8) + ((addr) & 0xFF))

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

#define ARR_LEN(a) (sizeof((a)) / sizeof((a)[0]))
#define IN_RANGE(n, a, b) ((a) <= (n) && (n) < (b))

#define TO_565(r, g, b) (((r) >> 3 << 11) | ((g) >> 2 << 5) | ((b) >> 3))

#define BOTTOM_BTN(tft, text) 10, (tft).height() - 34, (tft).width() - 20, 24, ((tft).width() + 2 - strlen((text)) * 12) / 2, 5, (text)

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

#define MCP_GPA(n) (n)
#define MCP_GPB(n) (8 + (n))

#define MCP_EE_WE MCP_GPB(7) // I2C addr 0x21
#define MCP_EE_OE MCP_GPB(7) // I2C addr 0x20

#define MCP_EE_DATA_PORT  0
#define MCP_EE_DATA(n)    (n) // I2C addr 0x21
#define MCP_EE_ADDRL_PORT 0
#define MCP_EE_ADDRH_PORT 1
#define MCP_EE_ADDR(n)    (n) // I2C addr 0x20

/*****************************************/
/** Other ********************************/
/*****************************************/

template<typename T>
void swap(T *a, T *b) {
  T temp = *a;
  *a = *b;
  *b = temp;
}

#endif
