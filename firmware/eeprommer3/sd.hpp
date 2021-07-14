#ifndef SD_HPP
#define SD_HPP

#include <Arduino.h>
#include "constants.hpp"

#include <SPI.h>
#include <SD.h>

#define PRINTER_LAMBDA (const char *text, uint8_t n = 1) -> void

class SdCtrl {
public:
  SdCtrl() {};
  SdCtrl(uint8_t cs, int8_t en = -1);

  uint8_t init();

  template<typename Printer>
  void print_files(Printer printer, const char *dir, uint8_t max_ind, uint8_t ind = 0) {
    File root = SD.open(dir);
    File file;

    while ((file = root.openNextFile())) {
      printer("  ", ind);
      Serial.println("Indenting...");

      if (file.isDirectory()) {
        Serial.println("Found directory!");

        char buf[50];
        sprintf(buf, "%s {\n", file.name());
        printer(buf);

        Serial.print("Indentation ");
        Serial.print(ind, DEC);
        Serial.print(" vs ");
        Serial.print(max_ind, DEC);
        Serial.println(".");

        if (ind < max_ind) print_files(printer, file.name(), max_ind, ind + 1);

        printer("  ", ind);
        printer("}\n");
      }
      else {
        Serial.println("Found file!");
        char buf[50];
        sprintf(buf, "%s%10lu\n", file.name(), file.size());
        printer(buf);
      }

      Serial.println("Closing!");
      file.close();
    }
  }

private:
  uint8_t m_cs;
  int8_t m_en;

  bool m_enabled;
};

#endif
