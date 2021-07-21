# eeprommer3 | hardware
This directory contains images of the PCB and schematic files, as well as
the EasyEDA files of the Arduino Mega shield for the EEPROM programmer,
for each revision.

The `layer_pngs/` directories of each revision contain PNGs of each of the
data layers of the PCB.

The PCB fits onto the right side of an Arduino Mega and has space for
two MCP23017 I2C port expanders, a 16x2 HD44780 character LCD, a 0.3x0.3"
potentiometer, two 3mm LEDs, two through-hole resistors, 2 solder jumper
switches, the actual AT28C256 EEPROM, as well as the headers for connecting
to the Arduino.

The PCB can also be used as a breakout board, depending on how headers are soldered on.
This is how it is intended to be used with non-Mega Arduinos.

With some modifications, it should also be possible to use this PCB with
other EEPROMs as well, such as the AT27Cxxx series or other EEPROMs in the
AT28Cxxx series (e.g. AT28C16, AT28C64).

The reason for fitting onto the right side of an Arduino Mega is that
the rest of the space is intended for a 2.8x3.2" TFT LCD display. One can
choose to use either this or the aforementioned 16x2 character LCD.
