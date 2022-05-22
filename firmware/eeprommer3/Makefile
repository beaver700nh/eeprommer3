.SILENT: all build compile upload run new del search love
.PHONY: all build compile upload run new del search love

.SHELL := /bin/bash

all: compile upload run
build: compile upload
go: upload run

compile:
	arduino-cli-mega-compile . --warnings=more --build-property=compiler.cpp.extra_flags=-std=c++17 $(FLAGS)

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

.ONESHELL:
del:
	read -p "Name of cpp/hpp pair to delete? " name

	rm $$name.cpp
	rm $$name.hpp

search:
	grep -n '$(PATTERN)' *.?pp

love:
	echo "not war"
