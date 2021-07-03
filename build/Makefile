.SILENT: all build compile clean
.PHONY: all build compile clean

all: compile
build: compile

VERSION := "\"0.0.7-dev\""

CXX_FLAGS := `wx-config --cxxflags` -Wpedantic -std=c++17
CXX_LIBS  := `wx-config --libs` -lserialport -DEEPROMMER3_VERSION=$(VERSION)

BIN_PATH := "binaries/Linux/ARM/"

compile:
	clang++ $(CXX_FLAGS) ../src/*.cpp -o "$(BIN_PATH)eeprommer3" $(CXX_LIBS)

clean:
	rm -i ../src/*~
