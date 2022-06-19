#ifndef FILE_HPP
#define FILE_HPP

#include <Arduino.h>
#include "constants.hpp"

#include "gui.hpp"
#include "sd.hpp"
#include "tft.hpp"
#include "tft_calc.hpp"
#include "tft_util.hpp"
#include "touch.hpp"

// Fwd decls just in case (circular dependencies)
class TftCtrl;
class TouchCtrl;
class SdCtrl;
class FileCtrl;
class TftChoiceMenu;

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

  virtual bool is_open();

  virtual const char *name();

  virtual uint16_t size();

  virtual uint8_t read();
  virtual uint16_t read(uint8_t *buf, uint16_t size);  // Reads `size` bytes from file into `buf`. Returns number of bytes read.

  virtual void write(uint8_t val);
  virtual uint16_t write(const uint8_t *buf, uint16_t size);  // Writes `size` bytes from `buf` to file. Returns number of bytes written.

  virtual void flush();  // Ensures that all data is written to file.

  virtual void close();

  // Creates a FileCtrl for a file with `access` at `path`. Selects file system using `fsys`.
  static FileCtrl *create_file(FileSystem fsys, const char *path, uint8_t access);

  // Checks if a file was opened correctly
  static bool check_valid(FileCtrl *file);

  FileSystem fsys = FileSystem::NONE;
};

/*
 * Specialization of `FileCtrl` for accessing SD card files.
 */
class FileCtrlSd : public FileCtrl {
public:
  FileCtrlSd(const char *path, uint8_t access);
  ~FileCtrlSd() override;

  bool is_open() override;

  const char *name() override;

  uint16_t size() override;

  uint8_t read() override;
  uint16_t read(uint8_t *buf, uint16_t size) override;

  void write(uint8_t val) override;
  uint16_t write(const uint8_t *buf, uint16_t size) override;

  void flush() override;

  void close() override;

private:
  File m_file;
};

namespace Gui {

/*
 * Yet another `MenuXXX`, this one to ask user to select a file from an SD card.
 */
class MenuSdFileSel : public MenuChoice {
public:
  MenuSdFileSel(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols);
  ~MenuSdFileSel();

  enum Status : uint8_t {
    OK,              // No errors
    CANCELED,        // User pressed "Cancel"
    FNAME_TOO_LONG,  // Filename was too long to fit in buffer
  };

  /*
   * Sets internal files buffer to the files in directory `path` on `sd`. Reads at most `max_files` files from directory.
   */
  void use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files);

  /*
   * Waits for user to select a file.
   * Returns Status::CANCELED if `Cancel` button pressed; Status::FNAME_TOO_LONG if path len >= `max_path_len`.
   * Returns Status::OK if everything fine; sets `file_path` = path to selected file.
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
        tft.height()    // take up as much space as possible
          - marg_v      // without the top margin
          - 78          // without the bottom margin
          + 2 * pad_v,  // but still need some amount
        pad_v, rows
      )
    );
  }

  SdFileInfo *m_files = nullptr;
  uint8_t m_num_files = 0;

  uint8_t m_num_rows;
};

};

namespace Dialog {

enum AskFileStatus : uint8_t {
  OK,              // No errors
  CANCELED,        // User pressed "Cancel"
  FNAME_TOO_LONG,  // Filename was too long to fit in buffer
  FSYS_INVALID,    // Selected filesystem does not exist
};

// Asks user for path to any file on any available file system. Puts resulting status into `status`. Returns a FileCtrl * for the file.
FileCtrl *ask_file(TftCtrl &tft, TouchCtrl &tch, const char *prompt, uint8_t access, AskFileStatus *status, bool must_exist, SdCtrl &sd);

// Asks user for a path to a file on SD card. Writes path into `out`. Returns a status based on user's choice.
AskFileStatus ask_fpath_sd(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *out, uint8_t len, bool must_exist, SdCtrl &sd);

// Asks user to select an existing file on SD card. Writes path into `out`. Returns a status based on user's choice.
Gui::MenuSdFileSel::Status ask_sel_fpath_sd(TftCtrl &tft, TouchCtrl &tch, const char *prompt, char *out, uint8_t len, SdCtrl &sd);

// Asks user to select a file system out of all the ones that are detected.
FileSystem ask_fsys(TftCtrl &tft, TouchCtrl &tch, const char *prompt, SdCtrl &sd);

};

#endif
