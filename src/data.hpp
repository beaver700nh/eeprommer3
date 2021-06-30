#ifndef DATA_HPP
#define DATA_HPP

#include "wx_dep.hpp"

class HexData {
public:
  HexData() {};
  HexData(
    wxPanel *panel, const wxSize &cell_size,
    wxFont &header_font, wxFont &normal_font
  );

  void setup_hilo();
  void setup_headers();
  void setup_data();

  void open_file(wxString fname);
  void save_file(wxString fname);
  void clear_hex();

private:
  wxPanel *panel;
  wxSize cell_size;

  wxFont header_font, normal_font;

  wxStaticText *data[16][16];
};

#endif
