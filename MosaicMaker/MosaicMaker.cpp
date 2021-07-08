#include "MosaicMaker.h"

#include <list>
#include "CompositeDlg.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Hello World", wxDefaultPosition, wxSize(1024,600))
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    //---------------------------------------------------------
    wxInitAllImageHandlers();

    wxPanel* panel = new wxPanel(this, -1);

    wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* vbox2 = new wxBoxSizer(wxVERTICAL);

    wxButton* btnBrowse = new wxButton(panel, ID_Browse, wxT("Browse"));
    hbox1->Add(btnBrowse);

    wxButton* btnBuild = new wxButton(panel, ID_Build, wxT("Build"));
    hbox1->Add(btnBuild);

    vbox1->Add(hbox1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mTreeBrowse = new wxTreeCtrl(panel, ID_BrowseTree);
    hbox2->Add(mTreeBrowse, 1, wxEXPAND);

    mImagePanel = new wxImagePanel(panel);
    vbox2->Add(mImagePanel, 1, wxEXPAND);

    mListBox = new wxListBox(panel, ID_PhotoList);
    vbox2->Add(mListBox, 1, wxEXPAND);

    hbox2->Add(vbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    vbox1->Add(hbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    panel->SetSizer(vbox1);

    Centre();

    Bind(wxEVT_BUTTON, &MyFrame::OnBrowse, this, ID_Browse);
    Bind(wxEVT_BUTTON, &MyFrame::OnBuild, this, ID_Build);

    Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::OnTreeClick, this, ID_BrowseTree);
    Bind(wxEVT_TREE_ITEM_ACTIVATED, &MyFrame::OnTreeDoubleClick, this, ID_BrowseTree);
    Bind(wxEVT_LISTBOX, &MyFrame::OnListClick, this, ID_PhotoList);
    Bind(wxEVT_LISTBOX_DCLICK, &MyFrame::OnListDoubleClick, this, ID_PhotoList);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnBrowse(wxCommandEvent& event)
{
    wxDirDialog dlg(NULL, "Choose input directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dlg.ShowModal() != wxID_CANCEL)
    {
        mTreeBrowse->DeleteAllItems();
        mFileMap.clear();

        wxString root = dlg.GetPath();
        root.Replace("%20", " ");

        RecurseFolders(root, NULL);

        // Expand all the nodes
        mTreeBrowse->ExpandAll();

    }
}

void MyFrame::OnBuild(wxCommandEvent& event)
{
    wxArrayString items = mListBox->GetStrings();
    if (items.Count() > 0)
    {
        CompositeDlg dlg;
        dlg.ShowModal();
    }
}

void MyFrame::OnTreeClick(wxCommandEvent& event)
{
    wxTreeItemId nodeId = mTreeBrowse->GetSelection();

    if (nodeId != NULL)
    {
        if (mFileMap.count(nodeId) > 0)
        {
            wxString file = mFileMap[nodeId];
            mImagePanel->SetNewImage(file);
        }
    }
}

void MyFrame::OnTreeDoubleClick(wxCommandEvent& event)
{
    wxTreeItemId nodeId = mTreeBrowse->GetSelection();

    if (nodeId != NULL)
    {
        if (mFileMap.count(nodeId) > 0)
        {
            wxString file = mFileMap[nodeId];
            mListBox->Append(file);
        }
    }
}

void MyFrame::OnListClick(wxCommandEvent& event)
{
    int nodeId = mListBox->GetSelection();

    if (nodeId != wxNOT_FOUND)
    {

        wxString file = mListBox->GetString(nodeId);
        mImagePanel->SetNewImage(file);
    }
}

void MyFrame::OnListDoubleClick(wxCommandEvent& event)
{
    int nodeId = mListBox->GetSelection();

    if (nodeId != wxNOT_FOUND)
    {
        mListBox->Delete(nodeId);
    }
}

void MyFrame::RecurseFolders(wxString path, wxTreeItemId rootId)
{
    wxTreeItemId nodeId;

    if (rootId == NULL)
    {
        nodeId = mTreeBrowse->AddRoot(path);
    }
    else
    {
        path.Replace("%20", " ");
        wxFileName pathName(path);
        nodeId = mTreeBrowse->AppendItem(rootId, pathName.GetFullName());
    }


    wxFileSystem* fileSys = new wxFileSystem();
    fileSys->ChangePathTo(path, true);

    std::list<wxString> folderList;

    wxString folder = fileSys->FindFirst("*", wxDIR);
    while (folder != "")
    {
        folderList.push_back(folder);
        folder = fileSys->FindNext();
    }

    std::list<wxString>::iterator it;
    for (it = folderList.begin(); it != folderList.end(); ++it) {
        RecurseFolders(*it, nodeId);
    }

    fileSys->ChangePathTo(path, true);

    wxString file = fileSys->FindFirst("*.jp*", wxFILE);
    while (file != "")
    {
        file.Replace("file:///", "");
        file.Replace("%20", " ");
        file.Replace("/", "\\");
        wxFileName pathName(file);
        mFileMap[mTreeBrowse->AppendItem(nodeId, pathName.GetFullName())] = file;

        file = fileSys->FindNext();
    }
}