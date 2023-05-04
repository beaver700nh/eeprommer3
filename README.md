# EEPROMmer3 -  The 3rd revision of my EEPROM programmer

## About

\[TODO - backstory, explain what ermr3 is]

This repository contains the code for both the interface and hardware
sides of eeprommer3, as well as the schematics and EAGLE files for the
hardware.

[See `software/README.md`](software/README.md)
[See `hardware/README.md`](hardware/README.md)
[See `firmware/README.md`](firmware/README.md)

## Obtaining the eeprommer3 software

\[TODO - remove prebuilt binaries because I will only ever have linux aarch64 builds]

### Building

Compile the code using make:
```shell
$ cd <path to eeprommer3>/software/build/
$ make -j$(nproc) ARCHTT="<your OS>/<Your architecture> # e.g. Linux/ARM or Windows/x86_64"
```

Then run the program to verify that it works:
```shell
$ cd <path to eeprommer3>/build/binaries/<your OS>/<your system architecture>/
$ ./eeprommer3
```

Or alternatively:
```shell
$ cd <path to eeprommer3>/build/
$ make run ARCHTT="<your OS>/<Your architecture>"
```

### Installing

If you wish to be able to access the `eeprommer3` from your
entire system, you may want to create a link to the executable:
```shell
$ cd /usr/local/bin/
$ sudo ln <path to eeprommer3>/build/binaries/<your OS>/<your architecture>/eeprommer3 eeprommer3
```

Or if you just downloaded the executable by itself:
```shell
$ cd /usr/local/bin/
$ sudo ln <path to the executable>/eeprommer3 eeprommer3
```

And then you can use `eeprommer3` from anywhere on your system!

## Usage

\[TODO]

If you find any bugs or have any suggestions, by all means report them
to the issue tracker on GitHub!
(https://github.com/beaver700nh/eeprommer3/)
