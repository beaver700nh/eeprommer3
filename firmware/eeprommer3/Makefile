.SILENT: all build compile upload run
.PHONY: all build compile upload run

all: compile upload run

build: compile upload

.ONESHELL:
compile:
ifeq ($(DEBUG_MODE), 1)
	debug_arg="--build-property=build.extra_flags=\"-DDEBUG_MODE=1\""
else
	debug_arg=""
endif

	arduino-cli-mega-compile . --warnings pedantic --build-property=compiler.cpp.extra_flags="-std=c++1z" "$$debug_arg"

upload:
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose

run:
ifeq ($(SER_MON), 1)
### Map LF to CR+LF to avoid staircase printing
	picocom --baud=115200 --databits=8 --parity=n --stopbits=1 --flow=n --imap="lfcrlf" /dev/ttyACM0
endif
