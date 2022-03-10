#include "info.hpp"
#include "wx_dep.hpp"

void AppInfo::help_dlg(wxFrame *parent) {
  wxMessageDialog dlg(parent, "Help", "Help");

  dlg.SetMessage(
    "eeprommer3 Help Dialog"
  );

  dlg.SetExtendedMessage(
    "EEPROM manipulation is under [Tools].\n"
    "Use the [File] menu to open and save files.\n"
    "Miscellaneous things are under [Actions].\n"
    "\n"
    "Vectors are 6502 jump vectors, like\n"
    "IRQ, NMI, and RES. ($FFFA-$FFFF).\n"
    "\n"
    "These are at the EEPROM addresses\n"
    "$7FFA-$7FFF but appear at $FFFA-$FFFF\n"
    "due to the hardware requiring them\n"
    "to be memory-mapped to there.\n"
    "You can ignore them if not needed.\n"
  );

  dlg.SetIcon(png_logo_wxicon);

  dlg.ShowModal();
}

void AppInfo::about_dlg() {
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
  info.AddDeveloper("Thanks to VSCode");

  wxAboutBox(info);
}
