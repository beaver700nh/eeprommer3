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

ifeq ($(SER_MON), 1)
	putty /dev/ttyACM0 -serial -sercfg 115200,8,n,1,N &
endif
