#include <cstdio>

#include "comm.hpp"
#include "dlgbox.hpp"
#include "main.hpp"
#include "menu.hpp"
#include "wx_dep.hpp"

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() {
  const auto win_size = wxSize(560, 460);

  auto *win = new MainFrame(
    "eeprommer3 - AT28Cxxx Programmer",
    wxDefaultPosition, win_size
  );

  win->CenterOnScreen();
  win->Show(true);
  SetTopWindow(win);

  return true;
}

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame *) nullptr, wxID_ANY, title, pos, size) {
  // Initialize the fonts
  normal_font = wxFont(10, wxMODERN, wxSLANT,  wxLIGHT, false, "", wxFONTENCODING_DEFAULT);
  header_font = wxFont(13, wxMODERN, wxNORMAL, wxBOLD,  false, "", wxFONTENCODING_DEFAULT);

  CreateStatusBar(1);
  SetStatusText("Hello, world!");

  menu_bar = MenuBar();
  SetMenuBar(menu_bar.get_menu_bar());

  static auto *panel = new wxPanel(this);
  hex_data = HexData(panel, wxSize(32, 22), header_font, normal_font);
  hex_data.setup_gui();

  // Default port config for comm with arduino is
  // 115200 baud, 8N1 bits, and no flow control
  port_config = PortConfig(115200, 8, SP_PARITY_NONE, 1, SP_FLOWCONTROL_NONE);

  CenterOnScreen();
}

void MainFrame::crash(wxString message, wxString title, int type) {
  DlgBox::error(message, "FATAL ERROR | " + title, type);
  Close(true);
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN,     MainFrame::OnMenuFileOpen)
EVT_MENU(wxID_SAVE,     MainFrame::OnMenuFileSave)
EVT_MENU(ID_READ,       MainFrame::OnMenuToolsRead)
EVT_MENU(ID_WRITE,      MainFrame::OnMenuToolsWrite)
EVT_MENU(ID_VECTOR,     MainFrame::OnMenuToolsVector)
EVT_MENU(ID_PORT,       MainFrame::OnMenuToolsPort)
EVT_MENU(ID_HELP,       MainFrame::OnMenuActionsHelp)
EVT_MENU(wxID_ABOUT,    MainFrame::OnMenuActionsAbout)
EVT_MENU(wxID_CLEAR,    MainFrame::OnMenuActionsClear)
EVT_MENU(wxID_EXIT,     MainFrame::OnMenuActionsQuit)
EVT_MENU(ID_TEST_READ,  MainFrame::OnMenuDebugTestRead)
EVT_MENU(ID_TEST_WRITE, MainFrame::OnMenuDebugTestWrite)
END_EVENT_TABLE()
