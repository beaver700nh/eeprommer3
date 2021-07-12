#ifndef MAIN_HPP
#define MAIN_HPP

#include "menu.hpp"
#include "data.hpp"
#include "comm.hpp"
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

  // Menu event handlers

  // [File] menu
  void OnMenuFileOpen  (wxCommandEvent &event);
  void OnMenuFileSave  (wxCommandEvent &event);

  // [Tools] menu
  void OnMenuToolsRead  (wxCommandEvent &event);
  void OnMenuToolsWrite (wxCommandEvent &event);
  void OnMenuToolsVector(wxCommandEvent &event);
  void OnMenuToolsPort  (wxCommandEvent &event);
  void OMTP_show_dialog(wxArrayString ports, wxString old_port);

  // [Actions] menu
  void OnMenuActionsHelp (wxCommandEvent &event);
  void OnMenuActionsAbout(wxCommandEvent &event);
  void OnMenuActionsClear(wxCommandEvent &event);
  void OnMenuActionsQuit (wxCommandEvent &event);

  // [Debug] menu
  void OnMenuDebugTestRead (wxCommandEvent &event);
  void OnMenuDebugTestWrite(wxCommandEvent &event);

  void crash(wxString message, wxString title, int type);

  DECLARE_EVENT_TABLE()

private:
  MenuBar  menu_bar;
  HexData  hex_data;
  PortCtrl port_ctrl;

  wxFont normal_font, header_font;
};

DECLARE_APP(MainApp)

#endif
