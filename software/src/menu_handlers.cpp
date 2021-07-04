#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>

#include "backend.hpp"
#include "file.hpp"
#include "main.hpp"
#include "wx_dep.hpp"

void MainFrame::OnMenuFileOpen(wxCommandEvent &WXUNUSED(event)) {
  FileIO_Status res = FileIO::open_file("file.bin", hex_data, this);
  wxString fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully opened file %s.", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to open file %s. (Does it exist?)", fname));
  }
}

void MainFrame::OnMenuFileSave(wxCommandEvent &WXUNUSED(event)) {
  FileIO_Status res = FileIO::save_file("file.bin", hex_data, this);
  wxString fname = std::get<1>(res);

  if (std::get<0>(res)) {
    SetStatusText(wxString::Format("Successfully saved to file %s.", fname));
  }
  else {
    SetStatusText(wxString::Format("Failed to save to file %s. (Out of space?)", fname));
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

  for (uint8_t i = 0; true; ++i) {
    ports[i] = (char *) malloc(sizeof(char) * 17);
    if (i == 255) break;
  }

  if (!get_ports(ports)) {
    printf("Error getting ports.\n");
    return;
  }

  for (uint8_t i = 0; ports[i] != nullptr; ++i) {
    printf("Port #%d:\t\t%s\n", i, ports[i]);
  }

  for (uint8_t i = 0; true; ++i) {
    free(ports[i]);
    if (i == 255) break;
  }

  free(ports);
}

void MainFrame::OnMenuActionsAbout(wxCommandEvent &WXUNUSED(event)) {
  wxAboutDialogInfo info;

  info.SetIcon(png_logo_wxicon);
  info.SetName("eeprommer3");
  info.SetVersion(EEPROMMER3_VERSION);
  info.SetDescription(
    "This is an AT28Cxxx EEPROM programmer frontend.\n"
    "It interfaces with a serial port which is connected\n"
    "to some hardware in order to program an EEPROM.\n"
    "\n"
    "More information for both the frontend (this program)\n"
    "and the backend (the hardware) can be found on GitHub\n"
    "at https://github.com/beaver700nh/eeprommer3/."
  );

  info.AddDeveloper("Anon Ymus");
  info.AddDeveloper("Thanks to StackOverflow");
  info.AddDeveloper("Thanks to wxwidgets.org");

  wxAboutBox(info);
}

void MainFrame::OnMenuActionsHelp(wxCommandEvent &WXUNUSED(event)) {
  wxMessageDialog dlg(this, "Help", "Help");

  dlg.SetMessage(
    "eeprommer3 Help Dialog"
  );

  dlg.SetExtendedMessage(
    "EEPROM manipulation is under [Tools].\n"
    "Use the [File] menu to open and save files.\n"
    "Miscellaneous things are under [Actions].\n"
    "\n"
    "Vectors are 6502 jump vectors, like\n"
    "IRQ, NMI, and RES. ($FFFA-$FFFF)\n"
    "You can ignore them if not needed.\n"
  );

  dlg.SetIcon(png_logo_wxicon);

  dlg.ShowModal();
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
