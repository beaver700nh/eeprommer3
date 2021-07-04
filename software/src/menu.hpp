#ifndef MENU_HPP
#define MENU_HPP

#include "wx_dep.hpp"

enum {
  ID_READ = 300,
  ID_WRITE,
  ID_VECTOR,
  ID_PORT,
  ID_HELP,
};

class MenuBar {
public:
  MenuBar();

  wxMenuBar *get_wxMenuBar();

private:
  wxMenuBar *menu_bar;
  wxMenu *file_menu, *tools_menu, *actions_menu;
};

#endif
