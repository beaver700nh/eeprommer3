# EEPROMmer3
## The 3rd revision of my EEPROM programmer

This repository contains the code for both the interface and
hardware sides of eeprommer3, as well as the schematics and
EAGLE files for the hardware.

The interface is written in C++ using wxWidgets and the
hardware provided is an Arduino Mega 2560 driving an Elegoo
2.8" TFT LCD display with a resistive touchscreen and the PCB
which contains the actual EEPROM programming hardware. Note
that you can also use other hardware, but you will have to
modify/redesign/fork some parts of the code (mostly just the
hardware/firmware side, though).

## Obtaining eeprommer3

### Downloading

You can download either the code or just the executable. If
you want/need to be able to make changes to the code, then
download the whole thing, otherwise just download the executable.

#### Just the executable

Go to https://github.com/beaver700nh/eeprommer3/blob/master/build/eeprommer3
and click `Download`. BOOM. Acquired. You can skip to the
`Installing` step.

#### The whole thing

##### The GUI way

Click the `Download as ZIP` button on the GitHub page.
Then `<path to eeprommer3>` will be wherever you downloaded
the code to.

##### The CLI way

You can use `wget` to download the source code.

```shell
$ cd <where you want to download eeprommer3>/
$ wget https://github.com/beaver700nh/eeprommer3/
```
Then `<path to eeprommer3>` will be wherever you downloaded
the code to.

##### Building

Compile the code using make:
```shell
$ cd <path to eeprommer3>/build/
$ make -j$(nproc)
```

Then run the program to verify that it works:
```shell
$ make run
```
This will only work if you are in the `eeprommer3/build`
directory. If you aren't, you can use this command instead:
```shell
$ <path to eeprommer3>/build/eeprommer3
```

### Installing

If you wish to be able to access the `eeprommer3` from your
entire system, you may want to create a link to the executable:
```shell
$ cd /usr/local/bin/
$ sudo ln <path to eeprommer3>/build/eeprommer3 eeprommer3
```
Or if you just downloaded the executable by itself:
```shell
$ cd /usr/local/bin/
$ sudo ln <path to the executable>/eeprommer3 eeprommer3
```

And then you can use `eeprommer3` from anywhere on your system!

## Usage

\[TODO]

If you find any bugs or have any suggestions, by all means
report them to the issue tracker!
