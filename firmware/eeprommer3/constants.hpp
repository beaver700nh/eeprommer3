#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define DEBUG_MODE // Comment out to disable debug printing

#define SD_CS 10
#define SD_EN A15

#define TFT_CS     A3
#define TFT_RS     A2
#define TFT_WR     A1
#define TFT_RD     A0
#define TFT_RESET  A4
#define TFT_DRIVER 0x9341

#define TFT_WIDTH  320
#define TFT_HEIGHT 240

#define JST_X_AXIS A12
#define JST_Y_AXIS A13
#define JST_BUTTON 46
#define JST_SPEED  5

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
