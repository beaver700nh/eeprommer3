#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "input.hpp"
#include "eeprom.hpp"
#include "sd.hpp"
#include "pfsd_core.hpp"

// Fwd decl because of circular dependency
class ProgrammerFromSdBaseCore;

/*
 * `ProgrammerFromSd` is a class that connects the front-end and back-end for programming the EEPROM from the on-board SD card.
 */
class ProgrammerFromSd {
public:
  ProgrammerFromSd(TYPED_CONTROLLERS);
  ~ProgrammerFromSd();

  void init();

  void run();
  void show_status(ProgrammerFromSdBaseCore::Status code);

public:
  static constexpr uint8_t NUM_ACTIONS = 12;

  ProgrammerFromSdBaseCore *m_cores[NUM_ACTIONS];

#define FUNC(type, name) (ProgrammerFromSdBaseCore::Func) &ProgrammerFromSd##type##Core::name

  ProgrammerFromSdBaseCore::Func action_map[NUM_ACTIONS] = {
    FUNC(Byte,   read ), FUNC(Byte,   write),
    FUNC(File,   read ), FUNC(File,   write),
    FUNC(Vector, read ), FUNC(Vector, write),
    FUNC(Multi,  read ), FUNC(Multi,  write),
    FUNC(Other,  paint), FUNC(Other,  debug),
    FUNC(Other,  about),
  };

#undef FUNC

  TftChoiceMenu m_menu;
  bool initialized;

  TftCtrl &m_tft;
  TouchCtrl &m_tch;
  EepromCtrl &m_ee;
  SdCtrl &m_sd;
};

#endif
