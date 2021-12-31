.SILENT: all build run
.PHONY: all build run

all: build run

.ONESHELL:
build:
ifeq ($(DEBUG_MODE), 1)
	debug_arg="--build-property=build.extra_flags=\"-DDEBUG_MODE=1\""
else
	debug_arg=""
endif

	arduino-cli-mega-compile . --warnings pedantic --build-property=compiler.cpp.extra_flags="-std=c++1z" "$$debug_arg"

run:
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose

### sleep to allow Arduino to initialize
### before starting picocom
	sleep 11

ifeq ($(SER_MON), 1)
### Baudrate:   115200
### paritY:     none
### Databits:   8
### stoPbits:   1
### MAPping:    LF -> CR + LF
	picocom -b 115200 -y n -d 8 -p 1 --imap="lfcrlf" /dev/ttyACM0 &
endif
