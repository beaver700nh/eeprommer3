all: build upload run

build:
	arduino-cli compile . --fqbn arduino:avr:mega --verbose

upload:
	arduino-cli upload . --fqbn arduino:avr:mega --port /dev/ttyACM0 --verbose

run:
	picocom --baud=115200 --databits=8 --parity=n --stopbits=1 --flow=n --imap="lfcrlf" /dev/ttyACM0
