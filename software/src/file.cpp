#include <cstdlib>
#include <fstream>

#include "file.hpp"
#include "util.hpp"
#include "wx_dep.hpp"

FileIO_Status FileIO::open_file(wxString f, HexData &hd, wxFrame *parent) {
  auto fname = wxLoadFileSelector("Choose a file to open", FILE_WILDCARD, f, parent);

  if (fname.empty()) {
    wxMessageBox("Couldn't open file.", "Error", wxOK, parent);
    return std::make_tuple(false, f);
  }

  std::ifstream file(fname);
  if (!file) return std::make_tuple(false, fname);

  hd.set_data(
    [&](uint8_t i, uint8_t j) -> wxString {
      // Write the next byte from file
      char val = file.get();
      return (file.eof() ? "??" : wxString::Format("%02x", (uint8_t) val));
    }
  );

  file.close();
  if (file.bad() || file.fail()) return std::make_tuple(false, fname);

  return std::make_tuple(true, fname);
}

FileIO_Status FileIO::save_file(wxString f, HexData &hd, wxFrame *parent) {
  auto fname = wxSaveFileSelector("Choose a file name to save as", FILE_WILDCARD, f, parent);

  if (fname.empty()) {
    wxMessageBox("Couldn't save file.", "Error", wxOK, parent);
    return std::make_tuple(false, f);
  }

  uint8_t arr[16][16];
  uint16_t count = hd.get_data(&arr);

  uint8_t *temp = (uint8_t *) malloc(sizeof(uint8_t) * 256);

  if (temp == nullptr) return std::make_tuple(false, fname);

  // Flatten arr into temp
  flatten_2d21d<uint8_t, 16, 16>(&arr, &temp, 4);

  // Write temp into file
  std::ofstream file(fname);
  if (!file) return std::make_tuple(false, fname);
  file.write((char *) temp, count);
  if (file.bad() || file.fail()) return std::make_tuple(false, fname);
  file.close();
  if (file.bad() || file.fail()) return std::make_tuple(false, fname);

  free(temp);

  return std::make_tuple(true, fname);
}
