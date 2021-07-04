#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>

#include "backend.hpp"
#include "file.hpp"
#include "info.hpp"
#include "main.hpp"
#include "wx_dep.hpp"

void MainFrame::OnMenuFileOpen(wxCommandEvent &WXUNUSED(event)) {
  FileIO_Status res = FileIO::open_file("file.bin", hex_data, this);
  wxString fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully opened file \"%s.\"", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to open file \"%s\". (Does it exist?)", fname));
  }
}

void MainFrame::OnMenuFileSave(wxCommandEvent &WXUNUSED(event)) {
  FileIO_Status res = FileIO::save_file("file.bin", hex_data, this);
  wxString fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully saved to file \"%s.\"", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to save to file \"%s\". (Out of space?)", fname));
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
}

void MainFrame::OnMenuToolsPort(wxCommandEvent &WXUNUSED(event)) {
  char **ports = (char **) malloc(sizeof(char *) * 256);

  if (ports == nullptr) {
    error("Could not allocate memory.", "Memory Error", wxOK);
    return;
  }

  for (uint8_t i = 0; true; ++i) {
    ports[i] = (char *) malloc(sizeof(char) * 17);

    if (ports[i] == nullptr) {
      error("Could not allocate memory.", "Memory Error", wxOK);

      for (uint8_t j = 0; j < i; ++j) {
        free(ports[j]);
      }

      free(ports);

      return;
    }

    if (i == 255) break;
  }

  /////////////////////////////////////////////////

  if (!get_ports(ports)) {
    printf("Error getting ports.\n");
    return;
  }

  for (uint8_t i = 0; ports[i] != nullptr; ++i) {
    printf("Port #%d:\t\t%s\n", i, ports[i]);
  }

  /////////////////////////////////////////////////

  for (uint8_t i = 0; true; ++i) {
    free(ports[i]);
    if (i == 255) break;
  }

  free(ports);
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
