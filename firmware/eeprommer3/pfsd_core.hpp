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
 * ProgrammerFromSdBaseCore is an ABC that contains some functions for manipulating the EEPROM
 * (EepromCtrl &ee) in a certain way. This certain way is defined in the child classes.
 */
class ProgrammerFromSdBaseCore {
public:
  ProgrammerFromSdBaseCore(TYPED_CONTROLLERS) : INIT_LIST_CONTROLLERS {};

  // These status codes are returned by `Func`s.
  enum Status : uint8_t {
    OK,          // There were no errors
    ERR_INVALID, // Attempted to perform an invalid action
    ERR_FILE,    // Unable to open file on SD card
    ERR_VERIFY,  // Verification failed (expectation != reality)
    ERR_MEMORY,  // Memory allocator returned null
  };

  // These functions return `Status`es.
  typedef Status (ProgrammerFromSdBaseCore::*Func)();

  Status read();
  Status write();

protected:
  Status verify(uint16_t addr, void *data, uint16_t len = 0);

  TftCtrl &m_tft;
  TouchCtrl &m_tch;
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
};

#define ADD_RW_CORE_CLASS_DECLARATION(name) class ProgrammerFromSd##name##Core : public ProgrammerFromSdBaseCore
#define ADD_RW_CORE_CLASS_BODY(name) \
  public: \
    ProgrammerFromSd##name##Core(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {}; \
  \
    Status read(); \
    Status write(); \
  \
  private: \
    Status verify(uint16_t addr, void *data);

// Manipulates a single byte at a time
ADD_RW_CORE_CLASS_DECLARATION(Byte) {
ADD_RW_CORE_CLASS_BODY(Byte)
};

// Reads files to and from EEPROM
ADD_RW_CORE_CLASS_DECLARATION(File) {
ADD_RW_CORE_CLASS_BODY(File)

private:
  // Returns true if everything is fine, false if there were errors
  // Writes file name (limited to `len` chars) into `fname`; writes status into `res`
  bool get_file_to_write_from(char *fname, uint8_t len, Status *res);
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

  typedef void (*MultiByteShowRangeByteReprFunc)(uint8_t input_byte, uint8_t *offset, char *text_repr, uint16_t *color);

  // Helper that shows `data` on screen, assumes `addr1` <= `addr2`
  void show_range(uint8_t *data, uint16_t addr1, uint16_t addr2, MultiByteShowRangeByteReprFunc repr);

  // Helper function of show_range() that shows only one page of data from a large buffer
  void show_page(uint8_t *data, uint16_t addr1, uint16_t addr2, MultiByteShowRangeByteReprFunc repr, uint8_t cur_page, uint8_t max_page);

  // Helper that stores `data` to a file on EEPROM
  void store_file(uint8_t *data, uint16_t len);

  // Hex mode shows the data as raw hexadecimal values in white
  static inline void multi_byte_repr_hex(uint8_t input_byte, uint8_t *offset, char *text_repr, uint16_t *color) {
    *offset = 0;
    *color = TftColor::WHITE;
    sprintf(text_repr, "%02X", input_byte);
  }

  // Chars mode shows the data as printable characters; white character if printable, gray "?" if not
  static inline void multi_byte_repr_chars(uint8_t input_byte, uint8_t *offset, char *text_repr, uint16_t *color) {
    *offset = 3;
    *color = (isprint((char) input_byte) ? TftColor::WHITE : TftColor::GRAY);
    sprintf(text_repr, "%c", (isprint((char) input_byte) ? (char) input_byte : '?'));
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
class ProgrammerFromSdOtherCore : public ProgrammerFromSdBaseCore {
public:
  ProgrammerFromSdOtherCore(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {};

  Status paint();
  Status debug();

  Status about();
  Status help();

  Status nop();

private:
  // Helper functions of debug()

  void do_debug_action(uint8_t action);
  void monitor_data_bus();
};

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?"); \
  m_tft.fillScreen(TftColor::BLACK); \
  \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

#endif
