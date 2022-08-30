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

  // Duplicates PROGMEM string into normal string, returns it.
  // Caller is responsible for freeing.
  char *strdup_P(const char *pstr);

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
  static void calculate_bords();
  static void print_ram_analysis();

  static constexpr uint8_t NUM_TYPES = 8;

  static inline const char NAME_0[] PROGMEM = "Data";
  static inline const char NAME_1[] PROGMEM = "BSS";
  static inline const char NAME_2[] PROGMEM = "Free";
  static inline const char NAME_3[] PROGMEM = "Stack";
  static inline const char NAME_4[] PROGMEM = "Unused";
  static inline const char NAME_5[] PROGMEM = "Heap";
  static inline const char NAME_6[] PROGMEM = "Free";
  static inline const char NAME_7[] PROGMEM = "8k Buf";

  static inline const char *NAMES[NUM_TYPES] {
    NAME_0, NAME_1, NAME_2, NAME_3, NAME_4, NAME_5, NAME_6, NAME_7,
  };

  static inline uint16_t bords[NUM_TYPES + 1];
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
 * M_ - Misc (Non-PROGMEM)
 */
namespace Strings {
#define ADD_STRING(type, name, value) inline const char type##_##name[] PROGMEM = value

  ADD_STRING(T, DONE,      "Done");
  ADD_STRING(T, MISMATCH,  "Mismatch");
  ADD_STRING(T, CANCELED,  "Canceled");
  ADD_STRING(T, VALUE,     "Value");
  ADD_STRING(T, TOO_BIG,   "EEPROM Overflow");
  ADD_STRING(T, TOO_LONG,  "String Overflow");
  ADD_STRING(T, INV_FSYS,  "Invalid Filesystem");
  ADD_STRING(T, WMULTI,    "Write Multiple Bytes");
  ADD_STRING(T, NOT_SUPP,  "Not Supported");
  ADD_STRING(T, RESULT,    "Result");
  ADD_STRING(T, SUCCESS,   "Success");
  ADD_STRING(T, FAILED,    "Failed");
  ADD_STRING(T, ABOUT,     "About");
  ADD_STRING(T, EL_INFO,   "Info");
  ADD_STRING(T, EL_WARN,   "Warning");
  ADD_STRING(T, EL_ERR,    "Error");
  ADD_STRING(T, DEBUGS,    "Debug Tools Menu");

  ADD_STRING(E, CANCELED,  "Operation was canceled.");
  ADD_STRING(E, TOO_BIG,   "File would be too large\nto fit into EEPROM there!\nAborted.");
  ADD_STRING(E, TOO_LONG,  "File name was too long\nto fit in the buffer.");
  ADD_STRING(E, INV_FSYS,  "The selected filesystem\ndoes not exist.");
  ADD_STRING(E, NO_DB_MON, "Data bus monitor is not\nsupported because DEBUG_MODE\nis disabled.");

  ADD_STRING(P, ACTION,    "Choose an action:");
  ADD_STRING(P, ADDR_GEN,  "Type an address:");
  ADD_STRING(P, DATA_GEN,  "Type the data:");
  ADD_STRING(P, VAL_GEN,   "Type a value:");
  ADD_STRING(P, VECTOR,    "Select the vector:");
  ADD_STRING(P, ADDR_FILE, "Where to write in EEPROM?");
  ADD_STRING(P, ADDR_VEC,  "Type new vector value:");
  ADD_STRING(P, ADDR_BEG,  "Type the start address:");
  ADD_STRING(P, ADDR_END,  "Type the end address:");
  ADD_STRING(P, FILE_TYPE, "Select the file type:");
  ADD_STRING(P, OFILE,     "Which file to read to?");
  ADD_STRING(P, IFILE,     "Which file to write from?");
  ADD_STRING(P, VIEW_METH, "Select viewing method:");
  ADD_STRING(P, STORE,     "Where to store data?");
  ADD_STRING(P, VERIFY,    "Verify data?");
  ADD_STRING(P, DATA_DIR,  "Which direction?");

  ADD_STRING(W, OFILE,     "Reading EEPROM to file...");
  ADD_STRING(W, IFILE,     "Writing file to EEPROM...");
  ADD_STRING(W, RMULTI,    "Reading EEPROM bytes...");
  ADD_STRING(W, WMULTI,    "Writing EEPROM bytes...");
  ADD_STRING(W, WAIT,      "Please wait...");
  ADD_STRING(W, LOAD,      "Loading...");

  ADD_STRING(F, READ,      "Done reading!");
  ADD_STRING(F, WRITE,     "Done writing!");
  ADD_STRING(F, VERIFY,    "Done verifying!");

  ADD_STRING(L, PROJ_NAME, "eeprommer3");
  ADD_STRING(L, SD_GOOD,   "SD init success!");
  ADD_STRING(L, SD_FAIL,   "SD init failed!");
  ADD_STRING(L, SD_DISAB,  "SD init disabled!");
  ADD_STRING(L, SD_INVAL,  "SD invalid status!");
  ADD_STRING(L, INTRO1,    "by @beaver700nh");
  ADD_STRING(L, INTRO2,    "made using Arduino");
  ADD_STRING(L, ARROW_L,   "\x11");
  ADD_STRING(L, ARROW_R,   "\x10");
  ADD_STRING(L, ARROW_U,   "\x1e");
  ADD_STRING(L, ARROW_D,   "\x1f");
  ADD_STRING(L, CANCEL,    "Cancel");
  ADD_STRING(L, CLOSE,     "Close");
  ADD_STRING(L, SKIP,      "Skip");
  ADD_STRING(L, CONFIRM,   "Confirm");
  ADD_STRING(L, CONTINUE,  "Continue");
  ADD_STRING(L, INPUT,     "Input");
  ADD_STRING(L, OUTPUT,    "Output");
  ADD_STRING(L, YES,       "Yes");
  ADD_STRING(L, NO,        "No");
  ADD_STRING(L, OK,        "OK");
  ADD_STRING(L, ADD_PAIR,  "Add Pair");
  ADD_STRING(L, NO_PAIRS1, "No pairs yet!");
  ADD_STRING(L, NO_PAIRS2, "Click `Add Pair' to add a pair!");
  ADD_STRING(L, VM_HEX,    "Show as Raw Hexadecimal");
  ADD_STRING(L, VM_CHAR,   "Show Printable Characters");
  ADD_STRING(L, VM_FILE,   "Write Data to a File");
  ADD_STRING(L, NO_HELP,   "No help text available.");
  ADD_STRING(L, UNK_REAS,  "Unknown reason.");
  ADD_STRING(L, FILE_SD,   "SD Card File");
  ADD_STRING(L, FILE_SER,  "Serial File");
  ADD_STRING(L, NO_FILES,  "Hmm, no files here...");
  ADD_STRING(L, X_CLOSE,   "x");
  ADD_STRING(L, EMPTY_STR, "");

  ADD_STRING(A, R_BYTE,    "Read Byte");
  ADD_STRING(A, W_BYTE,    "Write Byte");
  ADD_STRING(A, R_FILE,    "Read to File");
  ADD_STRING(A, W_FILE,    "Write from File");
  ADD_STRING(A, R_VECTOR,  "Read Vector");
  ADD_STRING(A, W_VECTOR,  "Write Vector");
  ADD_STRING(A, R_MULTI,   "Read Range");
  ADD_STRING(A, W_MULTI,   "Write Multiple");
  ADD_STRING(A, DRAW_TEST, "Draw Test");
  ADD_STRING(A, DEBUGS,    "Debug Tools");
  ADD_STRING(A, INFO,      "i");

  ADD_STRING(D, WE_HI,     "WE Hi (Disable)");
  ADD_STRING(D, WE_LO,     "WE Lo (Enable)");
  ADD_STRING(D, SET_ADDR,  "Set Address/OE");
  ADD_STRING(D, RD_DATA,   "Read Data Bus");
  ADD_STRING(D, WR_DATA,   "Write Data Bus");
  ADD_STRING(D, SET_DDIR,  "Set Data Dir");
  ADD_STRING(D, MON_DATA,  "Monitor Data");
  ADD_STRING(D, P_CHARSET, "Print Charset");
  ADD_STRING(D, SHOW_COL,  "Show Colors");
  ADD_STRING(D, AUX1,      "Aux1");
  ADD_STRING(D, AUX2,      "Aux2");

  ADD_STRING(I, SUBTITLE,  "- hardware/firmware side");
  ADD_STRING(I, LINE_1,    "AT28C256 EEPROM programmer using 2 I2C");
  ADD_STRING(I, LINE_2,    "MCP23017 chips. Use by itself, or plug");
  ADD_STRING(I, LINE_3,    "into computer. File access: connection");
  ADD_STRING(I, LINE_4,    "over USB or from microSD card in slot.");
  ADD_STRING(I, LINE_5,    "Made by beaver700nh (GitHub) 2021-2022");

  inline const char *const M_FILE_DIR = "/ERMR3/";

#undef ADD_STRING
}

#endif
