.SILENT: all build compile clean run
.PHONY: all build compile clean run

all: compile
build: compile

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

compile:
	echo "Output path: $(BIN_PATH)"
	echo "$(BIN_PATH)" >binpath

	clang++ $(CXX_FLAGS) ../src/*.cpp -o $(BIN_PATH) $(CXX_LIBS)

clean:
	rm -i ../src/*~

run:
	echo "Running file: $(BIN_PATH)"
	$(BIN_PATH)
