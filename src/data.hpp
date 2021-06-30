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

  void setup_gui();

  void setup_hilo();
  void setup_headers();
  void setup_data();

  void set_data(wxString (*fn)(uint8_t i, uint8_t j));
  void get_data(wxString (*out)[16][16]);

  void set_data(uint8_t i, uint8_t j, wxString v);
  wxString get_data(uint8_t i, uint8_t j);

  void for_each(void (*fn)(uint8_t i, uint8_t j, wxString v));

private:
  wxPanel *panel;
  wxSize cell_size;

  wxFont header_font, normal_font;

  wxStaticText *data[16][16];
};

#endif
