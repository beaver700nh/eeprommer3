# eeprommer3 | firmware
This directory contains code for the hardware (an Arduino Mega 2560 in the files provided).
The code is in C++ with a C-like style and uses the SD, SPI, Wire, and Adafruit_MCP23017 libraries.

The hardware allows for either a 2.8x3.2" TFT LCD or a 16x2 HD44780 character LCD (or theoretically both,
although this is not officially supported).
If one chooses to use the TFT, the code also requires the Elegoo/Adafruit_TFTLCD and TouchScreen libraries.
If one uses the 16x2 LCD instead, the code instead also requires the LiquidCrystal library.

The `serialcxx/` directory contains code for a basic serial communications test.
The `eeprommer3/` directory contains code for the real program which puts data into and EEPROM.

See `../software/README.md` for more information about code.
