.SILENT: all build compile upload run new
.PHONY: all build compile upload run new

.SHELL := /bin/bash

all: compile upload run

build: compile upload

.ONESHELL:
compile:
ifeq ($(DEBUG_MODE), 1)
	debug_arg="--build-property=build.extra_flags=\"-DDEBUG_MODE=1\""
else
	debug_arg=""
endif

ifeq ($(CLEAN), 1)
	debug_arg="$$debug_arg --clean"
endif

	arduino-cli-mega-compile . --warnings=more --build-property=compiler.cpp.extra_flags="-std=c++1z" "$$debug_arg"

upload:
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose

run:
ifeq ($(SER_MON), 1)
### Map LF to CR+LF to avoid staircase printing
	picocom --baud=115200 --databits=8 --parity=n --stopbits=1 --flow=n --imap="lfcrlf" /dev/ttyACM0
endif

.ONESHELL:
new:
	read -p "Name of new cpp/hpp pair? " name
	upper_name=$$(echo $$name | tr "[:lower:]" "[:upper:]")

	hpp=$$(cat file_template/hpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g")
	cpp=$$(cat file_template/cpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g")

	printf "$$hpp" >$$name.hpp
	printf "$$cpp" >$$name.cpp
