# EEPROMmer3 -  The 3rd revision of my EEPROM programmer

## About

eeprommer3 is my third attempt at a project that I started on a whim
back in 2021 because I didn't want to buy a commercial EEPROM programmer
for my 6502-based computer inspired by Ben Eater. Since then this code
base has grown into a massive, overengineered system.

This repository contains the code for both the interface and hardware
sides of eeprommer3, as well as the schematics and EAGLE files for the
hardware.

- [See `software/README.md`](software/README.md)
- [See `hardware/README.md`](hardware/README.md)
- [See `firmware/README.md`](firmware/README.md)

## Usage

eeprommer3 can be used with the firmware and hardware alone or with
integration with the software. If a microSD card is present, eeprommer3
can program the EEPROM with files from there. If connected to a computer
over USB, the EEPROM can also be programmed from files on the computer.

\[TODO - explain main loop and actions system]

If you find any bugs or have any suggestions, by all means report them
to the issue tracker on GitHub!
(https://github.com/beaver700nh/eeprommer3/)
