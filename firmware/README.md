# eeprommer3 | firmware
This directory contains code for the hardware (an Arduino Mega 2560 in
the files provided). The code is in C++ with a C-like style and uses the
SD, Wire, MCUFRIEND_kbv, TouchScreen, Adafruit_MCP23XXX, and
Adafruit_BusIO libraries.

The hardware allows for a 3.5" TFT LCD screen (with a touch screen).

The `serialcxx/` directory contains code for a basic test of the serial
communication between the firmware and the software.
The `eeprommer3/` directory contains code for the real program which
puts data into and EEPROM.

See `../software/README.md` for more information about code.
