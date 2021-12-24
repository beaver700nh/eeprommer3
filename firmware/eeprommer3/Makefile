.SILENT: all build run
.PHONY: all build run

all: build run

build:
	arduino-cli-mega-compile . --warnings pedantic

run:
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose
