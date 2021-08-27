#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define DEBUG_MODE // Comment out to disable debug printing

#define IN_RANGE(n, a, b) ((a) <= (n) && (n) < (b))

#define SD_CS 10
#define SD_EN A15

#define TFT_DRIVER 0x9486

#define TFT_WIDTH  480
#define TFT_HEIGHT 320

#define TS_XP     8
#define TS_XM     A2
#define TS_YP     A3
#define TS_YM     9
#define TS_RESIST 300

#define TS_MINX 913
#define TS_MAXX 131
#define TS_MINY 934
#define TS_MAXY 90

#define MCP_GPA(n) (n)
#define MCP_GPB(n) (8 + (n))

#define MCP_EE_WE MCP_GPB(7) // I2C addr 0x21
#define MCP_EE_OE MCP_GPB(7) // I2C addr 0x20

#define MCP_EE_DATA_PORT  0
#define MCP_EE_DATA(n)    n // I2C addr 0x21
#define MCP_EE_ADDRL_PORT 0
#define MCP_EE_ADDRH_PORT 1
#define MCP_EE_ADDR(n)    n // I2C addr 0x20

#endif
