#ifndef UTIL_HPP
#define UTIL_HPP

#include <Arduino.h>
#include "constants.hpp"

#undef swap

// A namespace with various miscellaneous utility functions
namespace Util {
  // Swaps two variables `a` and `b` of type `T`
  template<typename T>
  void swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
  }

  // `exit_condition` is a template type
  // to get around annoying lambda errors
  template<typename Func>
  bool skippable_delay(uint32_t time, Func exit_condition) {
    auto t1 = millis();

    while (millis() - t1 < time) {
      if (exit_condition()) return true;
    }

    return false;
  }

  // Function to validate an address
  void validate_addr(uint16_t *addr);

  // Function to validate two addresses (and make sure first is not greater than second)
  void validate_addrs(uint16_t *addr1, uint16_t *addr2);

  int available_memory();
};

/*
 * Contains various strings (some reused), put here to save memory
 *
 * T_ - Title
 * P_ - Prompt
 * E_ - Error
 * W_ - Working
 * F_ - Finished
 * L_ - Label
 */
namespace Strings {
  inline const char *const T_DONE      = "Done";
  inline const char *const T_MISMATCH  = "Mismatch";
  inline const char *const T_CANCELED  = "Canceled";
  inline const char *const T_TOO_BIG   = "EEPROM Overflow";
  inline const char *const T_TOO_LONG  = "String Overflow";
  inline const char *const T_INV_FSYS  = "Invalid Filesystem";
  inline const char *const T_WMULTI    = "Write Multiple Bytes";

  inline const char *const E_CANCELED  = "Operation was canceled.";
  inline const char *const E_TOO_BIG   = "File would be too large\nto fit into EEPROM there!\nAborted.";
  inline const char *const E_TOO_LONG  = "File name was too long\nto fit in the buffer.";
  inline const char *const E_INV_FSYS  = "The selected filesystem\ndoes not exist.";

  inline const char *const P_ADDR_GEN  = "Type an address:";
  inline const char *const P_DATA_GEN  = "Type the data:";

  inline const char *const P_ADDR_FILE = "Where to write in EEPROM?";

  inline const char *const P_ADDR_VEC  = "Type new vector value:";

  inline const char *const P_ADDR_BEG  = "Type the start address:";
  inline const char *const P_ADDR_END  = "Type the end address:";

  inline const char *const P_OFILE     = "Which file to read to?";
  inline const char *const P_IFILE     = "Which file to write from?";

  inline const char *const P_STORE     = "Where to store data?";

  inline const char *const P_VERIFY    = "Verify data?";

  inline const char *const W_OFILE     = "Reading EEPROM to file...";
  inline const char *const W_IFILE     = "Writing file to EEPROM...";

  inline const char *const W_RMULTI    = "Reading bytes from EEPROM...";
  inline const char *const W_WMULTI    = "Writing bytes to EEPROM...";

  inline const char *const W_WAIT      = "Please wait...";

  inline const char *const F_READ      = "Done reading!";
  inline const char *const F_WRITE     = "Done writing!";
  inline const char *const F_VERIFY    = "Done verifying!";

  inline const char *const L_LEFT      = "\x1e";
  inline const char *const L_RIGHT     = "\x1f";
  inline const char *const L_ADD_PAIR  = "Add Pair";
  inline const char *const L_DONE      = "Done";
  inline const char *const L_CANCEL    = "Cancel";
}

#endif
