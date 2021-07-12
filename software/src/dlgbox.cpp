#include "dlgbox.hpp"
#include "wx_dep.hpp"

void DlgBox::error(wxString message, wxString title, int type) {
  wxMessageBox(LENGTHEN_MSG(message), "ERROR | " + title, type | wxICON_ERROR);
}

void DlgBox::warn(wxString message, wxString title, int type) {
  wxMessageBox(LENGTHEN_MSG(message), "WARNING | " + title, type | wxICON_EXCLAMATION);
}

void DlgBox::info(wxString message, wxString title, int type) {
  wxMessageBox(LENGTHEN_MSG(message), "NOTE | " + title, type | wxICON_INFORMATION);
}
