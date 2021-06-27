#ifndef MAIN_HPP
#define MAIN_HPP

#ifdef WX_PRECOMP
#  include <wx/wxprec.h>
#else
#  include <wx/wx.h>
#endif

#undef __GXX_ABI_VERSION
#define __GXX_ABI_VERSION 1013

class MainApp : public wxApp {
public:
  virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

  void OnMenuFileOpen  (wxCommandEvent &event);
  void OnMenuFileSave  (wxCommandEvent &event);
  void OnMenuFileSaveAs(wxCommandEvent &event);

  void OnMenuToolsRead  (wxCommandEvent &event);
  void OnMenuToolsWrite (wxCommandEvent &event);
  void OnMenuToolsVector(wxCommandEvent &event);

  void OnMenuActionsAbout(wxCommandEvent &event);
  void OnMenuActionsHelp (wxCommandEvent &event);
  void OnMenuActionsQuit (wxCommandEvent &event);

  DECLARE_EVENT_TABLE()

private:
  wxMenuBar *menu_bar;
  wxMenu *file_menu, *tools_menu, *actions_menu;
};

int \
  ID_SAVEAS = wxNewId(),
  ID_READ   = wxNewId(),
  ID_WRITE  = wxNewId(),
  ID_VECTOR = wxNewId(),
  ID_HELP   = wxNewId();

DECLARE_APP(MainApp)

#endif
