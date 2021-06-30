#ifndef MAIN_HPP
#define MAIN_HPP

#include "data.hpp"
#include "menu.hpp"
#include "wx_dep.hpp"

#undef __GXX_ABI_VERSION
#define __GXX_ABI_VERSION 1013

#define DATA_IDS 200

class MainApp : public wxApp {
public:
  virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

  void OnMenuFileOpen  (wxCommandEvent &event);
  void OnMenuFileSave  (wxCommandEvent &event);

  void OnMenuToolsRead  (wxCommandEvent &event);
  void OnMenuToolsWrite (wxCommandEvent &event);
  void OnMenuToolsVector(wxCommandEvent &event);

  void OnMenuActionsAbout(wxCommandEvent &event);
  void OnMenuActionsHelp (wxCommandEvent &event);
  void OnMenuActionsClear(wxCommandEvent &event);
  void OnMenuActionsQuit (wxCommandEvent &event);

  DECLARE_EVENT_TABLE()

private:
  MenuBar menu_bar;
  HexData hex_data;

  wxIcon png_logo_wxicon = wxIcon("../eeprommer3.png", wxBITMAP_TYPE_PNG_RESOURCE, 64, 64);

  wxFont normal_font, header_font;
};

DECLARE_APP(MainApp)

#endif
