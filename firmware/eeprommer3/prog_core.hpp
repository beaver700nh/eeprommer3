#ifndef PFSD_CORE_HPP
#define PFSD_CORE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "ad_array.hpp"
#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

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
    ERR_FILE,    // Unable to open file on SD card
    ERR_VERIFY,  // Verification failed (expectation != reality)
    ERR_MEMORY,  // Memory allocator returned null
  };

  // These functions return `Status`es.
  typedef Status (ProgrammerBaseCore::*Func)();

  Status read();
  Status write();

protected:
  Status verify(uint16_t addr, void *data, uint16_t len = 0);

  TftCtrl &m_tft;
  TouchCtrl &m_tch;
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
};

#define ADD_RW_CORE_CLASS_DECLARATION(name) class Programmer##name##Core : public ProgrammerBaseCore

#define ADD_RW_CORE_CLASS_BODY_NO_CTOR(name) \
  public: \
    Status read(); \
    Status write(); \
  \
  private: \
    Status verify(uint16_t addr, void *data);

#define ADD_RW_CORE_CLASS_BODY(name) \
  public: \
    Programmer##name##Core(TYPED_CONTROLLERS) : ProgrammerBaseCore(CONTROLLERS) {}; \
  \
  ADD_RW_CORE_CLASS_BODY_NO_CTOR(name)

// Manipulates a single byte at a time
ADD_RW_CORE_CLASS_DECLARATION(Byte) {
ADD_RW_CORE_CLASS_BODY(Byte)
};

// Reads files to and from EEPROM
ADD_RW_CORE_CLASS_DECLARATION(File) {
public:
  ProgrammerFileCore(TYPED_CONTROLLERS);

ADD_RW_CORE_CLASS_BODY_NO_CTOR(File)

private:
  enum FileType : int8_t {NO_FILE = -1, SD_CARD_FILE, SERIAL_FILE};

  FileType get_file_type();

  // This enum is used to tell which file I/O methods are available.
  enum AvailableFileIO : uint8_t {
    NOT_AVAIL    = 0x00,
    OVER_SD_CARD = 0x01,
    OVER_SERIAL  = 0x02,
  };

  uint8_t m_available_file_io = AvailableFileIO::NOT_AVAIL;

  /******************************** SD FUNCTIONS ********************************/

  Status sd_read();
  Status sd_write();

  // Returns true if everything is fine, false if there were errors
  // Writes file name (limited to `len` chars) into `fname`; writes status into `res`
  bool get_file_to_write_from(char *fname, uint8_t len, Status *res);

  /******************************** SERIAL FUNCTIONS ********************************/ // - TODO

  Status serial_read();
  Status serial_write();
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

  // Contains info for customizing a byte's representation
  struct ByteRepr {
    uint8_t offset; // X-offset of byte on screen
    char text[3];   // Text representation of byte
    uint16_t color; // Color of byte
  };

  // Returns a `ByteRepr` for a given byte, tells how to format it
  typedef ByteRepr (*ByteReprFunc)(uint8_t input_byte);

  // Helper that shows `data` on screen, assumes `addr1` <= `addr2`
  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr);

  // Helper function of show_range() that shows only one page of data from a large buffer
  void show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, ByteReprFunc repr, uint8_t cur_page, uint8_t max_page);

  // Helper that stores `data` to a file on EEPROM
  void store_file(uint8_t *data, uint16_t len);

  // Hex mode shows the data as raw hexadecimal values in white
  static inline ByteRepr multi_byte_repr_hex(uint8_t input_byte) {
    ByteRepr repr;

    repr.offset = 0;
    repr.color = TftColor::WHITE;
    sprintf(repr.text, "%02X", input_byte);

    return repr;
  }

  // Chars mode shows the data as printable characters; white character if printable, gray "?" if not
  static inline ByteRepr multi_byte_repr_chars(uint8_t input_byte) {
    ByteRepr repr;
    
    repr.offset = 3;
    repr.color = (isprint((char) input_byte) ? TftColor::WHITE : TftColor::GRAY);
    sprintf(repr.text, "%c", (isprint((char) input_byte) ? (char) input_byte : '?'));

    return repr;
  }

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

  Status nop();

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
  };

  void do_debug_action(DebugAction action);
  void monitor_data_bus();
};

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?"); \
  m_tft.fillScreen(TftColor::BLACK); \
  \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

#endif
