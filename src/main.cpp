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
  actions_menu->Append(wxID_EXIT,  "Quit\tCtrl-Q", "Quit eeprommer3.");

  menu_bar->Append(file_menu,    "File");
  menu_bar->Append(tools_menu,   "Tools");
  menu_bar->Append(actions_menu, "Actions");

  SetMenuBar(menu_bar);
}

void MainFrame::add_contents() {
  const wxSize cell_size = wxSize(32, 22);

  wxPanel *panel = new wxPanel(this);

  wxStaticText *hi = new wxStaticText(
    panel, wxNewId(), "H", wxPoint(13, 5), cell_size
  );

  wxStaticText *lo = new wxStaticText(
    panel, wxNewId(), "L", wxPoint(25, 2), cell_size
  );

  hi->SetFont(normal_font);
  lo->SetFont(normal_font);

  wxStaticText *row_hdrs[16];
  wxStaticText *col_hdrs[16];

  wxStaticText *data[16][16];

  for (int i = 0; i < 16; ++i) {
    row_hdrs[i] = new wxStaticText(
      panel, wxNewId(), wxString::Format("%x0", i),
      wxPoint(10, (i + 1) * cell_size.GetHeight()), cell_size
    );

    col_hdrs[i] = new wxStaticText(
      panel, wxNewId(), wxString::Format("0%x", i),
      wxPoint((i + 1) * cell_size.GetWidth() + 10, 0), cell_size
    );

    row_hdrs[i]->SetFont(header_font);
    col_hdrs[i]->SetFont(header_font);
  }

  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      data[i][j] = new wxStaticText(
        panel, data_ids + i*16 + j, wxString::Format("%x%x", i, j),
        wxPoint(
          (j + 1) * cell_size.GetWidth() + 12,
          (i + 1) * cell_size.GetHeight() + 2
        ), cell_size
      );

      data[i][j]->SetFont(normal_font);
    }
  }
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN,  MainFrame::OnMenuFileOpen)
EVT_MENU(wxID_SAVE,  MainFrame::OnMenuFileSave)
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

void MainFrame::OnMenuToolsRead(wxCommandEvent &event) {

}

void MainFrame::OnMenuToolsWrite(wxCommandEvent &event) {

}

void MainFrame::OnMenuToolsVector(wxCommandEvent &event) {

}

void MainFrame::OnMenuActionsAbout(wxCommandEvent &event) {
  wxAboutDialogInfo info;

  info.SetIcon(png_logo_wxicon);
  info.SetName("eeprommer3");
  info.SetVersion("0.0.3-dev");
  info.SetDescription("This is an AT28C256 EEPROM programmer. (Frontend)");

  info.AddDeveloper("Anon Ymus");

  wxAboutBox(info);
}

void MainFrame::OnMenuActionsHelp(wxCommandEvent &event) {
  wxMessageDialog dlg(this, "Help", "Help");

  dlg.SetMessage(
    "eeprommer3 Help Dialog"
  );

  dlg.SetExtendedMessage(
    "EEPROM read: Tools > Read.\n"
    "EEPROM write: Tools > Write.\n"
    "Set vector**: Tools > Vector.\n"
    "\n"
    "Save read data to file: File > Save.\n"
    "Upload file to EEPROM: File > Open.\n"
    "\n"
    "**Vectors are 6502 jump vectors, like\n"
    "IRQ, NMI, and RES. ($FFFA-$FFFF)\n"
  );

  dlg.SetIcon(png_logo_wxicon);

  dlg.ShowModal();
}

void MainFrame::OnMenuActionsQuit(wxCommandEvent &event) {
  Close(true);
}
