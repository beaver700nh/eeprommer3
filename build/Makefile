.SILENT: all build compile run clean
.PHONY: all build compile run clean

all: compile
build: compile

CXX_FLAGS := `wx-config --cxxflags` -Wpedantic -std=c++17
CXX_LIBS  := `wx-config --libs` -lserialport

compile:
	clang++ $(CXX_FLAGS) ../src/*.cpp -o eeprommer3 $(CXX_LIBS)

run:
	./eeprommer3

clean:
	rm -i ../src/*~
