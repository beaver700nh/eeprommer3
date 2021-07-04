#ifndef FILE_HPP
#define FILE_HPP

#include <tuple>

#include "data.hpp"
#include "wx_dep.hpp"

#define FILE_WILDCARD "BIN file (*.bin)|*.bin|HEX file (*.hex)|*.hex"

typedef std::tuple<bool, wxString> FileIO_Status;

class FileIO {
public:
  static FileIO_Status open_file(wxString f, HexData &hd, wxFrame *parent);
  static FileIO_Status save_file(wxString f, HexData &hd, wxFrame *parent);
};

#endif
