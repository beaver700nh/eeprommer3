#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <unordered_map>

#include "wx_dep.hpp"

class MenuBar {
public:
  MenuBar() {};
  MenuBar(std::unordered_map<std::string, int> ids);

  wxMenuBar *get_wxMenuBar();

private:
  wxMenuBar *menu_bar;
  wxMenu *file_menu, *tools_menu, *actions_menu;
};

#endif
