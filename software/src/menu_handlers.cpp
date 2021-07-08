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

  port_ctrl.test_write(R"(abc<<i am data>>def[[im a cmd]]ghi<<<<\\\>\>>>jkl[[[[\\\]\]]])");
}

void MainFrame::OnMenuToolsPort(wxCommandEvent &WXUNUSED(event)) {
  char **ports = (char **) malloc(sizeof(char *) * 256); // 1

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
  } // 2
  // Extract into util T init2d<T>(x, y, std:function<void(wxString, wxString, int)>)

  int16_t num_ports = port_ctrl.list_ports(ports);

  if (num_ports < 0) {
    error("Error getting list of ports.", "Port Error", wxOK);
    return;
  }

  auto *chooser_box = new wxSingleChoiceDialog( // 1
    (wxFrame *) nullptr, "Choose a port.", "Port",
    wxArrayString(num_ports, (const char **) ports)
  );

  if (chooser_box->ShowModal() == wxID_OK) {
    const char *old_port = port_ctrl.get_cur_port();
    const char *old_label = (
      port_ctrl.is_initialized() ?
      wxString::Format("Port (%s)", old_port).ToUTF8().data() :
      "Port"
    );

    char new_port[20];
    strcpy(new_port, chooser_box->GetStringSelection().ToUTF8().data());

    int8_t res = port_ctrl.set_cur_port(new_port);

    if (res == 0) {
      SetStatusText(wxString::Format("Successfully set port to %s.", new_port));

      menu_bar.get_menu_bar()->SetLabel(
        menu_bar.get_menu_bar()->FindMenuItem("Tools", old_label),
        wxString::Format("Port (%s)", new_port)
      );
    }
    else if (res == 1) {
      error(wxString::Format("Error closing previous port (%s).", old_port), "Port Error", wxOK);
    }
    else if (res == 2) {
      error(wxString::Format("Error opening new port (%s).", new_port), "Port Error", wxOK);
    }
  } // 2
  // Extract into void MainFrame::OMTP_show_dialog(wxString old_port, wxString new_port)

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
