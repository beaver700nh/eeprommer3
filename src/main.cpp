#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "main.hpp"

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() {
  MainFrame *win = new MainFrame("Hello, world!", wxDefaultPosition, wxSize(600, 500));
  win->Show(true);
  SetTopWindow(win);

  return true;
}

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame *) nullptr, -1, title, pos, size) {
  CreateStatusBar(1);
  SetStatusText("Hello, world!", 0);

  menu_bar = new wxMenuBar();

  file_menu = new wxMenu();
  file_menu->Append(wxID_OPEN, "Open\tCtrl-O",          "Open an existing file.");
  file_menu->Append(wxID_SAVE, "Save\tCtrl-S",          "Save to currently open file.");
  file_menu->Append(ID_SAVEAS, "Save As\tCtrl-Shift-S", "Save this file under another name.");

  tools_menu = new wxMenu();
  tools_menu->Append(ID_READ,   "Read\tAlt-R",       "Read data from the EEPROM.");
  tools_menu->Append(ID_WRITE,  "Write\tAlt-W",      "Write data to the EEPROM.");
  tools_menu->Append(ID_VECTOR, "Set Vector\tAlt-V", "Set a 6502 jump vector.");

  actions_menu = new wxMenu();
  actions_menu->Append(wxID_ABOUT, "About\tF2",    "Shows information about eeprommer3.");
  actions_menu->Append(ID_HELP,    "Help\tF1",     "Shows a help screen.");
  actions_menu->Append(wxID_EXIT,  "Quit\tCtrl-Q", "Quit eeprommer3.");

  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");

  SetMenuBar(menu_bar);
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN,  MainFrame::OnMenuFileOpen)
EVT_MENU(wxID_SAVE,  MainFrame::OnMenuFileSave)
EVT_MENU(ID_SAVEAS,  MainFrame::OnMenuFileSaveAs)
EVT_MENU(ID_READ,    MainFrame::OnMenuToolsRead)
EVT_MENU(ID_WRITE,   MainFrame::OnMenuToolsWrite)
EVT_MENU(ID_VECTOR,  MainFrame::OnMenuToolsVector)
EVT_MENU(wxID_ABOUT, MainFrame::OnMenuActionsAbout)
EVT_MENU(ID_HELP,    MainFrame::OnMenuActionsHelp)
EVT_MENU(wxID_EXIT,  MainFrame::OnMenuActionsQuit)
END_EVENT_TABLE()

void MainFrame::OnMenuFileOpen(wxCommandEvent &event) {

}

void MainFrame::OnMenuFileSave(wxCommandEvent &event) {

}

void MainFrame::OnMenuFileSaveAs(wxCommandEvent &event) {
  
}

void MainFrame::OnMenuToolsRead(wxCommandEvent &event) {

}

void MainFrame::OnMenuToolsWrite(wxCommandEvent &event) {

}

void MainFrame::OnMenuToolsVector(wxCommandEvent &event) {

}

void MainFrame::OnMenuActionsAbout(wxCommandEvent &event) {
  wxLogMessage("About");
}

void MainFrame::OnMenuActionsHelp(wxCommandEvent &event) {
  wxLogMessage("Help");
}

void MainFrame::OnMenuActionsQuit(wxCommandEvent &event) {
  Close(true);
}
