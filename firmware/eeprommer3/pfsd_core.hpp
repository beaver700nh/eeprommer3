#ifndef PFSD_CORE_HPP
#define PFSD_CORE_HPP

#include <Arduino.h>
#include "constants.hpp"

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

#define ADD_RW_CORE_DECLARATION(name) \
  class ProgrammerFromSd##name##Core : public ProgrammerFromSdBaseCore { \
  public: \
    ProgrammerFromSd##name##Core(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {}; \
  \
    Status read(); \
    Status write(); \
  \
  private: \
    Status verify(uint16_t addr, void *data); \
  };

// Manipulates a single byte at a time
ADD_RW_CORE_DECLARATION(Byte)
// Reads files to and from EEPROM
ADD_RW_CORE_DECLARATION(File)
// Manipulates one 6502 jump vector at a time (NMI, RESET, IRQ)
ADD_RW_CORE_DECLARATION(Vector)
// Reads ranges and writes arrays of pairs
ADD_RW_CORE_DECLARATION(Multi)

// Miscellaneous other functions
class ProgrammerFromSdOtherCore : public ProgrammerFromSdBaseCore {
public:
  ProgrammerFromSdOtherCore(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {};

  Status paint();
  Status debug();
  Status nop();
};

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?"); \
  m_tft.fillScreen(TftColor::BLACK); \
  \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

#endif
