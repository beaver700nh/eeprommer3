.SHELL := /bin/bash

AVR_BIN := /usr/bin
PRINTF := /usr/bin/printf

PROJECT_NAME := eeprommer3
BIN_OUTPUT := build/bin

PORT := /dev/ttyACM0

export PROJECT_NAME := $(PROJECT_NAME)
export ARDUINO_DIR := /usr/share/arduino/hardware/archlinux-arduino/avr
export ARDUINO_DIR_3RDPARTY := ~/Arduino
export MMCU_NAME := atmega2560
export MMCU_ARCH := avr6
export MMCU_VARIANT := mega
export MMCU_FREQ := 16000000

define PrintFancy
	$(PRINTF) "\x1b[1m***\n*** %s\n***\x1b[0m\n" $(1)
endef

define PrintSimple
	$(PRINTF) "\x1b[1m%s\x1b[0m" $(1)
endef

define WaitPlugIn
	while [ ! -e $(PORT) ]; do
		$(call PrintSimple, "Please plug in your controller board. (Listening on $(PORT)...)")
		echo
		sleep 2
	done
endef

.PHONY: all
all: build run

.PHONY: build
build: compile upload

.PHONY: go
go: upload run

.PHONY: compile
compile:
	@$(call PrintFancy, "Compiling...")
	(mkdir -p build; cd build; cmake ..; make -j4)

.PHONY: upload
upload:
	@$(call PrintFancy, "Uploading...")
	@$(call WaitPlugIn)
	$(AVR_BIN)/avrdude -v -c wiring -p $(MMCU_NAME) -P $(PORT) -b 115200 -D -U flash:w:$(BIN_OUTPUT)/$(PROJECT_NAME).hex:i

.PHONY: run
run:
	@$(call PrintFancy, "Launching serial monitor picocom... [115200baud:8N1]")
	picocom --baud=115200 --databits=8 --parity=n --stopbits=1 --flow=n --imap="lfcrlf" $(PORT)

.PHONY: clean
clean:
	@$(call PrintFancy, "Removing build files...")
	if [ $(level) -gt 0 ]; then
		@$(call PrintSimple, "- The final ELF and related files")
		echo
		rm build/bin/*
	fi
	if [ $(level) -gt 1 ]; then
		@$(call PrintSimple, "- Library compiled archives")
		echo
		rm build/lib/lib*.lib*.a
	fi
	if [ $(level) -gt 2 ]; then
		@$(call PrintSimple, "- Core compiled archive")
		echo
		rm build/lib/lib*.core.a
	fi
	if [ $(level) -gt 3 ]; then
		@$(call PrintSimple, "- The entire build directory")
		echo
		rm -r build
	fi

.ONESHELL: new
.PHONY: new
new:
	@$(call PrintSimple, "Name of cpp/hpp pair to create? ")
	read name
	upper_name=$$(echo $$name | tr "[:lower:]" "[:upper:]")
	cat file_template/hpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g" >$$name.hpp
	cat file_template/cpp | sed "s/<%upper_name%>/$$upper_name/g" | sed "s/<%name%>/$$name/g" >$$name.cpp

.ONESHELL: del
.PHONY: del
del:
	@$(call PrintSimple, "Name of cpp/hpp pair to delete? ")
	read name
	rm $$name.cpp
	rm $$name.hpp

.ONESHELL: search
.PHONY: search
search:
	result=`grep -Gn --color=always '$(PATTERN)' src/*.?pp`
	
	if [ $$? = 1 ]; then
		@$(call PrintFancy, "No matches found.")
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

.ONESHELL: lsyms
.PHONY: lsyms
lsyms:
	@$(call PrintFancy, "Listing largest symbols in PROJECT_NAME")
	
	result=`$(AVR_BIN)/avr-nm -C -r -t d -f sysv --size-sort $(BIN_OUTPUT)/$(PROJECT_NAME).elf | grep -i ' [dbv] '`
	
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

.PHONY: lsegs
lsegs:
	@$(call PrintFancy, "Listing sizes of static segments")
	$(AVR_BIN)/avr-size $(BIN_OUTPUT)/$(PROJECT_NAME).elf

.PHONY: plmap
plmap:
	@$(call PrintFancy, "Printing linker .map file")
	cat $(BIN_OUTPUT)/$(PROJECT_NAME).elf.map

.PHONY: objdump
objdump:
	@$(call PrintFancy, "Running avr-objdump on PROJECT_NAME")
	$(AVR_BIN)/avr-objdump -t -C gnu-v3 $(BIN_OUTPUT)/$(PROJECT_NAME).elf
