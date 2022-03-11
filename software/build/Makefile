.SILENT: all build clean run
.PHONY: all build clean run

all: build

VERSION := "\"0.0.7-dev\""

CXX_FLAGS := `wx-config --cxxflags` -Wpedantic -std=c++17
CXX_LIBS  := `wx-config --libs` -lserialport -DEEPROMMER3_VERSION=$(VERSION)

EXEC_NAME := eeprommer3

ifdef ARCHTT

ifdef FULL_PATH
BIN_PATH := $(ARCHTT)
else
BIN_PATH := "binaries/$(ARCHTT)/$(EXEC_NAME)"
endif

else

TEMP := $(file < binpath)

ifneq ($(patsubst %$(EXEC_NAME),,$(TEMP)),)
BIN_PATH := "binaries/Linux/x86_64/$(EXEC_NAME)"
else
BIN_PATH := $(TEMP)
endif

endif

.ONESHELL:
SHELL := /bin/bash

build:
	echo "Output path: $(BIN_PATH)"
	echo "$(BIN_PATH)" >binpath

	if g++ $(CXX_FLAGS) ../src/*.cpp -o $(BIN_PATH) $(CXX_LIBS); then
		echo "Done compiling - PASSED!"
		exit 0
	else
		echo "Done compiling - FAILED!"
		exit 1
	fi

clean:
	echo "Cleaning temporary~ files"
	rm -i ../src/*~

run:
	echo "Running file: $(BIN_PATH)"
	$(BIN_PATH)
