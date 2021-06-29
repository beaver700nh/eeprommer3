#ifndef MAIN_HPP
#define MAIN_HPP

#include <wx/wx.h>

#ifdef WX_PRECOMP
#  include <wx/wxprec.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>

#undef __GXX_ABI_VERSION
#define __GXX_ABI_VERSION 1013

#define DATA_IDS 200
#define MENU_IDS 300

enum {
  ID_READ = MENU_IDS,
  ID_WRITE,
  ID_VECTOR,
  ID_HELP,
  ID_CLEAR,
};

class MainApp : public wxApp {
public:
  virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

  void add_menu();
  void add_contents();

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
  wxMenuBar *menu_bar;
  wxMenu *file_menu, *tools_menu, *actions_menu;

  wxFont normal_font, header_font;

  wxIcon png_logo_wxicon = wxIcon("../eeprommer3.png", wxBITMAP_TYPE_PNG_RESOURCE, 64, 64);

  wxPanel *panel;
  wxStaticText *hi, *lo;
  wxStaticText *row_hdrs[16], *col_hdrs[16];
  wxStaticText *data[16][16];

  void open_file(wxString fname);
  void save_file(wxString fname);
  void clear_hex();
};

DECLARE_APP(MainApp)

#endif
