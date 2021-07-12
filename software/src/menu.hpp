#ifndef MENU_HPP
#define MENU_HPP

#include "wx_dep.hpp"

enum {
  ID_READ = 300,
  ID_WRITE,
  ID_VECTOR,
  ID_PORT,
  ID_HELP,
  ID_TEST_READ,
  ID_TEST_WRITE,
};

class MenuBar {
public:
  MenuBar();

  wxMenuBar *get_menu_bar();

private:
  wxMenuBar *menu_bar;
  wxMenu *file_menu, *tools_menu, *actions_menu, *debug_menu;
};

#endif
