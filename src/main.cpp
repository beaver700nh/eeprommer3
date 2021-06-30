#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "main.hpp"
#include "wx_dep.hpp"

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() {
  const wxSize win_size = wxSize(560, 460);

  MainFrame *win = new MainFrame(
    "eeprommer3 - AT28C256 Programmer",
    wxDefaultPosition, win_size
  );

  win->Show(true);
  SetTopWindow(win);

  return true;
}

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame *) nullptr, -1, title, pos, size) {
  normal_font = wxFont(10, wxMODERN, wxSLANT,  wxLIGHT, false, "", wxFONTENCODING_DEFAULT);
  header_font = wxFont(13, wxMODERN, wxNORMAL, wxBOLD,  false, "", wxFONTENCODING_DEFAULT);

  CreateStatusBar(1);
  SetStatusText("Hello, world!", 0);

  static wxPanel *panel = new wxPanel(this);
  hex_data = HexData(panel, wxSize(32, 22), header_font, normal_font);

  add_menu();
  add_contents();
}

void MainFrame::add_menu() {
  menu_bar = new wxMenuBar();

  file_menu = new wxMenu();
  file_menu->Append(wxID_OPEN, "Open\tCtrl-O", "Open an existing file.");
  file_menu->Append(wxID_SAVE, "Save\tCtrl-S", "Save to currently open file.");

  tools_menu = new wxMenu();
  tools_menu->Append(ID_READ,   "Read\tAlt-R",       "Read data from the EEPROM.");
  tools_menu->Append(ID_WRITE,  "Write\tAlt-W",      "Write data to the EEPROM.");
  tools_menu->Append(ID_VECTOR, "Set Vector\tAlt-V", "Set a 6502 jump vector.");

  actions_menu = new wxMenu();
  actions_menu->Append(wxID_ABOUT, "About\tF2",    "Shows information about eeprommer3.");
  actions_menu->Append(ID_HELP,    "Help\tF1",     "Shows a help screen.");
  actions_menu->Append(ID_CLEAR,   "Clear\tAlt-C", "Clear the hex display.");
  actions_menu->Append(wxID_EXIT,  "Quit\tCtrl-Q", "Quit eeprommer3.");

  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");

  SetMenuBar(menu_bar);
}

void MainFrame::add_contents() {
  hex_data.setup_hilo();
  hex_data.setup_headers();
  hex_data.setup_data();
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN,  MainFrame::OnMenuFileOpen)
EVT_MENU(wxID_SAVE,  MainFrame::OnMenuFileSave)
EVT_MENU(ID_READ,    MainFrame::OnMenuToolsRead)
EVT_MENU(ID_WRITE,   MainFrame::OnMenuToolsWrite)
EVT_MENU(ID_VECTOR,  MainFrame::OnMenuToolsVector)
EVT_MENU(wxID_ABOUT, MainFrame::OnMenuActionsAbout)
EVT_MENU(ID_HELP,    MainFrame::OnMenuActionsHelp)
EVT_MENU(ID_CLEAR,   MainFrame::OnMenuActionsClear)
EVT_MENU(wxID_EXIT,  MainFrame::OnMenuActionsQuit)
END_EVENT_TABLE()
