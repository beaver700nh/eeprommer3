#ifndef INFO_HPP
#define INFO_HPP

#include "wx_dep.hpp"

class AppInfo {
public:
  static void help_dlg(wxFrame *parent);
  static void about_dlg();

  static inline auto png_logo_wxicon = wxIcon(
    "../../eeprommer3.png", wxBITMAP_TYPE_PNG_RESOURCE, 64, 64
  );
};

#endif
