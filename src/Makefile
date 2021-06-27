.SILENT: all build compile run
.PHONY: all build compile run

all: compile run
build: compile

compile:
	clang++ `wx-config --cxxflags` -Wall *.cpp -o eeprommer3 `wx-config --libs`

run:
	./eeprommer3
