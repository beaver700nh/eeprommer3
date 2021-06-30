#include <cstdint>
#include <functional>

#include "data.hpp"
#include "main.hpp"
#include "wx_dep.hpp"

HexData::HexData(
  wxPanel *panel, const wxSize &cell_size,
  wxFont &header_font, wxFont &normal_font
) {
  this->panel = panel;
  this->cell_size = cell_size;

  this->header_font = header_font;
  this->normal_font = normal_font;
}

void HexData::setup_gui() {
  setup_hilo();
  setup_headers();
  setup_data();
}

void HexData::setup_hilo() {
  wxStaticText *hi = new wxStaticText(panel, wxID_ANY, "H", wxPoint(13, 5), cell_size);
  wxStaticText *lo = new wxStaticText(panel, wxID_ANY, "L", wxPoint(25, 2), cell_size);

  hi->SetFont(normal_font);
  lo->SetFont(normal_font);
}

void HexData::setup_headers() {
  wxStaticText *row_hdrs[16], *col_hdrs[16];

  for (int i = 0; i < 16; ++i) {
    wxPoint rhp = wxPoint(10, (i + 1) * cell_size.GetHeight());
    wxPoint chp = wxPoint((i + 1) * cell_size.GetWidth() + 10, 0);

    row_hdrs[i] = new wxStaticText(panel, wxID_ANY, wxString::Format("%x0", i), rhp, cell_size);
    col_hdrs[i] = new wxStaticText(panel, wxID_ANY, wxString::Format("0%x", i), chp, cell_size);

    row_hdrs[i]->SetFont(header_font);
    col_hdrs[i]->SetFont(header_font);
  }
}

void HexData::setup_data() {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      wxPoint pos = wxPoint(
        (j + 1) * cell_size.GetWidth() + 12,
        (i + 1) * cell_size.GetHeight() + 2
      );

      data[i][j] = new wxStaticText(panel, DATA_IDS + i*16 + j, "??", pos, cell_size);
      data[i][j]->SetFont(normal_font);
    }
  }
}

void HexData::set_data(wxString (*in)[16][16], uint16_t count) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      if (((i << 4) | j) >= (count - 1)) {
        return;
      }

      data[i][j]->SetLabel((*in)[i][j]);
    }
  }
}

void HexData::get_data(wxString (*out)[16][16]) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      (*out)[i][j] = data[i][j]->GetLabel();
    }
  }
}

void HexData::set_data(uint8_t (*in)[16][16], uint16_t count) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      if (((i << 4) | j) >= (count - 1)) {
        return;
      }

      data[i][j]->SetLabel(wxString::Format("%d", (*in)[i][j]));
    }
  }
}

uint16_t HexData::get_data(uint8_t (*out)[16][16]) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      long temp;

      if (data[i][j]->GetLabel().ToLong(&temp, 16)) {
        (*out)[i][j] = (uint8_t) temp;
      }
      else {
        return (i << 4) | j;
      }
    }
  }

  return 0x100;
}

void HexData::set_data(uint8_t i, uint8_t j, wxString v) {
  data[i][j]->SetLabel(v);
}

wxString HexData::get_data(uint8_t i, uint8_t j) {
  return data[i][j]->GetLabel();
}

void HexData::set_data(std::function<wxString(uint8_t, uint8_t)> fn) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      data[i][j]->SetLabel(fn(i, j));
    }
  }
}

void HexData::for_each(std::function<wxString(uint8_t, uint8_t, wxStaticText *)> fn) {
  for (uint8_t i = 0; i < 16; ++i) {
    for (uint8_t j = 0; j < 16; ++j) {
      wxString temp = fn(i, j, data[i][j]);

      if      (temp == "return")  return;
      else if (temp == "nextrow") break;
      else if (temp == "break")   break;
    }
  }
}
