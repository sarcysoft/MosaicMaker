#include "CompositeDlg.h"
#include "GlobalConfig.h"

using namespace cv;

CompositeDlg::CompositeDlg(wxArrayString fileList)
    : wxDialog(NULL, wxID_ANY, "Composite Maker", wxDefaultPosition, wxSize(1024, 600))
{
    SetFileList(fileList);

    wxPanel* panel = new wxPanel(this, -1);

    wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnViewSource = new wxButton(panel, ID_ViewSource, wxT("View Image"));
    hbox1->Add(btnViewSource);

    vbox1->Add(hbox1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mInputPanel = new wxPanel(panel);
    hbox2->Add(mInputPanel, 1, wxEXPAND);

    mOutputPanel = new wxPanel(panel);
    hbox2->Add(mOutputPanel, 1, wxEXPAND);

    vbox1->Add(hbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    panel->SetSizer(vbox1);

    Bind(wxEVT_BUTTON, &CompositeDlg::OnViewSource, this, ID_ViewSource);

    if (mFileList.Count() > 0)
    {
        Mat image;
        image = imread(mFileList[0].ToStdString(), IMREAD_COLOR);
        wxClientDC dc(mInputPanel);
        DrawMat(image, dc);
    }
}

void CompositeDlg::SetFileList(wxArrayString fileList)
{
    mFileList = fileList;
}

void CompositeDlg::OnViewSource(wxCommandEvent& event)
{
    if (mFileList.Count() > 0)
    {
        Mat image;
        image = imread(mFileList[0].ToStdString(), IMREAD_COLOR);

        namedWindow("Source Image", WINDOW_AUTOSIZE);
        imshow("Source Image", image);
    }
}


void CompositeDlg::DrawMat(Mat image, wxDC& dc)
{
    int dcW, dcH;
    dc.GetSize(&dcW, &dcH);

    double scaleW = (double)image.cols / (double)dcW;
    double scaleH = (double)image.rows / (double)dcH;

    double scale = (scaleW > scaleH) ? scaleW : scaleH;
    int w = (int)(image.cols / scale);
    int h = (int)(image.rows / scale);

    Mat scaled;
    resize(image, scaled, Size(w, h), 0, 0, INTER_AREA);

    wxImage bitmap(scaled.cols, scaled.rows, scaled.data, true);

    dc.DrawBitmap(bitmap, 0, 0, false);
}