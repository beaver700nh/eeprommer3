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
    *a     = *b;
    *b     = temp;
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
};

struct Memory {
  static void calculate();
  static void repr();

  static void print_ram_analysis();

  static constexpr uint8_t NUM_TYPES = 6;

  enum Types {DATA, BSS, HEAP, FREE, STACK, TOTAL};
  enum Bords {RAM_START, BSS_START, HEAP_START, HEAP_END, STACK_PTR, RAM_END};

  static inline const char *const NAMES[NUM_TYPES] {"Data", "BSS", "Heap", "Free", "Stack", "Total"};

  static inline int32_t sizes[NUM_TYPES];
  static inline uint32_t bords[NUM_TYPES];

  static inline char repr_sizes[NUM_TYPES][26];
  static inline char repr_bords[NUM_TYPES][20];
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
 * A_ - Action
 * D_ - Debug
 * I_ - Info
 */
namespace Strings {
  inline const char *const T_DONE      = "Done";
  inline const char *const T_MISMATCH  = "Mismatch";
  inline const char *const T_CANCELED  = "Canceled";
  inline const char *const T_VALUE     = "Value";
  inline const char *const T_TOO_BIG   = "EEPROM Overflow";
  inline const char *const T_TOO_LONG  = "String Overflow";
  inline const char *const T_INV_FSYS  = "Invalid Filesystem";
  inline const char *const T_WMULTI    = "Write Multiple Bytes";
  inline const char *const T_NOT_SUPP  = "Not Supported";
  inline const char *const T_RESULT    = "Result";
  inline const char *const T_SUCCESS   = "Success";
  inline const char *const T_FAILED    = "Failed";
  inline const char *const T_ABOUT     = "About";
  inline const char *const T_EL_INFO   = "Info";
  inline const char *const T_EL_WARN   = "Warning";
  inline const char *const T_EL_ERR    = "Error";
  inline const char *const T_DEBUGS    = "Debug Tools Menu";

  inline const char *const E_CANCELED  = "Operation was canceled.";
  inline const char *const E_TOO_BIG   = "File would be too large\nto fit into EEPROM there!\nAborted.";
  inline const char *const E_TOO_LONG  = "File name was too long\nto fit in the buffer.";
  inline const char *const E_INV_FSYS  = "The selected filesystem\ndoes not exist.";
  inline const char *const E_NO_DB_MON = "Data bus monitor is not\nsupported because DEBUG_MODE\nis disabled.";

  inline const char *const P_ACTION    = "Choose an action:";
  inline const char *const P_ADDR_GEN  = "Type an address:";
  inline const char *const P_DATA_GEN  = "Type the data:";
  inline const char *const P_VAL_GEN   = "Type a value:";
  inline const char *const P_VECTOR    = "Select the vector:";
  inline const char *const P_ADDR_FILE = "Where to write in EEPROM?";
  inline const char *const P_ADDR_VEC  = "Type new vector value:";
  inline const char *const P_ADDR_BEG  = "Type the start address:";
  inline const char *const P_ADDR_END  = "Type the end address:";
  inline const char *const P_FILE_TYPE = "Select the file type:";
  inline const char *const P_OFILE     = "Which file to read to?";
  inline const char *const P_IFILE     = "Which file to write from?";
  inline const char *const P_VIEW_METH = "Select viewing method:";
  inline const char *const P_STORE     = "Where to store data?";
  inline const char *const P_VERIFY    = "Verify data?";
  inline const char *const P_DATA_DIR  = "Which direction?";

  inline const char *const W_OFILE     = "Reading EEPROM to file...";
  inline const char *const W_IFILE     = "Writing file to EEPROM...";
  inline const char *const W_RMULTI    = "Reading EEPROM bytes...";
  inline const char *const W_WMULTI    = "Writing EEPROM bytes...";
  inline const char *const W_WAIT      = "Please wait...";
  inline const char *const W_LOAD      = "Loading...";

  inline const char *const F_READ      = "Done reading!";
  inline const char *const F_WRITE     = "Done writing!";
  inline const char *const F_VERIFY    = "Done verifying!";

  inline const char *const L_PROJ_NAME = "eeprommer3";
  inline const char *const L_SD_GOOD   = "SD init success!";
  inline const char *const L_SD_FAIL   = "SD init failed!";
  inline const char *const L_SD_DISAB  = "SD init disabled!";
  inline const char *const L_SD_INVAL  = "SD invalid status!";
  inline const char *const L_INTRO1    = "by @beaver700nh";
  inline const char *const L_INTRO2    = "made using Arduino";
  inline const char *const L_ARROW_L   = "\x11";
  inline const char *const L_ARROW_R   = "\x10";
  inline const char *const L_ARROW_U   = "\x1e";
  inline const char *const L_ARROW_D   = "\x1f";
  inline const char *const L_CANCEL    = "Cancel";
  inline const char *const L_CLOSE     = "Close";
  inline const char *const L_SKIP      = "Skip";
  inline const char *const L_CONFIRM   = "Confirm";
  inline const char *const L_CONTINUE  = "Continue";
  inline const char *const L_INPUT     = "Input";
  inline const char *const L_OUTPUT    = "Output";
  inline const char *const L_YES       = "Yes";
  inline const char *const L_NO        = "No";
  inline const char *const L_OK        = "OK";
  inline const char *const L_GO_UP_DIR = "Parent Dir";
  inline const char *const L_ADD_PAIR  = "Add Pair";
  inline const char *const L_NO_PAIRS1 = "No pairs yet!";
  inline const char *const L_NO_PAIRS2 = "Click `Add Pair' to add a pair!";
  inline const char *const L_VM_HEX    = "Show as Raw Hexadecimal";
  inline const char *const L_VM_CHAR   = "Show Printable Characters";
  inline const char *const L_VM_FILE   = "Write Data to a File";
  inline const char *const L_NO_HELP   = "No help text available.";
  inline const char *const L_UNK_REAS  = "Unknown reason.";
  inline const char *const L_FILE_SD   = "SD Card File";
  inline const char *const L_FILE_SER  = "Serial File";
  inline const char *const L_X_CLOSE   = "x";
  inline const char *const L_EMPTY_STR = "";

  inline const char *const A_R_BYTE    = "Read Byte";
  inline const char *const A_W_BYTE    = "Write Byte";
  inline const char *const A_R_FILE    = "Read to File";
  inline const char *const A_W_FILE    = "Write from File";
  inline const char *const A_R_VECTOR  = "Read Vector";
  inline const char *const A_W_VECTOR  = "Write Vector";
  inline const char *const A_R_MULTI   = "Read Range";
  inline const char *const A_W_MULTI   = "Write Multiple";
  inline const char *const A_DRAW_TEST = "Draw Test";
  inline const char *const A_DEBUGS    = "Debug Tools";
  inline const char *const A_INFO      = "i";

  inline const char *const D_WE_HI     = "WE Hi (Disable)";
  inline const char *const D_WE_LO     = "WE Lo (Enable)";
  inline const char *const D_SET_ADDR  = "Set Address/OE";
  inline const char *const D_RD_DATA   = "Read Data Bus";
  inline const char *const D_WR_DATA   = "Write Data Bus";
  inline const char *const D_SET_DDIR  = "Set Data Dir";
  inline const char *const D_MON_DATA  = "Monitor Data";
  inline const char *const D_P_CHARSET = "Print Charset";
  inline const char *const D_SHOW_COL  = "Show Colors";
  inline const char *const D_AUX1      = "Aux1";
  inline const char *const D_AUX2      = "Aux2";

  inline const char *const I_SUBTITLE  = "- hardware/firmware side";
  inline const char *const I_LINE_1    = "AT28C256 EEPROM programmer using 2 I2C";
  inline const char *const I_LINE_2    = "MCP23017 chips. Use: standalone device";
  inline const char *const I_LINE_3    = "or computer peripheral via USB. Allows";
  inline const char *const I_LINE_4    = "access to SD card connected on SPI bus";
  inline const char *const I_LINE_5    = "Made by beaver700nh (GitHub) 2021-2022";
}

#endif
