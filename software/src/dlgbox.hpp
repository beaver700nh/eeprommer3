#ifndef DLGBOX_HPP
#define DLGBOX_HPP

#include "wx_dep.hpp"

namespace DlgBox {
  void error(wxString message, wxString title, int type);
  void warn (wxString message, wxString title, int type);
  void info (wxString message, wxString title, int type);
}

#endif
