#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "eeprom.hpp"
#include "error.hpp"
#include "gui.hpp"
#include "new_delete.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "prog.hpp"

static const char PSTR_HELP_0[] PROGMEM = "Read a byte from EEPROM.";
static const char PSTR_HELP_1[] PROGMEM = "Write a byte to EEPROM.";
static const char PSTR_HELP_2[] PROGMEM = "Read entire EEPROM to a file.";
static const char PSTR_HELP_3[] PROGMEM = "Write file to EEPROM somewhere.";
static const char PSTR_HELP_4[] PROGMEM = "Read 6502 jump vector contents.";
static const char PSTR_HELP_5[] PROGMEM = "Write to a 6502 jump vector.";
static const char PSTR_HELP_6[] PROGMEM = "Read multiple bytes from EEPROM.";
static const char PSTR_HELP_7[] PROGMEM = "Write multiple bytes to EEPROM.";
static const char PSTR_HELP_8[] PROGMEM = "";
static const char PSTR_HELP_9[] PROGMEM = "";
static const char PSTR_HELP_A[] PROGMEM = "Show info/about/credits menu.";

static const char PSTR_DETAILS_0[] PROGMEM = "There were no errors.";
static const char PSTR_DETAILS_1[] PROGMEM = "Attempted to perform\nan invalid action.";
static const char PSTR_DETAILS_2[] PROGMEM = "Unable to open file.";
static const char PSTR_DETAILS_3[] PROGMEM = "Verification failed.\nContent read did not match\nwhat was written.";
static const char PSTR_DETAILS_4[] PROGMEM = "Memory allocation failed.\n(malloc() probably\nreturned null)";

Programmer::Programmer(TYPED_CONTROLLERS) : m_menu(10, 10, 50, 10, 2, 30, true), INIT_LIST_CONTROLLERS {
  static ProgrammerBaseCore
    *core_byte   = new ProgrammerByteCore  (CONTROLLERS),
    *core_file   = new ProgrammerFileCore  (CONTROLLERS),
    *core_vector = new ProgrammerVectorCore(CONTROLLERS),
    *core_multi  = new ProgrammerMultiCore (CONTROLLERS),
    *core_other  = new ProgrammerOtherCore (CONTROLLERS);

  m_cores[ 0] = core_byte;
  m_cores[ 1] = core_byte;
  m_cores[ 2] = core_file;
  m_cores[ 3] = core_file;
  m_cores[ 4] = core_vector;
  m_cores[ 5] = core_vector;
  m_cores[ 6] = core_multi;
  m_cores[ 7] = core_multi;
  m_cores[ 8] = core_other;
  m_cores[ 9] = core_other;
  m_cores[10] = core_other;
}

Programmer::~Programmer() {
  for (uint8_t i = 0; i < NUM_ACTIONS; ++i) {
    delete m_cores[i];
    m_cores[i] = nullptr;
  }
}

void show_help(TftCtrl &tft, uint8_t btn_id, bool is_confirm) {
  if (is_confirm) return;

  static const char *const helps[] {
    PSTR_HELP_0, PSTR_HELP_1,
    PSTR_HELP_2, PSTR_HELP_3,
    PSTR_HELP_4, PSTR_HELP_5,
    PSTR_HELP_6, PSTR_HELP_7,
    PSTR_HELP_8, PSTR_HELP_9,
    PSTR_HELP_A,
  };  // todo make this progmem + global

  char help_text[128];

  if (btn_id >= ARR_LEN(helps) || strlen_P(helps[btn_id]) == 0) {
    strcpy(help_text, "No help text available.");
  }
  else {
    strcpy_P(help_text, helps[btn_id]);
  }

  tft.fillRect(10, 250, tft.width(), 16, TftColor::BLACK);
  tft.drawText(10, 250, help_text, TftColor::PURPLE, 2);
}

void Programmer::init() {
  m_menu.add_btn_calc(m_tft, "Read Byte",       TftColor::BLUE,           TftColor::CYAN          );
  m_menu.add_btn_calc(m_tft, "Write Byte",      TftColor::RED,            TftColor::PINKK         );
  m_menu.add_btn_calc(m_tft, "Read to File",    TftColor::CYAN,           TftColor::BLUE          );
  m_menu.add_btn_calc(m_tft, "Write from File", TftColor::PINKK,          TftColor::RED           );
  m_menu.add_btn_calc(m_tft, "Read Vector",     TO_565(0x00, 0x17, 0x00), TftColor::LGREEN        );
  m_menu.add_btn_calc(m_tft, "Write Vector",    TO_565(0x3F, 0x2F, 0x03), TO_565(0xFF, 0xEB, 0x52));
  m_menu.add_btn_calc(m_tft, "Read Range",      TftColor::LGREEN,         TftColor::DGREEN        );
  m_menu.add_btn_calc(m_tft, "Write Multiple",  TftColor::BLACK,          TftColor::ORANGE        );
  m_menu.add_btn_calc(m_tft, "Draw Test",       TftColor::DGRAY,          TftColor::GRAY          );
  m_menu.add_btn_calc(m_tft, "Debug Tools",     TftColor::DGRAY,          TftColor::GRAY          );

  m_menu.add_btn(new Gui::Btn(TftCalc::right(m_tft, 24, 10), 10, 24, 24, "i", TftColor::WHITE, TftColor::BLUE));

  m_menu.add_btn_confirm(m_tft, true);

#ifndef DEBUG_MODE
  m_menu.get_btn(8)->operation(false);
  m_menu.get_btn(9)->operation(false);
#endif

  m_menu.set_callback(show_help);

  initialized = true;
}

void Programmer::run() {
  if (!initialized) return;

  uint8_t cur_choice = 0;

  // Main Loop

  while (true) {
    Memory::print_ram_analysis();

    m_tft.drawText(10, 10, "Choose an action:", TftColor::CYAN, 3);
    show_help(m_tft, cur_choice, false);
    cur_choice = m_menu.wait_for_value(m_tch, m_tft);

    m_tft.fillScreen(TftColor::BLACK);

    SER_LOG_PRINT("Executing action #%d: %s.\n", cur_choice, m_menu.get_btn(cur_choice)->get_text());

    ProgrammerBaseCore::Status status_code = ProgrammerBaseCore::Status::ERR_INVALID;

    auto the_core   = m_cores[cur_choice];
    auto the_action = action_map[cur_choice];

    // Run the action function if available
    if (cur_choice < NUM_ACTIONS) {
      status_code = (the_core->*the_action)();
    }

    SER_LOG_PRINT("Action returned status code %d.\n\n", status_code);

    show_status(status_code);

    m_tft.fillScreen(TftColor::BLACK);
  }
}

void Programmer::show_status(ProgrammerBaseCore::Status code) {
  static const char *const details[] {
    PSTR_DETAILS_0,
    PSTR_DETAILS_1,
    PSTR_DETAILS_2,
    PSTR_DETAILS_3,
    PSTR_DETAILS_4,
  }; // todo make this progmem + global

  bool success = (code == ProgrammerBaseCore::Status::OK);

  const char *success_str      = (success ? "Success!" : "Failed!");
  const uint16_t success_color = (success ? TftColor::GREEN : TftColor::RED);

  char code_text[128];

  if (code >= ARR_LEN(details)) {
    strcpy(code_text, "Unknown reason.");
  }
  else {
    strcpy_P(code_text, details[code]);
  }

  m_tft.drawText(15, TftCalc::bottom(m_tft, 16, 44), success_str, success_color, 2);

  Dialog::show_error(m_tft, m_tch, ErrorLevel::INFO, "Result", code_text);
}
