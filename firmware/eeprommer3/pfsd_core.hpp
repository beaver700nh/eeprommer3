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

#define ADD_CORE_DECLARATION(name) \
  class ProgrammerFromSd##name##Core : public ProgrammerFromSdBaseCore { \
  public: \
    ProgrammerFromSd##name##Core(TYPED_CONTROLLERS) : ProgrammerFromSdBaseCore(CONTROLLERS) {}; \
  \
    Status read(); \
    Status write(); \
  \
  private: \
    Status verify(uint16_t addr, void *data, uint16_t len = 0); \
  };

ADD_CORE_DECLARATION(Byte)
ADD_CORE_DECLARATION(File)
ADD_CORE_DECLARATION(Vector)

#define RETURN_VERIFICATION_OR_VALUE(value, ...) \
  bool should_verify = ask_yesno(m_tft, m_tch, "Verify data?"); \
  m_tft.fillScreen(TftColor::BLACK); \
  \
  return (should_verify ? verify(__VA_ARGS__) : value);

#define RETURN_VERIFICATION_OR_OK(...) RETURN_VERIFICATION_OR_VALUE(Status::OK, __VA_ARGS__)

#endif
