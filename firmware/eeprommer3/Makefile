.SILENT: all build run
.PHONY: all build run

all: build run

build:
	arduino-cli-mega-compile . --warnings pedantic --build-property=compiler.cpp.extra_flags=\"-std=c++1z\"

run:
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose
