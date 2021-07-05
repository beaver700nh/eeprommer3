.SILENT: all build compile clean
.PHONY: all build compile clean

all: compile
build: compile

VERSION := "\"0.0.7-dev\""

CXX_FLAGS := `wx-config --cxxflags` -Wpedantic -std=c++17
CXX_LIBS  := `wx-config --libs` -lserialport -DEEPROMMER3_VERSION=$(VERSION)

ifdef ARCHTT

ifdef FULL_PATH
BIN_PATH := $(ARCHTT)
else
BIN_PATH := "binaries/$(ARCHTT)/"
endif

else

TEMP := $(file < binpath)

ifneq ($(patsubst %/,,$(TEMP)),)
BIN_PATH := "binaries/Linux/x86_64/"
else
BIN_PATH := $(TEMP)
endif

endif

compile:
	echo "Output path: $(BIN_PATH)"
	echo "$(BIN_PATH)" >binpath

	clang++ $(CXX_FLAGS) ../src/*.cpp -o "$(BIN_PATH)eeprommer3" $(CXX_LIBS)

clean:
	rm -i ../src/*~
