#include <Arduino.h>
#include "constants.hpp"

#include <SD.h>

#include "tft.hpp"
#include "input.hpp"
#include "eeprom.hpp"
#include "sd.hpp"

#include "prog.hpp"

ProgrammerFromSd::ProgrammerFromSd(TYPED_CONTROLLERS) : INIT_LIST_CONTROLLERS {
  m_cores[0] = new ProgrammerFromSdByteCore  (CONTROLLERS);
  m_cores[1] = new ProgrammerFromSdFileCore  (CONTROLLERS);
  m_cores[2] = new ProgrammerFromSdVectorCore(CONTROLLERS);
  m_cores[3] = new ProgrammerFromSdMultiCore (CONTROLLERS);
  m_cores[4] = new ProgrammerFromSdOtherCore (CONTROLLERS);
}

ProgrammerFromSd::~ProgrammerFromSd() {
  for (uint8_t i = 0; i < NUM_ACTIONS / 2; ++i) {
    delete m_cores[i];
  }
}

void show_help(TftCtrl &tft, uint8_t btn_id, bool is_confirm) {
  if (is_confirm) return;

  static const char *helps[] = {
    "Read a byte from EEPROM.",
    "Write a byte to EEPROM.",
    "Read entire EEPROM to a file.",
    "Write file to EEPROM somewhere.",
    "Read 6502 jump vector contents.",
    "Write to a 6502 jump vector.",
    "Read multiple bytes from EEPROM.",
    "Write multiple bytes to EEPROM.",
  };

  tft.fillRect(10, 250, tft.width(), 16, TftColor::BLACK);
  tft.drawText(10, 250, (btn_id < ARR_LEN(helps) ? helps[btn_id] : "No help text available."), TftColor::PURPLE, 2);
}

void ProgrammerFromSd::run() {
  TftChoiceMenu menu(10, 10, 50, 10, 2, 30, true);
  menu.add_btn_calc(m_tft, "Read Byte",       TftColor::BLUE,           TftColor::CYAN);
  menu.add_btn_calc(m_tft, "Write Byte",      TftColor::RED,            TftColor::PINKK);
  menu.add_btn_calc(m_tft, "Read to File",    TftColor::CYAN,           TftColor::BLUE);
  menu.add_btn_calc(m_tft, "Write from File", TftColor::PINKK,          TftColor::RED);
  menu.add_btn_calc(m_tft, "Read Vector",     TO_565(0x00, 0x17, 0x00), TftColor::LGREEN);
  menu.add_btn_calc(m_tft, "Write Vector",    TO_565(0x3F, 0x2F, 0x03), TO_565(0xFF, 0xEB, 0x52));
  menu.add_btn_calc(m_tft, "Read Range",      TftColor::LGREEN,         TftColor::DGREEN);
  menu.add_btn_calc(m_tft, "Write Multiple",  TftColor::BLACK,          TftColor::ORANGE);
  menu.add_btn_calc(m_tft, "Draw Test",       TftColor::DGRAY,          TftColor::GRAY);
  menu.add_btn_calc(m_tft, "Debug Tools",     TftColor::DGRAY,          TftColor::GRAY);
  menu.add_btn_confirm(m_tft, true);

#ifndef DEBUG_MODE
  menu.get_btn(8)->operation(false);
  menu.get_btn(9)->operation(false);
#endif

  menu.set_callback(show_help);

  uint8_t cur_choice = 0;

  // Main Loop

  while (true) {
    m_tft.drawText(10, 10, "Choose an action:", TftColor::CYAN, 3);
    show_help(m_tft, cur_choice, false);
    cur_choice = menu.wait_for_value(m_tch, m_tft);

    m_tft.fillScreen(TftColor::BLACK);

    SER_LOG_PRINT("Executing action #%d: %s.", cur_choice, menu.get_btn(cur_choice)->get_text());

    ProgrammerFromSdBaseCore::Status status_code = (
      cur_choice < NUM_ACTIONS ?
      (m_cores[cur_choice / 2]->*action_map[cur_choice])() :
      ProgrammerFromSdBaseCore::Status::ERR_INVALID
    );

    SER_LOG_PRINT("Action returned status code %d.", status_code);

    show_status(status_code);

    Util::wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);
  }
}

void ProgrammerFromSd::show_status(ProgrammerFromSdBaseCore::Status code) {
  m_tft.drawText(10, 10, "Result:", TftColor::CYAN, 4);

  const char *details_buf[] = {
    "No errors.",
    "Invalid action.",
    "Failed to open file.",
    "Verification failed.",
    "Failed to allocate memory.",
  };

  const char  *str_repr = (code == ProgrammerFromSdBaseCore::Status::OK ? "Success!" : "Failed!");
  uint16_t   color_repr = (code ? TftColor::RED : TftColor::GREEN);
  const char *text_repr = (code < 5 ? details_buf[code] : "Unknown reason.");

  m_tft.drawText(15, 50, str_repr, color_repr, 3);
  m_tft.drawText(15, 80, text_repr, TftColor::PURPLE, 2);
}
