#ifndef DATA_HPP
#define DATA_HPP

#include <functional>

#include "wx_dep.hpp"

#define SETDATA_LAMBDA      (uint8_t i, uint8_t j) -> wxString
#define SETDATA_LAMBDA_TYPE std::function<wxString(uint8_t, uint8_t)>

#define FOREACH_LAMBDA      (uint8_t i, uint8_t j, wxStaticText *d) -> wxString
#define FOREACH_LAMBDA_TYPE std::function<wxString(uint8_t, uint8_t, wxStaticText *)>

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

  void set_data(wxString (*in)[16][16], uint16_t count = 0x100);
  void get_data(wxString (*out)[16][16]);

  void set_data(uint8_t (*in)[16][16], uint16_t count = 0x100);
  uint16_t get_data(uint8_t (*out)[16][16]);

  void set_data(uint8_t i, uint8_t j, wxString v);
  wxString get_data(uint8_t i, uint8_t j);

  void set_data(std::function<wxString(uint8_t, uint8_t)> fn);
  void for_each(FOREACH_LAMBDA_TYPE fn);

private:
  wxPanel *panel;
  wxSize cell_size;

  wxFont header_font, normal_font;

  wxStaticText *data[16][16];
};

#endif
