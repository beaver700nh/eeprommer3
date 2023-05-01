cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME $ENV{PROJECT_NAME})

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

project(${PROJECT_NAME})

link_directories(
  /usr/lib/gcc/avr/${CMAKE_CXX_COMPILER_VERSION}/$ENV{MMCU_ARCH}
  /usr/avr/lib/$ENV{MMCU_ARCH}
)

add_executable(
  ${PROJECT_NAME}.elf
  src/eeprommer3.cpp
  src/ad_array.cpp
  src/comm.cpp
  src/dialog.cpp
  src/eeprom.cpp
  src/error.cpp
  src/file.cpp
  src/gui.cpp
  src/new_delete.cpp
  src/prog.cpp
  src/prog_core.cpp
  src/sd.cpp
  src/strfmt.cpp
  src/tft.cpp
  src/tft_calc.cpp
  src/tft_util.cpp
  src/touch.cpp
  src/util.cpp
  src/vector.cpp
  src/xram.cpp
  $ENV{ARDUINO_DIR}/cores/arduino/HardwareSerial.cpp
  $ENV{ARDUINO_DIR}/cores/arduino/HardwareSerial0.cpp
  $ENV{ARDUINO_DIR}/cores/arduino/Print.cpp
  $ENV{ARDUINO_DIR}/cores/arduino/WMath.cpp
  $ENV{ARDUINO_DIR}/cores/arduino/hooks.c
  $ENV{ARDUINO_DIR}/cores/arduino/wiring.c
  $ENV{ARDUINO_DIR}/cores/arduino/wiring_analog.c
  $ENV{ARDUINO_DIR}/cores/arduino/wiring_digital.c
  $ENV{ARDUINO_DIR}/libraries/SPI/src/SPI.cpp
  $ENV{ARDUINO_DIR}/libraries/Wire/src/Wire.cpp
  $ENV{ARDUINO_DIR}/libraries/Wire/src/utility/twi.c
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/SD.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/File.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/Sd2Card.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/SdFile.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/SdVolume.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/MCUFRIEND_kbv/MCUFRIEND_kbv.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/MCUFRIEND_kbv/extras/unused/ADA_GFX_kbv.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_TouchScreen/TouchScreen.cpp
)

add_custom_command(
  TARGET ${PROJECT_NAME}.elf
  POST_BUILD
  COMMAND avr-objcopy
  ARGS -O ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.hex
)

target_include_directories(
  ${PROJECT_NAME}.elf
  PUBLIC
  $ENV{ARDUINO_DIR}/cores/arduino
  $ENV{ARDUINO_DIR}/variants/$ENV{MMCU_VARIANT}
  $ENV{ARDUINO_DIR}/libraries
  $ENV{ARDUINO_DIR}/libraries/SPI/src
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/MCUFRIEND_kbv/extras/unused
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_GFX_Library
)

target_compile_options(
  ${PROJECT_NAME}.elf
  PUBLIC
  -mmcu=$ENV{MMCU_NAME}
  -DARDUINO=100
  -D__AVR__
  -D$ENV{MMCU_MACRO}
  -DF_CPU=$ENV{MMCU_FREQ}
  -Wall
  -Wextra
  -fshort-enums
  -fno-threadsafe-statics
  -O1
)

target_link_libraries(
  ${PROJECT_NAME}.elf
  PUBLIC
  -Wl,-m$ENV{MMCU_ARCH},--gc-sections,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY},--defsym,__heap_start=0x808000,--defsym,__heap_end=0x80DFFF
)
