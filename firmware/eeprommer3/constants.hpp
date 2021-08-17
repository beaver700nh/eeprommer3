#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define DEBUG_MODE // Comment out to disable debug printing

#define TFT_CS     A3
#define TFT_RS     A2
#define TFT_WR     A1
#define TFT_RD     A0
#define TFT_RESET  A4
#define TFT_DRIVER 0x9341

#define SD_CS 10
#define SD_EN A15

#define TS_XP     8
#define TS_XM     A2
#define TS_YP     A3
#define TS_YM     9
#define TS_RESIST 300

#define TS_MIN_PRESSURE 10

#define TS_CALIB_MINX 78
#define TS_CALIB_MAXX 925
#define TS_CALIB_MINY 72
#define TS_CALIB_MAXY 905

#endif
