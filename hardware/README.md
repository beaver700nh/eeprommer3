# eeprommer3 | hardware

The hardware provided is an Arduino Mega 2560 driving a 3.5" TFT LCD
display (ILI9486) with a resistive touchscreen. The PCB contains the
actual EEPROM programming hardware. Note that you can also use other
hardware, but you will have to modify/redesign/fork some parts of the
code (mostly just the hardware/firmware side, though).

## v2.y - August 2022

This directory contains images and LibrePCB files for each revision of
the eeprommer3 Arduino Mega shield.

The PCB, which covers the whole Arduino as a pass-through shield,
holds a socket for the AT28C256 EEPROM. It also contains the EEPROM
driver circuitry and RAM expansion circuitry.

Support for use as a breakout board as well as use with boards other
than Arduino Megas has been dropped. It would probably also be very
difficult to modify the PCB for use with other EEPROMs.

Support for the character LCD has also been dropped in favor of a TFT
LCD touchscreen, which is now 3.5" instead of 2.8". The screen plugs in
on top of the shield instead of next to it.

## v1.y - July 2021

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
the rest of the space is intended for a 2.8" TFT LCD display. One can
choose to use either this or the aforementioned 16x2 character LCD.
