#include "menu.hpp"
#include "wx_dep.hpp"

MenuBar::MenuBar() {
  // Initialize the menu bar
  menu_bar = new wxMenuBar();

  // Initialize each of the menus

  // [File] menu
  file_menu = new wxMenu();
  file_menu->Append(wxID_OPEN, "Open\tCtrl-O", "Open an existing file.");
  file_menu->Append(wxID_SAVE, "Save\tCtrl-S", "Save to currently open file.");

  // [Tools] menu
  tools_menu = new wxMenu();
  tools_menu->Append(ID_READ,   "Read\tAlt-R",       "Read data from the EEPROM.");
  tools_menu->Append(ID_WRITE,  "Write\tAlt-W",      "Write data to the EEPROM.");
  tools_menu->Append(ID_VECTOR, "Set Vector\tAlt-V", "Set a 6502 jump vector.");
  tools_menu->Append(ID_PORT,   "Port\tAlt-P",       "Select the port with the hardware.");

  // [Actions] menu
  actions_menu = new wxMenu();
  actions_menu->Append(ID_HELP,    "Help\tF1",     "Shows a help screen.");
  actions_menu->Append(wxID_ABOUT, "About\tF2",    "Shows information about eeprommer3.");
  actions_menu->Append(wxID_CLEAR, "Clear\tAlt-C", "Clear the hex display.");
  actions_menu->Append(wxID_EXIT,  "Quit\tCtrl-Q", "Quit eeprommer3.");

  // [Debug] menu
  debug_menu = new wxMenu();
  debug_menu->Append(ID_TEST_READ,  "Test Read\tAlt-Shift-R",  "Test reading using serialcxx.");
  debug_menu->Append(ID_TEST_WRITE, "Test Write\tAlt-Shift-W", "Test writing using serialcxx.");

  // Put the menus into the menu bar
  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");
  menu_bar->Append(debug_menu,   "Debug");
}

wxMenuBar *MenuBar::get_menu_bar() {
  return menu_bar;
}
