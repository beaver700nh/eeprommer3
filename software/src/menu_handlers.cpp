#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <tuple>

#include "comm.hpp"
#include "dlgbox.hpp"
#include "file.hpp"
#include "info.hpp"
#include "main.hpp"
#include "util.hpp"
#include "wx_dep.hpp"

void MainFrame::OnMenuFileOpen(wxCommandEvent &WXUNUSED(event)) {
  auto res = FileIO::open_file("file.bin", hex_data);
  auto fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully opened file '%s.'", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to open file '%s.' (Does it exist?)", fname));
  }
}

void MainFrame::OnMenuFileSave(wxCommandEvent &WXUNUSED(event)) {
  auto res = FileIO::save_file("file.bin", hex_data);
  auto fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully saved to file '%s.'", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to save to file '%s.' (Out of space?)", fname));
  }
}

void MainFrame::OnMenuToolsRead(wxCommandEvent &WXUNUSED(event)) {
  printf("You pressed: [Tools] > [Read]\n");
}

void MainFrame::OnMenuToolsWrite(wxCommandEvent &WXUNUSED(event)) {
  printf("You pressed: [Tools] > [Write]\n");
}

void MainFrame::OnMenuToolsVector(wxCommandEvent &WXUNUSED(event)) {
  printf("You pressed: [Tools] > [Vector]\n");
}

void MainFrame::OnMenuToolsPort(wxCommandEvent &WXUNUSED(event)) {
  auto **ports = init2d<char>(256, 17, DlgBox::error);

  int16_t num_ports = port_ctrl.list_ports(ports);

  if (num_ports < 0) {
    DlgBox::error("Error getting list of ports.", "Port Error", wxOK);
    return;
  }

  OMTP_show_dialog(wxArrayString(num_ports, (const char **) ports), port_ctrl.get_cur_port());

  free2d((void **) ports, 256);
}

void MainFrame::OMTP_show_dialog(wxArrayString ports, wxString old_port) {
  // Initialize a dialog
  auto *chooser_box = new wxSingleChoiceDialog(
    (wxFrame *) nullptr, "Choose a port.", "Port", ports
  );

  // Show the dialog
  if (chooser_box->ShowModal() == wxID_OK) {
    // User pressed [OK]
    wxString port_hint = (port_ctrl.is_initialized() ? " (" + old_port + ")" : "");
    wxString old_label = "Port" + port_hint + "\tAlt-P";

    wxString new_port = chooser_box->GetStringSelection();

    int8_t res = port_ctrl.set_cur_port(new_port);

    if (res == 0) {
      // Success
      SetStatusText("Successfully set port to " + new_port + ".");

      menu_bar.get_menu_bar()->SetLabel(
        menu_bar.get_menu_bar()->FindMenuItem("Tools", old_label),
        "Port (" + new_port + ")\tAlt-P"
      );
    }
    else if (res == 1) {
      // Closing error
      DlgBox::error("Error closing previous port (" + old_port + ").", "Port Error", wxOK);
    }
    else if (res == 2) {
      // Opening error
      DlgBox::error("Error opening new port (" + new_port + ").", "Port Error", wxOK);
    }
  }
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
      // Set all cells to "??"
      return wxString("??");
    }
  );

  SetStatusText("Hex display has been cleared.");
}

void MainFrame::OnMenuActionsQuit(wxCommandEvent &WXUNUSED(event)) {
  SetStatusText("Goodbye, world!");

  Close(true);
}

void MainFrame::OnMenuDebugTestRead(wxCommandEvent &WXUNUSED(event)) {
  printf("Tools > Read\n");

  char buf1[20], buf2[20], buf3[20];

  port_ctrl.test_write(R"([[parrotFOOBAR#!abcdef]])");
  port_ctrl.test_read(3, buf1);
  port_ctrl.test_read("#!", buf2);
  port_ctrl.test_read(6, buf3); // intentionally overflows buffer

  DlgBox::info(
    wxString::Format(
      "buf1:\t" "%s\n"
      "buf2:\t" "%s\n"
      "buf3:\t" "%s",
      buf1, buf2, buf3
    ),
    "Test result!", wxOK
  );
}

void MainFrame::OnMenuDebugTestWrite(wxCommandEvent &WXUNUSED(event)) {
  printf("Tools > Write\n");

  port_ctrl.test_write(R"(abc<<i am data>>def[[im a cmd]]ghi<<<<\\\>\>>>jkl[[[[\\\]\]]])");
}
