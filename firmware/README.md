# eeprommer3 | firmware

## Brief Description

This directory contains code for the hardware (an Arduino Mega 2560 and a 3.5"
TFT LCD touchscreen). The code is in C++ with a C-like style and uses the SPI,
Wire, SD, MCUFRIEND\_kbv, and Adafruit\_TouchScreen, libraries, as well as AVR's
`<util/delay.h>` header.

\[TODO - briefly mention heap in external RAM]

## Directories

\[TODO - explain addrlatchtest]

The `eeprommer3/` directory contains code for the actual firmware.

\[TODO - explain memory]

The `serialcxx/` directory contains code for a basic test of the serial
communication between the firmware and the software.

\[TODO - explain xmem]

## Files

### `ad_array.cpp`/`ad_array.hpp`

These files define the `AddrDataPair` class to hold the pair of a 16-bit
address and an 8-bit data value. These files also define the `AddrDataArray`
class which holds multiple `AddrDataPair`s.

### `comm.cpp`/`comm.hpp`

These files contain code for a basic protocol for communication with a host
device over the serial port. Note that the code here is very old and is not
guaranteed to work properly. It is also not being used at the moment.

### `constants.hpp`

This file is included in almost all other files in this directory. It
contains various `#define`d macros and constants for general utility use in
the firmware code.

### `dialog.cpp`/`dialog.hpp`

These files define functions used to display helpful full-screen dialogs for
asking the user to enter or select values. This is done by using GUI classes
defined in `gui.cpp`/`gui.hpp`.

### `eeprom.cpp`/`eeprom.hpp`

These files define the `EepromCtrl` class which acts as the core back end of
eeprommer3, and handles the actual hardware interfacing with the EEPROM chip.

### `eeprommer3.cpp`

This is the main file from which code execution starts.

### `error.cpp`/`error.hpp`

These files add the namespace `ErrorLevel`, which contains an enumeration for
an error level from INFO to ERROR, and some tables to translate these values
into their respective names and colors. They also add the Dialog function
`show_error`, which shows an error with its error level, title, and message to
the TFT, then waits for the user to press a "Continue" button.

### `file.cpp`/`file.hpp`

These files contain functions and classes for handling files, file paths, and
file systems. They contain code for the `FileSystem` enumeration, a `Gui` class
to ask the user to select a file on the SD card, and a Dialog function to ask
the user to either type a file path or select a file on any available file
system. The `FileUtil` namespace contains functions for editing file paths.
Note that the only currently supported file system is the SD card.

### `gui.cpp`/`gui.hpp`

These files contain the `Gui` namespace which contains many classes for GUI-
related things like buttons, menus, and progress bars. Some other files add
things to the `Gui` namespace.

### `new_delete.cpp`/`new_delete.hpp`

These files add better support for C++'s `new` and `delete` operators, because
the default Arduino support for these can be glitchy and spotty.

### `prog.cpp`/`prog.hpp`

These files define the class `Programmer` which is the main firmware class. It
hooks together all the Cores and uses a menu to let the user call functions
defined in them.

### `prog_core.cpp`/`prog_core.hpp`

These files contain classes called Cores, each handling one way of programming
the EEPROM. These classes tie together the front end and back end. For example,
`ProgrammerByteCore` contains functions for letting the user read and write a
single byte to and from the EEPROM.

### `sd.cpp`/`sd.hpp`

These files simply define the class `SdCtrl` which uses the built-in Arduino SD
class to provide more features such as easily getting all files in a directory,
and easier initialization.

### `startup.bin`

This doesn't actually contain code, but it's rather an image file encoded in a
way that can be easily decoded by the code and drawn to the TFT screen. The
image is encoded as a series of 16-bit values that encode colors in 565 format.

### `strfmt.cpp`/`strfmt.hpp`

These files contain useful functions and macros for string manipulation, such as
inline string formatting and auto-computed xxprintf buffer lengths, useful in
printing messages to the serial port or the TFT.

### `tft.cpp`/`tft.hpp`

These files contain the `TftCtrl` class, a wrapper around MCUFRIEND\_kbv, and
the `TftColor` namespace, which contains an enumeration for many 565-format
colors used across the firmware code.

### `tft_calc.cpp`/`tft_calc.hpp`

These files contain the namespace `TftCalc`, which contains functions for
calculating TFT coordinates.

### `tft_util.cpp`/`tft_util.hpp`

These files contain the namespace `TftUtil`, which contains miscellaneous
utilities relating to the TFT screen.

### `touch.cpp`/`touch.hpp`

These files define the `TouchCtrl` class, which is a wrapper around the
TouchScreen class. `TouchCtrl` adds features like automatically mapping raw
touchscreen coordinates to TFT coordinates using calibrated values.

### `util.cpp`/`util.hpp`

These files contain miscellaneous helpful functions that don't really belong in
any other category. The functions can be found inside the namespace `Util`.

### `vector.cpp`/`vector.hpp`

These files define the `Vector` struct, which stores information about one of
the three 6502 jump vectors IRQ, RST, and NMI. Also, the function `ask_vector`
is added to the Dialog namespace.

### `xram.cpp`/`xram.hpp`

\[TODO - explain more deeply how it works]
