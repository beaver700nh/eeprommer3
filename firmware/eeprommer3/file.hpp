#ifndef FILE_HPP
#define FILE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "sd.hpp"

// Fwd decls just in case (circular dependencies)
class TftCtrl;
class TouchCtrl;
class SdCtrl;
class TftChoiceMenu;

/*
 * Simple little helper struct to store name of file and whether it is a directory.
 */
struct FileInfo {
  char name[13];
  bool is_dir;
};

/*
 * Yet another `TftXXXMenu`, this one to ask user to select a file from an SD card. Inherits from `TftChoiceMenu`.
 */
class TftFileSelMenu : public TftChoiceMenu {
public:
  TftFileSelMenu(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols);
  ~TftFileSelMenu();

  enum Status : uint8_t {
    OK,             // No errors
    CANCELED,       // User pressed "Cancel"
    FNAME_TOO_LONG, // Filename was too long to fit in buffer
  };

  /*
   * Sets internal files buffer to the files in directory `path` on `sd`. Reads at most `max_files` files from directory.
   */
  void use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files);

  /* Waits for user to select a file.
   *
   * Returns Status::CANCELED if user pressed `Cancel` button.
   * Returns Status::FNAME_TOO_LONG if resulting path is `max_path_len` chars or longer.
   *
   * If all conditions are met, return Status::OK and sets
   * `file_path` to the path to the file which the user selected.
   */
  Status wait_for_value(TouchCtrl &tch, TftCtrl &tft, SdCtrl &sd, char *file_path, uint8_t max_path_len);

private:
  // The supplied `cols` or the maximum number of cols that will fit, whichever is smaller
  static inline uint8_t calc_num_cols(TftCtrl &tft, uint8_t cols) {
    return MIN(cols, MAX((tft.width() - 10) / (73 + 10), 1));
  }

  // A fraction 1/`rows` of allotted vertical space (screen height with `marg_v` vertical margin), constrained to at least 16
  static inline uint8_t calc_btn_height(TftCtrl &tft, uint8_t rows, uint8_t marg_v, uint8_t pad_v) {
    return MAX(
      16, TftCalc::fraction(
      tft.height() // take up as much space as possible
      - marg_v     // without the top margin
      - 78         // without the bottom margin
      + 2 * pad_v, // but still need some amount
      pad_v, rows
      )
    );
  }

  FileInfo *m_files = nullptr;
  uint8_t m_num_files = 0;

  uint8_t m_num_rows;
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

// Ask user to select a file on SD card. Writes path into `out`. Return a Status based on user's choice.
TftFileSelMenu::Status ask_file(TftCtrl &tft, TouchCtrl &tch, SdCtrl &sd, const char *prompt, char *out, uint8_t len);

#endif
