#ifndef FILE_HPP
#define FILE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "sd.hpp"

struct FileInfo {
  char name[13];
  bool is_dir;
};

// Status type returned by ask_file()
enum AskFileStatus {FILE_STATUS_OK, FILE_STATUS_CANCELED, FILE_STATUS_FNAME_TOO_LONG};

class TftCtrl;
class TouchCtrl;
class SdCtrl;

// Ask user to select a file on SD card. Writes path into `out`.
// Return FILE_STATUS_FNAME_TOO_LONG if path is `len` chars or longer.
// Return FILE_STATUS_CANCELED is user presses "Cancel" button.
// Return FILE_STATUS_OK if everything goes smoothly.
AskFileStatus ask_file(TftCtrl &tft, TouchCtrl &tch, SdCtrl &sd, char *out, uint8_t len);

class TftChoiceMenu;

class TftFileSelMenu : public TftChoiceMenu {
public:
  TftFileSelMenu(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols);
  ~TftFileSelMenu();

  void use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files);

  AskFileStatus wait_for_value(TouchCtrl &tch, TftCtrl &tft, char *file_path, uint8_t max_path_len);

private:
  // The smaller of the supplied `cols` and the maximum number of cols that will fit
  inline uint8_t calc_num_cols(TftCtrl &tft, uint8_t cols) {
    return MIN(cols, MAX((tft.width() - 10) / (73 + 10), 1));
  }

  // 1/`rows` of the allotted vertical space, constrained to at least 16
  inline uint8_t calc_btn_height(TftCtrl &tft, uint8_t rows) {
    return MAX(16, TftCalc::fraction_y(tft, m_marg_v, rows));
  }

  FileInfo *m_files = nullptr;
  uint8_t m_num_files = 0;
};

// A namespace containing functions for manipulations of file paths
namespace FileUtil {
  // Go up a directory from `path`. Return whether operation was successful.
  bool go_up_dir(char *path);

  // Set `path` to "`path`/`file`". Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_file(char *path, const char *file, uint8_t len);

  // Set `path` to "`path`/`dir`/". Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_dir(char *path, const char *dir, uint8_t len);

  // Set `path` to file `sub_path` inside `path`. Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_path(char *path, FileInfo *sub_path, uint8_t len);
};

#endif