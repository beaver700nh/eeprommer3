#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "sd.hpp"

#include "file.hpp"

AskFileStatus ask_file(TftCtrl &tft, TouchCtrl &tch, SdCtrl &sd, char *out, uint8_t len) {
  // Helper functions
  TftChoiceMenu *create_fname_menu(TftCtrl &tft, uint8_t rows, uint8_t cols);
  void update_fname_menu(TftMenu *menu, FileInfo *files, uint8_t num);

  AskFileStatus status = FILE_STATUS_OK;

  const uint8_t rows = 8, cols = MAX((tft.width() - 10) / (73 + 10), 1), max_files = rows * cols;
  auto *files = (FileInfo *) malloc(max_files * sizeof(FileInfo));

  char cur_dir[len + 1] = "/";

  tft.drawText(10, 10, "Files:", TftColor::CYAN, 4);
  TftChoiceMenu *menu = create_fname_menu(tft, rows, cols);

  while (true) {
    uint8_t num = sd.get_files(cur_dir, files, max_files);
    update_fname_menu(menu, files, num);

    auto val = menu->wait_for_value(tch, tft);

    if (val == menu->get_num_btns() - 2) {
      status = FILE_STATUS_CANCELED;
      break;
    }
    else if (val == menu->get_num_btns() - 3) {
      go_up_dir(cur_dir);
      continue;
    }

    // User selected a file, not a control button

    PRINTF_NOBUF(Serial, "Info:\n\tcur_dir: %s,\n\tfname: %s\n", cur_dir, files[val].name);

    if (!go_down_path(out, files + val, len)) {
      status = FILE_STATUS_FNAME_TOO_LONG;
    }

    // If the file was a regular file, user has selected the needed file, done
    if (!files[val].is_dir) {
      break;
    }
  }

  delete menu;
  free(files);

  return status;
}

// Remember to delete returned menu after use!
TftChoiceMenu *create_fname_menu(TftCtrl &tft, uint8_t rows, uint8_t cols) {
  auto *menu = new TftChoiceMenu(6, 6, 50, 10, cols, 18, true);

  for (uint8_t j = 0; j < rows; ++j) {
    for (uint8_t i = 0; i < cols; ++i) {
      menu->add_btn_calc(tft, "", TftColor::BLACK, TftColor::WHITE);
      menu->get_btn(menu->get_num_btns() - 1)->set_font_size(1);
    }
  }

  uint16_t _y = TftCalc::bottom(tft, 24, 44), _w = TftCalc::fraction_x(tft, 10, 2);

  menu->add_btn(new TftBtn(10,      _y, _w, 24, "Parent Dir"));
  menu->add_btn(new TftBtn(20 + _w, _y, _w, 24, "Cancel"));
  menu->add_btn_confirm(tft, true);

  return menu;
}

void update_fname_menu(TftMenu *menu, FileInfo *files, uint8_t num) {
  uint8_t i;
  PRINTF_NOBUF(Serial, "num: %d\n", num);

  // Update and enable all the needed buttons.
  for (i = 0; i < num; ++i) {
    menu->get_btn(i)->visibility(true);
    menu->get_btn(i)->operation(true);
    menu->get_btn(i)->set_text(files[i].name);

    PRINTF_NOBUF(Serial, "Enabled button %d.\n", i);
  }

  // Disable everything else except `Confirm` button.
  for (/* no init clause */; i < menu->get_num_btns() - 3; ++i) {
    menu->get_btn(i)->visibility(false);
    menu->get_btn(i)->operation(false);

    PRINTF_NOBUF(Serial, "Disabled button %d.\n", i);
  }
}

bool go_up_dir(char *path) {
  // Empty path is treated as root dir
  if (strlen(path) == 0) return false;

  bool success = true;

  char *_path = strdup(path);
  char *end = _path + strlen(_path);

  // Remove trailing '/' from copy of `path`
  *(end - 1) = '\0';

  char *new_end = strrchr(_path, '/');

  if (new_end == nullptr) {
    // `path` was nothing but a slash, there is nothing to go up into
    success = false;
  }
  else {
    memset(_path, '\0', end - new_end - 2);
    strcpy(path, _path);
  }

  free(_path);
  return success;
}

bool go_down_file(char *path, const char *file, uint8_t len) {
  if (strlen(path) + strlen(file) >= len) {
    // Doesn't fit, fail
    return false;
  }

  strcat(path, file);
  return true;
}

bool go_down_dir(char *path, const char *dir, uint8_t len) {
  // Paste a trailing slash onto `dir`
  auto *temp = (char *) malloc((strlen(dir) + 1) * sizeof(char));
  strcpy(temp, dir);
  strcat(temp, "/");

  // Delegates to go_down_file but passes `dir` with a trailing slash
  bool result = go_down_file(path, temp, len);

  free(temp);

  return result;
}

bool go_down_path(char *path, FileInfo *sub_path, uint8_t len) {
  // Delegate to go_down_file() or go_down_dir()
  return (sub_path->is_dir ? go_down_dir : go_down_file)(path, sub_path->name, len);
}
