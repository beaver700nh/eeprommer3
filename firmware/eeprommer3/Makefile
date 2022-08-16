.SILENT: all build compile upload run new del search lsyms lsegs plmap objdump love
.PHONY: all build compile upload run new del search lsyms lsegs plmap objdump love

.SHELL := /bin/bash

PRINTF := /usr/bin/printf

AVR_NM      := ~/.arduino15/packages/arduino/tools/avr-gcc/*/bin/avr-nm
AVR_SIZE    := ~/.arduino15/packages/arduino/tools/avr-gcc/*/bin/avr-size
AVR_OBJDUMP := ~/.arduino15/packages/arduino/tools/avr-gcc/*/bin/avr-objdump

TMP_BUILD_DIR := /tmp/arduino-sketch-*/

PROGRAM := eeprommer3.ino

BUILD_FLAGS := '-std=c++17\ -fshort-enums'

all: compile upload run
build: compile upload
go: upload run

define PrintFancy
	$(PRINTF) "\x1b[1m***\n*** %s\n***\x1b[0m\n" $(1)
endef

define PrintSimple
	$(PRINTF) "\x1b[1m%s\x1b[0m" $(1)
endef

compile:
	$(call PrintFancy, "Compiling...")
	arduino-cli-mega-compile . --warnings=more '--build-property=compiler.cpp.extra_flags=$(BUILD_FLAGS)' $(FLAGS)

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

.ONESHELL:
lsyms:
	$(call PrintFancy, "Listing largest symbols in program")

	result=`$(AVR_NM) -C -r -t d -f sysv --size-sort $(TMP_BUILD_DIR)/$(PROGRAM).elf | grep -i ' [dbv] '`

	printf "%s" "$$result" | while IFS="\n" read -r line; do
		IFS="|"
		set -- $$line

		padding=$$((100-$${#1}))
		spaces=`perl -E "say \" \" x $$padding"`
		$(PRINTF) "\x1b[1m%s\x1b[0m%s" "$$1" "$$spaces"

		for col in 2 3 4 5 6 7; do
			text=`eval echo "\$$"$$col`

			if [ $$col = 5 ]; then
				$(PRINTF) " | \x1b[1m$$text\x1b[0m"
			else
				echo -n " | $$text"
			fi
		done

		echo
	done

lsegs:
	$(call PrintFancy, "Listing sizes of static segments")
	$(AVR_SIZE) $(TMP_BUILD_DIR)/$(PROGRAM).elf

plmap:
	$(call PrintFancy, "Printing linker .map file")
	cat $(TMP_BUILD_DIR)/$(PROGRAM).map

objdump:
	$(call PrintFancy, "Running avr-objdump on program")
	$(AVR_OBJDUMP) -t $(TMP_BUILD_DIR)/$(PROGRAM).elf

love:
	$(call PrintFancy, "... not war!")
