#ifndef DLGBOX_HPP
#define DLGBOX_HPP

#include "wx_dep.hpp"

#define LENGTHEN_MSG_STR_A "~~~~~~~~<eeprommer3>~~~~~~~~"
#define LENGTHEN_MSG_STR_B "~~~~~~~~</eeprommer3>~~~~~~~~"
#define LENGTHEN_MSG_STR   LENGTHEN_MSG_STR_A

#define LENGTHEN_MSG(msg) \
  wxString("") + LENGTHEN_MSG_STR_A + "\n" + msg + "\n" + LENGTHEN_MSG_STR_B

namespace DlgBox {
  void error(wxString message, wxString title, int type);
  void warn (wxString message, wxString title, int type);
  void info (wxString message, wxString title, int type);
}

#endif
