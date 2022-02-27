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

// Go up a directory from `path`. Return whether operation was successful.
bool go_up_dir(char *path);

// Set `path` to "`path`/`file`". Return false if resulting path is `len` chars or longer, true otherwise.
bool go_down_file(char *path, const char *file, uint8_t len);

// Set `path` to "`path`/`dir`/". Return false if resulting path is `len` chars or longer, true otherwise.
bool go_down_dir(char *path, const char *dir, uint8_t len);

// Set `path` to file `sub_path` inside `path`. Return false if resulting path is `len` chars or longer, true otherwise.
bool go_down_path(char *path, FileInfo *sub_path, uint8_t len);

#endif