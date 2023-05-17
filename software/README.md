# eeprommer3 | software

This directory contains a small Python program which acts as an interface
for the firmware. The program listens for commands over the selected serial
port, then reacts accordingly.

## Prerequisites

Python 3, PySerial
```shell
$ python -m pip install pyserial
```
The program also invokes the `dialog` command, which should be builtin on Linux.

## Usage

Important: Make sure you add yourself to your OS's serial port access group.
For most distributions this is `dialout`:
```shell
$ sudo adduser yourusername dialout
```
For Arch this is `uucp`:
```shell
$ sudo gpasswd -a yourusername uucp
```

Make sure your board is plugged in and is not being accessed by any other programs.

```shell
$ python src/main.py --port PORT
```
Where `PORT` is something like `/dev/ttyACM0`, `/dev/ttyUSB0`, or `/dev/ttyS0`.
