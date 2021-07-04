#include <cstdlib>
#include <fstream>

#include "file.hpp"
#include "wx_dep.hpp"

FileIO_Status FileIO::open_file(wxString f, HexData &hd, wxFrame *parent) {
  wxString fname = wxLoadFileSelector("Choose a file to open", FILE_WILDCARD, f, parent);

  if (fname.empty()) {
    wxMessageBox("Couldn't open file.", "Error", wxOK, parent);
    return std::make_tuple(false, "");
  }

  std::ifstream file(fname);

  hd.set_data(
    [&](uint8_t i, uint8_t j) -> wxString {
      char val = file.get();
      return (file.eof() ? "??" : wxString::Format("%02x", (uint8_t) val));
    }
  );

  file.close();

  return std::make_tuple(true, fname);
}

FileIO_Status FileIO::save_file(wxString f, HexData &hd, wxFrame *parent) {
  wxString fname = wxSaveFileSelector("Choose a file name to save as", FILE_WILDCARD, f, parent);

  if (fname.empty()) {
    wxMessageBox("Couldn't save file.", "Error", wxOK, parent);
    return std::make_tuple(false, "");
  }

  std::ofstream file(fname);

  uint8_t arr[16][16];
  uint16_t count = hd.get_data(&arr);

  uint8_t *temp = (uint8_t *) malloc(sizeof(uint8_t) * 256);

  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      temp[(i << 4) | j] = arr[i][j];
    }
  }

  file.write((char *) temp, count);
  file.close();

  free(temp);

  return std::make_tuple(true, fname);
}
