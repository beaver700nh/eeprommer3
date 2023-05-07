#include <Arduino.h>
#include "constants.hpp"

#include <SD/src/SD.h>

#include "eeprom.hpp"
#include "error.hpp"
#include "gui.hpp"
#include "new_delete.hpp"
#include "sd.hpp"
#include "strfmt.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"
#include "util.hpp"

#include "prog.hpp"

extern TftCtrl tft;
extern TouchCtrl tch;

Programmer::Programmer() : m_menu(10, 10, 50, 10, 2, 30, true) {
  // Empty body because all work done in init list
}

void show_help(uint8_t btn_id, bool is_confirm) {
  if (is_confirm) return;

  static const char *const helps[] PROGMEM {
    Strings::H_R_BYTE,
    Strings::H_W_BYTE,
    Strings::H_R_FILE,
    Strings::H_W_FILE,
    Strings::H_R_VECTOR,
    Strings::H_W_VECTOR,
    Strings::H_R_MULTI,
    Strings::H_W_MULTI,
    Strings::H_DRAW_TEST,
    Strings::H_DEBUGS,
    Strings::H_INFO,
    Strings::H_X_CLOSE,
  };

  tft.fillRect(10, 250, tft.width() - 10, 16, TftColor::BLACK);

  if (btn_id >= ARR_LEN(helps)) {
no_help:
    tft.drawText_P(10, 250, Strings::L_NO_HELP, TftColor::GRAY, 2);
    return;
  }

  const char *const help = pgm_read_word_near(helps + btn_id);

  if (strlen_P(help) == 0) {
    goto no_help;
  }

  tft.drawText_P(10, 250, help, TftColor::PURPLE, 2);
}

void Programmer::init() {
  m_menu.add_btn_calc(Strings::A_R_BYTE,    TftColor::BLUE,           TftColor::CYAN          );
  m_menu.add_btn_calc(Strings::A_W_BYTE,    TftColor::RED,            TftColor::PINKK         );
  m_menu.add_btn_calc(Strings::A_R_FILE,    TftColor::CYAN,           TftColor::BLUE          );
  m_menu.add_btn_calc(Strings::A_W_FILE,    TftColor::PINKK,          TftColor::RED           );
  m_menu.add_btn_calc(Strings::A_R_VECTOR,  TO_565(0x00, 0x17, 0x00), TftColor::LGREEN        );
  m_menu.add_btn_calc(Strings::A_W_VECTOR,  TO_565(0x3F, 0x2F, 0x03), TO_565(0xFF, 0xEB, 0x52));
  m_menu.add_btn_calc(Strings::A_R_MULTI,   TftColor::LGREEN,         TftColor::DGREEN        );
  m_menu.add_btn_calc(Strings::A_W_MULTI,   TftColor::BLACK,          TftColor::ORANGE        );
  m_menu.add_btn_calc(Strings::A_DRAW_TEST, TftColor::DGRAY,          TftColor::GRAY          );
  m_menu.add_btn_calc(Strings::A_DEBUGS,    TftColor::DGRAY,          TftColor::GRAY          );

  m_menu.add_btn(new Gui::Btn(TftCalc::right(tft, 24, 10), 10, 24, 24, Strings::A_INFO,    TftColor::WHITE,  TftColor::BLUE));
  m_menu.add_btn(new Gui::Btn(TftCalc::right(tft, 24, 44), 10, 24, 24, Strings::A_X_CLOSE, TftColor::YELLOW, TftColor::RED));

  m_menu.add_btn_confirm(true);

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

    tft.drawText_P(10, 10, Strings::P_ACTION, TftColor::CYAN, 3);
    show_help(cur_choice, false);
    cur_choice = m_menu.wait_for_value();

    tft.fillScreen(TftColor::BLACK);

    SER_LOG_PRINT("Executing action #%d.\n", cur_choice);

    ProgrammerBaseCore::Status status_code = ProgrammerBaseCore::Status::ERR_INVALID;

    auto action = action_map[cur_choice];

    // Run the action function if available
    if (cur_choice < NUM_ACTIONS) {
      status_code = (*action)();
    }

    SER_LOG_PRINT("Action returned status code %d.\n", status_code);
    SER_LOG_PRINT("\n");

    show_status(status_code);

    tft.fillScreen(TftColor::BLACK);
  }
}

void Programmer::show_status(ProgrammerBaseCore::Status code) {
  static const char *const details[] PROGMEM {
    Strings::S_CODE_0,
    Strings::S_CODE_1,
    Strings::S_CODE_2,
    Strings::S_CODE_3,
    Strings::S_CODE_4,
  };

  bool success = (code == ProgrammerBaseCore::Status::OK);

  const char *success_str      = (success ? Strings::T_SUCCESS : Strings::T_FAILED);
  const uint16_t success_color = (success ? TftColor::GREEN : TftColor::RED);

  tft.drawText_P(15, TftCalc::bottom(tft, 16, 44), success_str, success_color, 2);

  const char *const status_text = (code < ARR_LEN(details) ? (const char *) pgm_read_word_near(details + code) : Strings::L_UNK_REAS);

  Dialog::show_error(ErrorLevel::INFO, 0x3, Strings::T_RESULT, status_text);
}
