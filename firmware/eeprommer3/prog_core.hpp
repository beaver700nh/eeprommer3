#ifndef PFSD_CORE_HPP
#define PFSD_CORE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "ad_array.hpp"
#include "eeprom.hpp"
#include "file.hpp"
#include "gui.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "touch.hpp"

#define ADD_RW_CORE_CLASS_DECLARATION(name) class Programmer##name##Core : public ProgrammerBaseCore

#define ADD_RW_CORE_CLASS_BODY_NO_CTOR(name) \
  public: \
    Status read(); \
    Status write(); \
  \
  protected: \
    Status verify(uint16_t addr, void *data);

#define ADD_RW_CORE_CLASS_BODY(name) \
  public: \
    Programmer##name##Core(TYPED_CONTROLLERS) : ProgrammerBaseCore(CONTROLLERS) {}; \
  \
  ADD_RW_CORE_CLASS_BODY_NO_CTOR(name)

/*
 * ProgrammerBaseCore is an ABC that contains functions for manipulating EEPROM
 * (EepromCtrl &ee) in a way which is defined in the child classes.
 */
class ProgrammerBaseCore {
public:
  ProgrammerBaseCore(TYPED_CONTROLLERS) : INIT_LIST_CONTROLLERS {};

  // These status codes are returned by `Func`s.
  enum Status : uint8_t {
    OK,          // There were no errors
    ERR_INVALID, // Attempted to perform an invalid action
    ERR_FILE,    // Unable to open file
    ERR_VERIFY,  // Verification failed (expectation != reality)
    ERR_MEMORY,  // Memory allocator returned null
  };

  // These functions return `Status`es.
  typedef Status (ProgrammerBaseCore::*Func)();

  Status nop();

  TftCtrl &m_tft;
  TouchCtrl &m_tch;
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
};

/*************************************************/
/******** Start of ProgrammerCore Classes ********/
/*************************************************/

/*** Manipulates a single byte at a time ***/
ADD_RW_CORE_CLASS_DECLARATION(Byte) {
ADD_RW_CORE_CLASS_BODY(Byte)
};

/*** Reads files to and from EEPROM ***/
ADD_RW_CORE_CLASS_DECLARATION(File) {
public:
  ProgrammerFileCore(TYPED_CONTROLLERS);

ADD_RW_CORE_CLASS_BODY_NO_CTOR(File)

private:
  // Performs some checks on file, returns resulting status.
  Status check_valid(FileCtrl *file);

  Status read_to_fsys(const char *fpath, FileSystem fsys);
  Status write_from_fsys(const char *fpath, FileSystem fsys, uint16_t addr);

  void do_read_operation(FileCtrl *file);
  void do_write_operation(FileCtrl *file, uint16_t addr);

  // Gets file path from file system `fsys`, writes at most `len` chars into `out`, writes status into `res`
  // Returns true if successful, false otherwise
  bool get_file_to_write_from(char *out, uint8_t len, Status *res, FileSystem fsys);

  bool sd_get_file_to_write_from(char *out, uint8_t len, Status *res);
};

// Manipulates one 6502 jump vector at a time (NMI, RESET, IRQ)
ADD_RW_CORE_CLASS_DECLARATION(Vector) {
ADD_RW_CORE_CLASS_BODY(Vector)
};

// Reads ranges and writes arrays of pairs
ADD_RW_CORE_CLASS_DECLARATION(Multi) {
ADD_RW_CORE_CLASS_BODY(Multi)

private:
  /******************************** READ RANGE HELPERS ********************************/

  void read_with_progress_bar(uint8_t *data, uint16_t addr1, uint16_t addr2);

  // Contains info for customizing a byte's representation
  struct ByteRepr {
    uint8_t offset; // X-offset of byte on screen
    char text[3];   // Text representation of byte
    uint16_t color; // Color of byte
  };

  // Returns a `ByteRepr` for a given byte, tells how to format it
  typedef ByteRepr (*ByteReprFunc)(uint8_t input_byte);

  void handle_data(uint8_t *data, uint16_t addr1, uint16_t addr2); // In a user-friendly way, shows read data to user

  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr);                                    // Show whole range on TFT
  void show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr, uint8_t cur_page, uint8_t max_page); // One page at a time

  void draw_page_axis_labels(); // Draw markers: 00, 10, ..., F0; 00, 01, ..., 0F

  void store_file(uint8_t *data, uint16_t len); // Stores data to SD card

  // Hex mode shows the data as raw hexadecimal values in white
  static inline ByteRepr multi_byte_repr_hex(uint8_t input_byte);

  // Chars mode shows the data as printable characters; white character if printable, gray "?" if not
  static inline ByteRepr multi_byte_repr_chars(uint8_t input_byte);

  /******************************** WRITE RANGE HELPERS ********************************/

  // Helper that draws the pairs to be written to EEPROM
  void draw_pairs(
    uint16_t margin_l, uint16_t margin_r, uint16_t margin_u, uint16_t margin_d,
    uint16_t height, uint16_t padding, uint8_t n, uint8_t offset, AddrDataArray &buf, TftMenu &del_btns
  );

  // Helper that polls `menu` and reacts to it: deletes/adds buttons, scrolls menu, and writes to EEPROM as requested
  bool poll_menus_and_react(
    TftMenu &menu, TftMenu &del_btns, AddrDataArray *buf, uint16_t *scroll, const uint16_t max_scroll
  );

  // Helper function of poll_menus_and_react() that requests and adds a pair to `buf`
  void add_pair_from_user(AddrDataArray *buf);
};

// Miscellaneous other functions
class ProgrammerOtherCore : public ProgrammerBaseCore {
public:
  ProgrammerOtherCore(TYPED_CONTROLLERS) : ProgrammerBaseCore(CONTROLLERS) {};

  Status paint();
  Status debug();

  Status about();

private:
  // Helper functions of debug()

  enum DebugAction : uint8_t {
    DISABLE_WRITE,       // Set !WE high (disable)
    ENABLE_WRITE,        // Set !WE low (enable)
    SET_ADDR_BUS_AND_OE, // Set 16 bits: 15 = !OE, 14-0 = address
    READ_DATA_BUS,       // Read 8 bits from data bus
    WRITE_DATA_BUS,      // Write 8 bits to data bus
    SET_DATA_DIR,        // Set data bus as input or output
    MONITOR_DATA_BUS,    // Poll and show data bus at an interval
    PRINT_CHARSET,       // Print TFT driver's charset, 0x00-0xFF
    SHOW_COLORS,         // Show all colors supported by program
    ACTION_AUX1,         // Customizable auxiliary action
    ACTION_AUX2,         // Customizable auxiliary action
  };

  void do_debug_action(DebugAction action);
  void monitor_data_bus();
  void debug_action_aux1();
  void debug_action_aux2();
};

#undef ADD_RW_CORE_CLASS_DECLARATION
#undef ADD_RW_CORE_CLASS_BODY
#undef ADD_RW_CORE_CLASS_BODY_NO_CTOR

// Helper function to get an address; same as ask_val<uint16_t> but has built-in validation
uint16_t ask_addr(TftCtrl &tft, TouchCtrl &tch, const char *prompt);

// Helper function to choose a file system out of all the ones that are detected
FileSystem ask_fsys(TftCtrl &tft, TouchCtrl &tch, const char *prompt, SdCtrl &sd);

#endif
