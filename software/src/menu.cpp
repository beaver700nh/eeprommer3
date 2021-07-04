#include "menu.hpp"
#include "wx_dep.hpp"

MenuBar::MenuBar() {
  menu_bar = new wxMenuBar();

  file_menu = new wxMenu();
  file_menu->Append(wxID_OPEN, "Open\tCtrl-O", "Open an existing file.");
  file_menu->Append(wxID_SAVE, "Save\tCtrl-S", "Save to currently open file.");

  tools_menu = new wxMenu();
  tools_menu->Append(ID_READ,   "Read\tAlt-R",       "Read data from the EEPROM.");
  tools_menu->Append(ID_WRITE,  "Write\tAlt-W",      "Write data to the EEPROM.");
  tools_menu->Append(ID_VECTOR, "Set Vector\tAlt-V", "Set a 6502 jump vector.");
  tools_menu->Append(ID_PORT,   "Port\tAlt-P",       "Select the port with the hardware.");

  actions_menu = new wxMenu();
  actions_menu->Append(wxID_ABOUT, "About\tF2",    "Shows information about eeprommer3.");
  actions_menu->Append(ID_HELP,    "Help\tF1",     "Shows a help screen.");
  actions_menu->Append(wxID_CLEAR, "Clear\tAlt-C", "Clear the hex display.");
  actions_menu->Append(wxID_EXIT,  "Quit\tCtrl-Q", "Quit eeprommer3.");

  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");
}

wxMenuBar *MenuBar::get_wxMenuBar() {
  return menu_bar;
}
