#ifndef PFSD_CORE_HPP
#define PFSD_CORE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"

// Fwd decl due to circular dependency
class ProgrammerFromSd;

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

  virtual Status read() = 0;
  virtual Status write() = 0;

protected:
  virtual Status verify(uint16_t addr, void *data, uint16_t len = 0) = 0;

  TftCtrl &m_tft;
  TouchCtrl &m_tch;
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
};

class ProgrammerFromSdByteCore : public ProgrammerFromSdBaseCore {
public:
  ProgrammerFromSdByteCore(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {};

  Status read();
  Status write();

private:
  Status verify(uint16_t addr, void *data, uint16_t len = 0);
};

#endif
