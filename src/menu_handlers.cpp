#include <cstdio>

#include "main.hpp"

void MainFrame::OnMenuFileOpen(wxCommandEvent &event) {
  wxString fname = wxLoadFileSelector(
    "Choose a file to open",
    "BIN file (*.bin)|*.bin|HEX file (*.hex)|*.hex",
    "file.bin", this
  );

  if (fname.empty()) {
    wxMessageBox("Couldn't open file.", "Error", wxOK, this);
    return;
  }

  hex_data.set_data(
    [](uint8_t i, uint8_t j) -> wxString {
      (void) i;
      (void) j;

      return wxString::Format("%02x", 0x00);
    }
  );
}

void MainFrame::OnMenuFileSave(wxCommandEvent &event) {
  wxString fname = wxSaveFileSelector(
    "Choose a file name to save as",
    "BIN file (*.bin)|*.bin|HEX file (*.hex)|*.hex",
    "file.bin", this
  );

  if (fname.empty()) {
    wxMessageBox("Couldn't save file.", "Error", wxOK, this);
    return;
  }

  hex_data.for_each(
    [](uint8_t i, uint8_t j, wxString v) -> void {
      printf("At row %d col %d: %s", i, j, v.ToUTF8().data());
    }
  );
}

void MainFrame::OnMenuToolsRead(wxCommandEvent &event) {
  printf("Tools > Read\n");
}

void MainFrame::OnMenuToolsWrite(wxCommandEvent &event) {
  printf("Tools > Write\n");
}

void MainFrame::OnMenuToolsVector(wxCommandEvent &event) {
  printf("Tools > Vector\n");
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

void MainFrame::OnMenuActionsClear(wxCommandEvent &event) {
  hex_data.set_data(
    [](uint8_t i, uint8_t j) -> wxString {
      return wxString("??");
    }
  );
}

void MainFrame::OnMenuActionsQuit(wxCommandEvent &event) {
  Close(true);
}
