.SILENT: all build compile upload run new del search love
.PHONY: all build compile upload run new del search love

.SHELL := /bin/bash

all: compile upload run
build: compile upload
go: upload run

define PrintFancy
	/usr/bin/printf "\x1b[1m***\n*** %s\n***\x1b[0m\n" $(1)
endef

define PrintSimple
	/usr/bin/printf "\x1b[1m%s\x1b[0m" $(1)
endef

compile:
	$(call PrintFancy, "Compiling...")
	arduino-cli-mega-compile . --warnings=more --build-property=compiler.cpp.extra_flags=-std=c++17 $(FLAGS)

upload:
	$(call PrintFancy, "Uploading...")
	arduino-cli-mega-upload . /dev/ttyACM0 --verbose

run:
ifeq ($(SER_MON), 1)
### Map LF to CR+LF to avoid staircase printing
	$(call PrintFancy, "Launching serial monitor (picocom)... [115200baud:8N1]")
	picocom --baud=115200 --databits=8 --parity=n --stopbits=1 --flow=n --imap="lfcrlf" /dev/ttyACM0
else
	$(call PrintFancy, "Nothing to do because serial monitor is not enabled.")
endif

.ONESHELL:
new:
	$(call PrintSimple, "Name of cpp/hpp pair to create? ")
	read name
	upper_name=$$(echo $$name | tr "[:lower:]" "[:upper:]")

	hpp=$$(cat file_template/hpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g")
	cpp=$$(cat file_template/cpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g")

	printf "$$hpp" >$$name.hpp
	printf "$$cpp" >$$name.cpp

.ONESHELL:
del:
	$(call PrintSimple, "Name of cpp/hpp pair to delete? ")
	read name

	rm $$name.cpp
	rm $$name.hpp

.ONESHELL:
search:
	result=`grep -Gn --color=always '$(PATTERN)' *.?pp *.ino`

	if [ $$? = 1 ]; then
		$(call PrintFancy, "No matches found.")
		exit
	fi

### This line fixes a bug but I have no idea how
	result=`printf "%s\n " "$$result"`

	printf "%s" "$$result" | while IFS="\n" read -r line; do
		location=`printf "%s\n" "$$line" | grep -oP '^[^:]+:[^:]+:[^K]+K'`
		contents=`printf "%s\n" "$$line" | grep -oP '^[^:]+:[^:]+:[^K]+K\K.*'`
		locplain=`printf "%s\n" "$$location" | sed 's/\x1b\[[0-9]*[mK]//g'`

		padding=$$((24-$${#locplain}))
		spaces=`perl -E "say \".\" x $$padding"`

		printf "%s%s|%s\n" "$$location" "$$spaces" "$$contents"
	done

love:
	$(call PrintFancy, "... not war!")
