#include <cstdio>

#include "main.hpp"
#include "wx_dep.hpp"

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() {
  const wxSize win_size = wxSize(560, 460);

  MainFrame *win = new MainFrame(
    "eeprommer3 - AT28Cxxx Programmer",
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
  SetStatusText("Hello, world!");

  menu_bar = MenuBar();
  SetMenuBar(menu_bar.get_wxMenuBar());

  static wxPanel *panel = new wxPanel(this);
  hex_data = HexData(panel, wxSize(32, 22), header_font, normal_font);
  hex_data.setup_gui();
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN,  MainFrame::OnMenuFileOpen)
EVT_MENU(wxID_SAVE,  MainFrame::OnMenuFileSave)
EVT_MENU(ID_READ,    MainFrame::OnMenuToolsRead)
EVT_MENU(ID_WRITE,   MainFrame::OnMenuToolsWrite)
EVT_MENU(ID_VECTOR,  MainFrame::OnMenuToolsVector)
EVT_MENU(ID_PORT,    MainFrame::OnMenuToolsPort)
EVT_MENU(wxID_ABOUT, MainFrame::OnMenuActionsAbout)
EVT_MENU(ID_HELP,    MainFrame::OnMenuActionsHelp)
EVT_MENU(wxID_CLEAR, MainFrame::OnMenuActionsClear)
EVT_MENU(wxID_EXIT,  MainFrame::OnMenuActionsQuit)
END_EVENT_TABLE()
