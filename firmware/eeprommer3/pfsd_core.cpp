#include <Arduino.h>
#include "constants.hpp"

#include "eeprom.hpp"
#include "input.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "file.hpp"

#include "pfsd_core.hpp"

/***************************/
/******** BYTE CORE ********/
/***************************/

ProgrammerFromSdBaseCore::Status ProgrammerFromSdByteCore::read() {
  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Type an address:");
  uint8_t data = m_ee.read(addr);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, STRFMT_NOBUF("Value at address %04X:", addr),       TftColor::CYAN,   3);
  m_tft.drawText(20,  50, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(data)), TftColor::YELLOW, 2);
  m_tft.drawText(20,  80, STRFMT_NOBUF("OCT: %03o",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 110, STRFMT_NOBUF("HEX: %02X",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 140, STRFMT_NOBUF("DEC: %-3d",      data),               TftColor::YELLOW, 2);
  m_tft.drawText(20, 170, STRFMT_NOBUF("CHR: %c",        data),               TftColor::YELLOW, 2);

  Util::wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdByteCore::write() {
  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Type an address:");
  m_tft.fillScreen(TftColor::BLACK);
  uint8_t data = ask_val<uint8_t>(m_tft, m_tch, "Type the data:");

  m_ee.write(addr, data);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                             TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("data %02X", data),     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("address %04X.", addr), TftColor::GREEN,  4);

  Util::wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(addr, (void *) &data);
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdByteCore::verify(uint16_t addr, void *data, uint16_t len) {
  UNUSED_VAR(len);

  uint8_t actual = m_ee.read(addr);

  if (actual != *(uint8_t *) data) {
    m_tft.drawText(10, 10, "Result:",                                         TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %02X", *(uint8_t *) data), TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %02X", actual),            TftColor::MAGENTA, 3);

    Util::wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}

/***************************/
/******** FILE CORE ********/
/***************************/

ProgrammerFromSdBaseCore::Status ProgrammerFromSdFileCore::read() {
  Status status = Status::OK;

  char fname[64];
  ask_str(m_tft, m_tch, "File to read to?", fname, 63);

  m_tft.fillScreen(TftColor::BLACK);

  uint8_t this_page[256];
  File file = SD.open(fname, O_CREAT | O_WRITE | O_TRUNC);

  if (!file) status = Status::ERR_FILE;
  else {
    m_tft.drawText(10, 10, "Working... Progress:", TftColor::CYAN, 3);

    TftProgressIndicator bar(m_tft, 127, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

    bar.for_each(
      [this, &this_page, &file]TFT_PROGRESS_INDICATOR_LAMBDA {
        uint16_t addr = progress * 0x0100;

        this->m_ee.read(addr, addr + 0xFF, this_page);
        file.write(this_page, 256);

        return this->m_tch.is_touching();
      }
    );

    m_tft.drawText(10, 110, "Done reading!", TftColor::CYAN);
    Util::wait_continue(m_tft, m_tch);
  }

  file.flush();
  file.close();
  m_tft.fillScreen(TftColor::BLACK);
  return status;
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdFileCore::write() {
  Status status = Status::OK;

  char fname[64];
  ask_file(m_tft, m_tch, m_sd, "File to write from?", fname, 63);
  m_tft.fillScreen(TftColor::BLACK);

  uint16_t addr = ask_val<uint16_t>(m_tft, m_tch, "Where to write in EEPROM?");
  uint16_t cur_addr = addr;
  m_tft.fillScreen(TftColor::BLACK);

  uint8_t this_page[256];
  File file = SD.open(fname, O_READ);

  if (!file) status = Status::ERR_FILE;
  else {
    m_tft.drawText(10, 10, "Working... Progress:", TftColor::CYAN, 3);

    TftProgressIndicator bar(m_tft, ceil((float) file.size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

    bar.for_each(
      [this, &this_page, &file, &cur_addr]TFT_PROGRESS_INDICATOR_LAMBDA {
        auto len = file.read(this_page, 256);
        this->m_ee.write(cur_addr, this_page, MIN(len, 256));

        cur_addr += 0x0100; // Next page

        return this->m_tch.is_touching();
      }
    );

    m_tft.drawText(10, 110, "Done writing!", TftColor::CYAN);
    Util::wait_continue(m_tft, m_tch);
  }

  file.close();
  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_VALUE(status, addr, fname);
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdFileCore::verify(uint16_t addr, void *data, uint16_t len) {
  UNUSED_VAR(len);

  Status status = Status::OK;

  File file = SD.open((const char *) data, O_READ);

  uint8_t expectation[256];
  uint8_t reality[256];

  if (!file) return Status::ERR_FILE;

  m_tft.drawText(10, 10, STRFMT_NOBUF("Verifying `%s' at %04X...", (const char *) data, addr), TftColor::CYAN);

  TftProgressIndicator bar(m_tft, ceil((float) file.size() / 256.0) - 1, 10, 50, TftCalc::fraction_x(m_tft, 10, 1), 40);

  bool complete = bar.for_each(
    [this, &expectation, &reality, &file, &addr]TFT_PROGRESS_INDICATOR_LAMBDA {
      auto nbytes = file.read(expectation, 256);
      this->m_ee.read(addr, addr + nbytes, reality);

      if (memcmp(expectation, reality, nbytes) != 0) {
        this->m_tft.drawText(10, 110, STRFMT_NOBUF("Mismatch between %04X and %04X!", addr, addr + 0xFF), TftColor::RED);

        // Request to quit loop
        return true;
      }

      addr += 0x0100; // Next page

      return false;
    }
  );

  m_tft.drawText(10, 110, "Done verifying!", TftColor::CYAN);
  Util::wait_continue(m_tft, m_tch);

  // `complete` is true if the loop finished normally
  if (!complete) status = Status::ERR_VERIFY;

  file.close();
  m_tft.fillScreen(TftColor::BLACK);
  return status;
}

/*****************************/
/******** VECTOR CORE ********/
/*****************************/

// Vector is a helper class for vector-related actions.
struct Vector {
  Vector(uint8_t id) : m_id(id), m_addr(0xFFF8 + 2 * (id + 1)) {};

  void update(EepromCtrl &ee) {
    m_lo = ee.read(m_addr);
    m_hi = ee.read(m_addr + 1);
    m_val = (m_hi << 8) | m_lo;
  }

  uint8_t m_id;
  uint16_t m_addr;

  uint8_t m_lo, m_hi;
  uint16_t m_val;

  inline static const char *NAMES[3] = {"NMI", "RESET", "IRQ"};
};

Vector ask_vector(TftCtrl &tft, TouchCtrl &tch) {
  return Vector(
    ask_choice(
      tft, tch, "Which vector?", 3, 54, 1, 3,
      Vector::NAMES[0], TftColor::CYAN,           TftColor::BLUE,
      Vector::NAMES[1], TO_565(0x7F, 0xFF, 0x7F), TftColor::DGREEN,
      Vector::NAMES[2], TftColor::PINKK,          TftColor::RED
    )
  );
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdVectorCore::read() {
  Vector vec = ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  m_tft.drawText( 10,  10, STRFMT_NOBUF("Value of %s vector:", Vector::NAMES[vec.m_id]), TftColor::CYAN,   3);
  m_tft.drawText(320,  50, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1),      TftColor::BLUE,   2);
  m_tft.drawText( 16,  50, STRFMT_NOBUF("HEX: %04X", vec.m_val),                         TftColor::YELLOW, 2);
  m_tft.drawText( 16,  80, STRFMT_NOBUF("BIN: " BYTE_FMT, BYTE_FMT_VAL(vec.m_hi)),       TftColor::YELLOW, 2);
  m_tft.drawText( 16, 110, STRFMT_NOBUF(".... " BYTE_FMT, BYTE_FMT_VAL(vec.m_lo)),       TftColor::YELLOW, 2);

  Util::wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  return Status::OK;
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdVectorCore::write() {
  Vector vec = ask_vector(m_tft, m_tch);
  vec.update(m_ee);

  m_tft.fillScreen(TftColor::BLACK);

  uint16_t new_val = ask_val<uint16_t>(m_tft, m_tch, "Type the new value:");
  m_ee.write(vec.m_addr,     new_val & 0xFF);
  m_ee.write(vec.m_addr + 1, new_val >> 8);

  m_tft.fillScreen(TftColor::BLACK);
  m_tft.drawText(10,  10, "Wrote",                                                 TftColor::DGREEN, 3);
  m_tft.drawText(10,  37, STRFMT_NOBUF("value %04X", new_val),                     TftColor::GREEN,  4);
  m_tft.drawText(10,  73, "to",                                                    TftColor::DGREEN, 3);
  m_tft.drawText(10, 100, STRFMT_NOBUF("vector %s.", Vector::NAMES[vec.m_id]),     TftColor::GREEN,  4);
  m_tft.drawText(10, 136, STRFMT_NOBUF("(%04X-%04X)", vec.m_addr, vec.m_addr + 1), TftColor::DGREEN, 2);

  Util::wait_continue(m_tft, m_tch);

  m_tft.fillScreen(TftColor::BLACK);

  RETURN_VERIFICATION_OR_OK(vec.m_addr, (void *) &new_val)
}

ProgrammerFromSdBaseCore::Status ProgrammerFromSdVectorCore::verify(uint16_t addr, void *data, uint16_t len) {
  uint16_t actual = (m_ee.read(addr + 1) << 8) | m_ee.read(addr);

  if (actual != *(uint16_t *) data) {
    m_tft.drawText(10, 10, "Result:",                                          TftColor::ORANGE,  4);
    m_tft.drawText(15, 50, STRFMT_NOBUF("Expected: %04X", *(uint16_t *) data), TftColor::PURPLE,  3);
    m_tft.drawText(15, 77, STRFMT_NOBUF("Actual:   %04X", actual),             TftColor::MAGENTA, 3);

    Util::wait_continue(m_tft, m_tch);

    m_tft.fillScreen(TftColor::BLACK);

    return Status::ERR_VERIFY;
  }

  return Status::OK;
}
