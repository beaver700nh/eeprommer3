cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME $ENV{PROJECT_NAME})

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

set(
  COMPILE_OPTIONS
  -g
  -CC
  -mmcu=$ENV{MMCU_NAME}
  -DARDUINO=10607
  -DARDUINO_AVR_MEGA2560
  -DARDUINO_ARCH_AVR
  -D__AVR__
  -D__AVR_ATmega2560__
  -DF_CPU=$ENV{MMCU_FREQ}
  -Wall
  -Wextra
  -Wno-error=narrowing
  -fpermissive
  -fno-exceptions
  -ffunction-sections
  -fdata-sections
  -fno-threadsafe-statics
  #  -flto
  -Os
)

set(
  COMPILE_INCLUDES
  $ENV{ARDUINO_DIR}/cores/arduino
  $ENV{ARDUINO_DIR}/variants/mega
)

project(${PROJECT_NAME})

link_directories(
  /usr/lib/gcc/avr/${CMAKE_CXX_COMPILER_VERSION}/$ENV{MMCU_ARCH}
  /usr/avr/lib/$ENV{MMCU_ARCH}
)

################################################################
## Core
################################################################

file(
  GLOB CORE_SOURCES CONFIGURE_DEPENDS
  "$ENV{ARDUINO_DIR}/cores/arduino/*.S"
  "$ENV{ARDUINO_DIR}/cores/arduino/*.c"
  "$ENV{ARDUINO_DIR}/cores/arduino/*.cpp"
)

add_library(
  ${PROJECT_NAME}.core
  STATIC
  ${CORE_SOURCES}
)

target_include_directories(
  ${PROJECT_NAME}.core
  PUBLIC
  ${COMPILE_INCLUDES}
)

target_compile_options(
  ${PROJECT_NAME}.core
  PUBLIC
  ${COMPILE_OPTIONS}
)

################################################################
## Libraries
################################################################

add_library(
  ${PROJECT_NAME}.libSPI
  STATIC
  $ENV{ARDUINO_DIR}/libraries/SPI/src/SPI.cpp
)

target_include_directories(
  ${PROJECT_NAME}.libSPI
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR}/libraries/SPI/src
)

target_compile_options(
  ${PROJECT_NAME}.libSPI
  PUBLIC
  ${COMPILE_OPTIONS}
)

add_library(
  ${PROJECT_NAME}.libWire
  STATIC
  $ENV{ARDUINO_DIR}/libraries/Wire/src/Wire.cpp
  $ENV{ARDUINO_DIR}/libraries/Wire/src/utility/twi.c
)

target_include_directories(
  ${PROJECT_NAME}.libWire
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR}/libraries/Wire/src
)

target_compile_options(
  ${PROJECT_NAME}.libWire
  PUBLIC
  ${COMPILE_OPTIONS}
)

add_library(
  ${PROJECT_NAME}.libSD
  STATIC
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/SD.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/File.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/Sd2Card.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/SdFile.cpp
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src/utility/SdVolume.cpp
)

target_include_directories(
  ${PROJECT_NAME}.libSD
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/SD/src
)

target_compile_options(
  ${PROJECT_NAME}.libSD
  PUBLIC
  ${COMPILE_OPTIONS}
)

add_library(
  ${PROJECT_NAME}.libMCUFRIEND_kbv
  STATIC
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/MCUFRIEND_kbv/MCUFRIEND_kbv.cpp
)

target_include_directories(
  ${PROJECT_NAME}.libMCUFRIEND_kbv
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR}/libraries/SPI/src
  $ENV{ARDUINO_DIR}/libraries/Wire/src
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/MCUFRIEND_kbv/extras/unused
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_GFX_Library
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_BusIO
)

target_compile_options(
  ${PROJECT_NAME}.libMCUFRIEND_kbv
  PUBLIC
  ${COMPILE_OPTIONS}
)

add_library(
  ${PROJECT_NAME}.libAdafruit_GFX_Library
  STATIC
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_GFX_Library/Adafruit_GFX.cpp
)

target_include_directories(
  ${PROJECT_NAME}.libAdafruit_GFX_Library
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR}/libraries/SPI/src
  $ENV{ARDUINO_DIR}/libraries/Wire/src
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_GFX_Library
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_BusIO
)

target_compile_options(
  ${PROJECT_NAME}.libAdafruit_GFX_Library
  PUBLIC
  ${COMPILE_OPTIONS}
)

add_library(
  ${PROJECT_NAME}.libAdafruit_TouchScreen
  STATIC
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_TouchScreen/TouchScreen.cpp
)

target_include_directories(
  ${PROJECT_NAME}.libAdafruit_TouchScreen
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries/Adafruit_TouchScreen
)

target_compile_options(
  ${PROJECT_NAME}.libAdafruit_TouchScreen
  PUBLIC
  ${COMPILE_OPTIONS}
)

################################################################
## Executable
################################################################

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
)

add_custom_command(
  TARGET ${PROJECT_NAME}.elf
  POST_BUILD
  COMMAND avr-objcopy
  ARGS -O ihex -R .eeprom ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.hex
  COMMAND avr-size
  ARGS -A ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf
)

target_include_directories(
  ${PROJECT_NAME}.elf
  PUBLIC
  ${COMPILE_INCLUDES}
  $ENV{ARDUINO_DIR}/libraries
  $ENV{ARDUINO_DIR_3RDPARTY}/libraries
)

target_compile_options(
  ${PROJECT_NAME}.elf
  PUBLIC
  ${COMPILE_OPTIONS}
)

target_link_libraries(
  ${PROJECT_NAME}.elf
  PUBLIC
  ${PROJECT_NAME}.core
  ${PROJECT_NAME}.libSPI
  ${PROJECT_NAME}.libWire
  ${PROJECT_NAME}.libSD
  ${PROJECT_NAME}.libMCUFRIEND_kbv
  ${PROJECT_NAME}.libAdafruit_GFX_Library
  ${PROJECT_NAME}.libAdafruit_TouchScreen
  -g
  -mmcu=$ENV{MMCU_NAME}
  -fuse-linker-plugin
  -flto
  -O1
  -Wl,-m$ENV{MMCU_ARCH},--gc-sections,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY},--defsym,__heap_start=0x808000,--defsym,__heap_end=0x80DFFF
)
