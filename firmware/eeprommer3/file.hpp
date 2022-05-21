#ifndef FILE_HPP
#define FILE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "touch.hpp"

// Fwd decls just in case (circular dependencies)
class TftCtrl;
class TouchCtrl;
class SdCtrl;
class FileCtrl;
class TftChoiceMenu;

/*
 * Simple little helper struct to store name of file and whether it is a directory.
 */
struct SdFileInfo {
  char name[13];
  bool is_dir;
};

/*
 * Set of flags to represent types of files.
 */
enum FileSystem : uint8_t {
  NONE       = 0x00,
  ON_SD_CARD = 0x01,
  ON_SERIAL  = 0x02,
};

/*
 * Miscellaneous file-related utilities.
 */
namespace FileUtil {
  // Go up a directory from `path`. Return whether operation was successful.
  bool go_up_dir(char *path);

  // Set `path` to "`path`/`file`". Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_file(char *path, const char *file, uint8_t len);

  // Set `path` to "`path`/`dir`/". Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_dir(char *path, const char *dir, uint8_t len);

  // Set `path` to file `sub_path` inside `path`. Return false if resulting path is `len` chars or longer, true otherwise.
  bool go_down_path(char *path, SdFileInfo *sub_path, uint8_t len);

  // Scans parameters to find which file systems are available, returns the available ones.
  uint8_t get_available_file_systems(SdCtrl &sd);
};

// TODO: add serial file support

/*
 * An ABC that can access a file from any available file system.
 */
class FileCtrl {
public:
  FileCtrl() {};
  virtual ~FileCtrl() {};

  virtual bool is_open(); // Tells whether file is open.

  virtual const char *name(); // Gets name of file.

  virtual uint16_t size(); // Gets size of file.

  virtual uint8_t read();                             // Reads a byte from file, return it.
  virtual uint16_t read(uint8_t *buf, uint16_t size); // Reads `size` bytes from file into `buf`. Returns number of bytes read.

  virtual void write(uint8_t val);                           // Writes `val` to file.
  virtual uint16_t write(const uint8_t *buf, uint16_t size); // Writes `size` bytes from `buf` to file. Returns number of bytes written.

  virtual void flush(); // Ensures that all data is written to file.

  virtual void close(); // Closes the file.

  // Creates a FileCtrl for a file with `access` at `path`. Selects file system using `fsys`.
  static FileCtrl *create_file(FileSystem fsys, const char *path, uint8_t access);
};

/*
 * Specialization of `FileCtrl` for accessing SD card files.
 */
class FileCtrlSd : public FileCtrl {
public:
  FileCtrlSd(const char *path, uint8_t access);
  ~FileCtrlSd();

  bool is_open();

  const char *name();

  uint16_t size();

  uint8_t read();
  uint16_t read(uint8_t *buf, uint16_t size);

  void write(uint8_t val);
  uint16_t write(const uint8_t *buf, uint16_t size);

  void flush();

  void close();

private:
  File m_file;
};

/*
 * Yet another `TftXXXMenu`, this one to ask user to select a file from an SD card. Inherits from `TftChoiceMenu`.
 */
class TftSdFileSelMenu : public TftChoiceMenu {
public:
  TftSdFileSelMenu(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols);
  ~TftSdFileSelMenu();

  enum Status : uint8_t {
    OK,             // No errors
    CANCELED,       // User pressed "Cancel"
    FNAME_TOO_LONG, // Filename was too long to fit in buffer
  };

  /*
   * Sets internal files buffer to the files in directory `path` on `sd`. Reads at most `max_files` files from directory.
   */
  void use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files);

  /*
   * Waits for user to select a file.
   *
   * Returns Status::CANCELED if user pressed `Cancel` button.
   * Returns Status::FNAME_TOO_LONG if resulting path is `max_path_len` chars or longer.
   *
   * If all conditions are met, return Status::OK and set `file_path` to path to file that user selected.
   */
  Status wait_for_value(TouchCtrl &tch, TftCtrl &tft, SdCtrl &sd, char *file_path, uint8_t max_path_len);

private:
  // The supplied `cols` or the maximum number of cols that will fit, whichever is smaller
  static inline uint8_t calc_num_cols(TftCtrl &tft, uint8_t cols) {
    return MIN(cols, MAX((tft.width() - 10) / (73 + 10), 1));
  }

  // Fraction 1/`rows` of allotted vertical space, constrained to at least 16 pixels
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

  SdFileInfo *m_files = nullptr;
  uint8_t m_num_files = 0;

  uint8_t m_num_rows;
};

// Ask user to select a file on SD card. Writes path into `out`. Return a Status based on user's choice.
TftSdFileSelMenu::Status ask_file(TftCtrl &tft, TouchCtrl &tch, SdCtrl &sd, const char *prompt, char *out, uint8_t len);

#endif
