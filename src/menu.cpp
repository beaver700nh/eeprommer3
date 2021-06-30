#include <string>
#include <unordered_map>

#include "menu.hpp"
#include "wx_dep.hpp"

MenuBar::MenuBar(std::unordered_map<std::string, int> ids) {
  menu_bar = new wxMenuBar();

  file_menu = new wxMenu();
  file_menu->Append(ids.at("open"), "Open\tCtrl-O", "Open an existing file.");
  file_menu->Append(ids.at("save"), "Save\tCtrl-S", "Save to currently open file.");

  tools_menu = new wxMenu();
  tools_menu->Append(ids.at("read"),   "Read\tAlt-R",       "Read data from the EEPROM.");
  tools_menu->Append(ids.at("write"),  "Write\tAlt-W",      "Write data to the EEPROM.");
  tools_menu->Append(ids.at("vector"), "Set Vector\tAlt-V", "Set a 6502 jump vector.");

  actions_menu = new wxMenu();
  actions_menu->Append(ids.at("about"), "About\tF2",    "Shows information about eeprommer3.");
  actions_menu->Append(ids.at("help"),  "Help\tF1",     "Shows a help screen.");
  actions_menu->Append(ids.at("clear"), "Clear\tAlt-C", "Clear the hex display.");
  actions_menu->Append(ids.at("exit"),  "Quit\tCtrl-Q", "Quit eeprommer3.");

  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");
}

wxMenuBar *MenuBar::get_wxMenuBar() {
  return menu_bar;
}
