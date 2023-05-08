#ifndef PROG_CORE_HPP
#define PROG_CORE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "ad_array.hpp"
#include "eeprom.hpp"
#include "file.hpp"
#include "gui.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "touch.hpp"

#define ADD_RWV_METHODS                               \
  public:                                             \
    static Status read();                             \
    static Status write();                            \
                                                      \
  protected:                                          \
    static Status verify(uint16_t addr, void *data);

/*
 * ProgrammerBaseCore is an ABC that contains functions for manipulating
 * EEPROM (EepromCtrl ee) in a way which is defined in the child classes.
 */
class ProgrammerBaseCore {
public:
  // These status codes are returned by `Func`s.
  enum Status : uint8_t {
    OK,           // There were no errors
    ERR_INVALID,  // Attempted to perform an invalid action
    ERR_FILE,     // Unable to open file
    ERR_VERIFY,   // Verification failed (expectation != reality)
    ERR_MEMORY,   // Memory allocator returned null
  };

  typedef Status (*Func)();

  static Status nop();
};

/*************************************************/
/******** Start of ProgrammerCore Classes ********/
/*************************************************/

/*** Manipulates a single byte at a time ***/
class ProgrammerByteCore : public ProgrammerBaseCore {
  ADD_RWV_METHODS
};

/*** Reads files to and from EEPROM ***/
class ProgrammerFileCore : public ProgrammerBaseCore {
  ADD_RWV_METHODS

private:
  static void read_operation_core(FileCtrl *file);

  static bool write_from_file(FileCtrl *file, uint16_t addr);
  static void write_operation_core(FileCtrl *file, uint16_t addr);
};

// Manipulates one 6502 jump vector at a time (NMI, RESET, IRQ)
class ProgrammerVectorCore : public ProgrammerBaseCore {
  ADD_RWV_METHODS
};

// Reads ranges and writes arrays of pairs
class ProgrammerMultiCore : public ProgrammerBaseCore {
  ADD_RWV_METHODS

private:
  /******************************** READ RANGE HELPERS ********************************/

  static void read_operation_core(uint8_t *data, uint16_t addr1, uint16_t addr2);

  static Status handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2, Gui::MenuChoice *menu);  // Shows read data to user nicely

  static Status store_file(uint8_t *data, uint16_t len);  // Stores data to SD card
  static void store_file_operation_core(uint8_t *data, uint16_t len, FileCtrl *file);

  /******************************** WRITE RANGE HELPERS ********************************/

  static void write_operation_core(AddrDataArray *buf);
};

// Miscellaneous other functions
class ProgrammerOtherCore : public ProgrammerBaseCore {
public:
  static Status paint();
  static Status debug();

  static Status about();
  static Status restart();

private:
  // Helper functions of debug()

  enum DebugAction : uint8_t {
    DISABLE_WRITE,        // Set !WE high (disable)
    ENABLE_WRITE,         // Set !WE low (enable)
    SET_ADDR_BUS_AND_OE,  // Set 16 bits: 15 = !OE, 14-0 = address
    READ_DATA_BUS,        // Read 8 bits from data bus
    WRITE_DATA_BUS,       // Write 8 bits to data bus
    SET_DATA_DIR,         // Set data bus as input or output
    MONITOR_DATA_BUS,     // Poll and show data bus at an interval
    PRINT_CHARSET,        // Print TFT driver's charset, 0x00-0xFF
    SHOW_COLORS,          // Show all colors supported by program
    ACTION_AUX1,          // Customizable auxiliary action
    ACTION_AUX2,          // Customizable auxiliary action
  };

  static void do_debug_action(DebugAction action);
  static void show_data_bus();
  static void monitor_data_bus();
  static void set_data_dir();
  static void debug_action_aux1();
  static void debug_action_aux2();
};

#undef ADD_RWV_METHODS

#endif
