#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>

#include "comm.hpp"
#include "file.hpp"
#include "info.hpp"
#include "main.hpp"
#include "util.hpp"
#include "wx_dep.hpp"

void MainFrame::OnMenuFileOpen(wxCommandEvent &WXUNUSED(event)) {
  auto res = FileIO::open_file("file.bin", hex_data, this);
  auto fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully opened file '%s.'", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to open file '%s.' (Does it exist?)", fname));
  }
}

void MainFrame::OnMenuFileSave(wxCommandEvent &WXUNUSED(event)) {
  auto res = FileIO::save_file("file.bin", hex_data, this);
  auto fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully saved to file '%s.'", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to save to file '%s.' (Out of space?)", fname));
  }
}

void MainFrame::OnMenuToolsRead(wxCommandEvent &WXUNUSED(event)) {
  printf("Tools > Read\n");
}

void MainFrame::OnMenuToolsWrite(wxCommandEvent &WXUNUSED(event)) {
  printf("Tools > Write\n");
}

void MainFrame::OnMenuToolsVector(wxCommandEvent &WXUNUSED(event)) {
  printf("Tools > Vector\n");

  auto *test = new wxFrame(
    (wxFrame *) nullptr, wxID_ANY, "Test window",
    wxDefaultPosition, wxSize(700, 700)
  );

  test->Show(true);
}

void MainFrame::OnMenuToolsPort(wxCommandEvent &WXUNUSED(event)) {
  char **ports = (char **) malloc(sizeof(char *) * 256);

  if (ports == nullptr) {
    error("Could not allocate memory.", "Memory Error", wxOK);
    return;
  }

  for (uint16_t i = 0; i < 256; ++i) {
    ports[i] = (char *) malloc(sizeof(char) * 17);

    if (ports[i] == nullptr) {
      error("Could not allocate memory.", "Memory Error", wxOK);
      free2d((void **) ports, i);
      return;
    }
  }

  if (!port_ctrl.list_ports(ports)) {
    error("Error getting list of ports.", "Port Error", wxOK);
    return;
  }

  // Show dialog box with chooser

  // If ok
  //   Init portctrl
  //   Write status
  //   Update menu

  int8_t res = port_ctrl.set_cur_port("/dev/ttyACM0");

  if (res == 0) {
    port_ctrl.test_write("<<data :)>>");
    port_ctrl.test_write("[[cmd!]]");
  }
  else if (res == 1) {
    error("Error closing previous port.", "Port Error", wxOK);
  }
  else if (res == 2) {
    error("Error opening new port.", "Port Error", wxOK);
  }

  free2d((void **) ports, 256);
}

void MainFrame::OnMenuActionsHelp(wxCommandEvent &WXUNUSED(event)) {
  AppInfo::help_dlg(this);
}

void MainFrame::OnMenuActionsAbout(wxCommandEvent &WXUNUSED(event)) {
  AppInfo::about_dlg();
}

void MainFrame::OnMenuActionsClear(wxCommandEvent &WXUNUSED(event)) {
  hex_data.set_data(
    []SETDATA_LAMBDA {
      return wxString("??");
    }
  );

  SetStatusText("Hex display has been cleared.");
}

void MainFrame::OnMenuActionsQuit(wxCommandEvent &WXUNUSED(event)) {
  SetStatusText("Goodbye, world!");

  Close(true);
}
