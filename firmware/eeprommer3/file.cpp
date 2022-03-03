#include <Arduino.h>
#include "constants.hpp"

#include "tft.hpp"
#include "sd.hpp"

#include "file.hpp"

AskFileStatus ask_file(TftCtrl &tft, TouchCtrl &tch, SdCtrl &sd, char *out, uint8_t len, const char *prompt) {
  const uint8_t rows = 6, cols = 6;

  tft.drawText(10, 10, prompt, TftColor::CYAN, 4);

  TftFileSelMenu menu(tft, 10, 10, 50, 10, rows, cols);

  return menu.wait_for_value(tch, tft, sd, out, len);
}

TftFileSelMenu::TftFileSelMenu(TftCtrl &tft, uint8_t pad_v, uint8_t pad_h, uint8_t marg_v, uint8_t marg_h, uint8_t rows, uint8_t cols)
  : TftChoiceMenu(pad_v, pad_h, marg_v, marg_h, calc_num_cols(tft, cols), calc_btn_height(tft, rows, marg_v, pad_v), true) {
  m_num_rows = rows;

  for (uint8_t j = 0; j < m_num_rows; ++j) {
    for (uint8_t i = 0; i < m_num_cols; ++i) {
      add_btn_calc(tft, "", TftColor::BLACK, TftColor::WHITE);
      get_btn(get_num_btns() - 1)->set_font_size(1);
    }
  }

  uint16_t _y = TftCalc::bottom(tft, 24, 44), _w = TftCalc::fraction_x(tft, 10, 2);

  add_btn(new TftBtn(10,      _y, _w, 24, "Parent Dir", TftColor::CYAN,  TftColor::BLUE));
  add_btn(new TftBtn(20 + _w, _y, _w, 24, "Cancel",     TftColor::PINKK, TftColor::RED ));
  add_btn_confirm(tft, true);

  m_files = (FileInfo *) malloc(m_num_rows * m_num_cols * sizeof(FileInfo));
}

TftFileSelMenu::~TftFileSelMenu() {
  free(m_files);
}

void TftFileSelMenu::use_files_in_dir(SdCtrl &sd, const char *path, uint8_t max_files) {
  uint8_t i;

  m_num_files = sd.get_files(path, m_files, max_files);

  // Update and enable all the needed buttons.
  for (i = 0; i < m_num_files; ++i) {
    get_btn(i)->visibility(true);
    get_btn(i)->operation(true);
    get_btn(i)->set_text(m_files[i].name);

    if (m_files[i].is_dir) {
      get_btn(i)->set_bg(TftColor::CYAN);
    }
    else {
      get_btn(i)->set_bg(TftColor::ORANGE);
    }
  }

  // Disable everything else except the control buttons.
  for (/* no init clause */; i < get_num_btns() - 3; ++i) {
    get_btn(i)->visibility(false);
    get_btn(i)->operation(false);
  }
}

AskFileStatus TftFileSelMenu::wait_for_value(TouchCtrl &tch, TftCtrl &tft, SdCtrl &sd, char *file_path, uint8_t max_path_len) {
  char cur_path[max_path_len + 1] = "/";

  while (true) {
    tft.fillRect(0, m_marg_v, tft.width(), tft.height() - m_marg_v, TftColor::BLACK);

    deselect_all();
    select(0);

    use_files_in_dir(sd, cur_path, m_num_rows * m_num_cols);
    uint8_t btn_id = TftChoiceMenu::wait_for_value(tch, tft);

    if (btn_id == get_num_btns() - 2) {
      return FILE_STATUS_CANCELED;
    }

    if (btn_id == get_num_btns() - 3) {
      FileUtil::go_up_dir(cur_path);
      continue;
    }

    if (strlen(cur_path) + strlen(m_files[btn_id].name) >= max_path_len) return FILE_STATUS_FNAME_TOO_LONG;

    // User selected a file, not a control button

    if (!FileUtil::go_down_path(cur_path, m_files + btn_id, max_path_len)) {
      return FILE_STATUS_FNAME_TOO_LONG;
    }

    // If the file was a regular file, user has selected the needed file, done
    if (!m_files[btn_id].is_dir) {
      strncpy(file_path, cur_path, max_path_len);
      return FILE_STATUS_OK;
    }
  }
}

namespace FileUtil {
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
      success = false;
    }
    else {
      memset(new_end + 1, '\0', end - new_end - 2);
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
};
