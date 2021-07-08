#pragma once
// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>

#include "wx\treectrl.h"
#include "wx\filesys.h"

#include "wxImagePanel.h"

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();
private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnBrowse(wxCommandEvent& event);
    void OnBuild(wxCommandEvent& event);

    void OnTreeClick(wxCommandEvent& event);
    void OnTreeDoubleClick(wxCommandEvent& event);

    void RecurseFolders(wxString path, wxTreeItemId rootId);

    void OnListClick(wxCommandEvent& event);
    void OnListDoubleClick(wxCommandEvent& event);

    std::map<wxTreeItemId, wxString> mFileMap;
    wxTreeCtrl* mTreeBrowse;

    wxImagePanel* mImagePanel;
    wxListBox* mListBox;
};


