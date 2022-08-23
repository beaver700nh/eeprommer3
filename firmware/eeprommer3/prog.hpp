#ifndef PROG_HPP
#define PROG_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "gui.hpp"
#include "prog_core.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "touch.hpp"

// Fwd decl because of circular dependency
class ProgrammerBaseCore;

/*
 * `Programmer` is a class that connects the front-end and back-end for programming the EEPROM .
 */
class Programmer {
public:
  Programmer();
  ~Programmer();

  void init();

  void run();
  void show_status(ProgrammerBaseCore::Status code);

  static constexpr uint8_t NUM_ACTIONS = 11;

  ProgrammerBaseCore *m_cores[NUM_ACTIONS];

#define FUNC(type, name) ((ProgrammerBaseCore::Func) &Programmer##type##Core::name)

  ProgrammerBaseCore::Func action_map[NUM_ACTIONS] {
    FUNC(Byte,   read),
    FUNC(Byte,   write),
    FUNC(File,   read),
    FUNC(File,   write),
    FUNC(Vector, read),
    FUNC(Vector, write),
    FUNC(Multi,  read),
    FUNC(Multi,  write),
    FUNC(Other,  paint),
    FUNC(Other,  debug),
    FUNC(Other,  about),
  };

#undef FUNC

  Gui::MenuChoice m_menu;
  bool initialized;
};

#endif
