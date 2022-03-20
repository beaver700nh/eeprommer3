# EEPROMmer3 -  The 3rd revision of my EEPROM programmer

## About

This repository contains the code for both the interface and hardware
sides of eeprommer3, as well as the schematics and EAGLE files for the
hardware.

### Software
The interface is written in C++ using wxWidgets as the GUI library.

**NOTE: Because I am developing on Linux ARM, the executable for your
system might either not exist or be outdated. Thus you may have to
compile the code by yourself. Apologies for the inconvenience.**

### Hardware
The hardware provided is an Arduino Mega 2560 driving a 3.5" TFT LCD
display (ILI9486) with a resistive touchscreen. The PCB contains the
actual EEPROM programming hardware. Note that you can also use other
hardware, but you will have to modify/redesign/fork some parts of the
code (mostly just the hardware/firmware side, though).

### Firmware
This is the code that runs on the Arduino.

## Introduction

\[TODO - tell an overview of using eeprommer3]

## Obtaining the eeprommer3 software

### Downloading

You can download either the code or just the executable. If you want or
need to be able to make changes to the code, then download the whole
thing, otherwise just download the executable.

#### Just the executable

##### The GUI way

Go to https://github.com/beaver700nh/eeprommer3/blob/master/software/build/binaries/.
Then click on your computer's OS and architecture, followed by the
`eeprommer3` file. Click `Download`. That's all; it is now acquired. You
can skip to the `Installing` step.

##### The CLI way

You can use `wget` to download the executable.

```shell
$ cd <where you want to download the executable>/
$ wget https://github.com/beaver700nh/eeprommer3/blob/master/software/build/binaries/<your OS>/<your system architecture>/eeprommer3.
```

#### The whole thing

##### The GUI way

Click the `📥 Code` button, then the `Download as ZIP` button on the
GitHub page. Then `<path to eeprommer3>` will be wherever you downloaded
the code to.

##### The CLI way

You can use `wget` to download the source code.

```shell
$ cd <where you want to download eeprommer3>/
$ wget https://github.com/beaver700nh/eeprommer3/
```

Then `<path to eeprommer3>` will be wherever you downloaded the code to.

##### Building

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
