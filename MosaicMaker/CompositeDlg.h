#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx\bitmap.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

class CompositeDlg :
    public wxDialog
{
public:
    CompositeDlg(wxArrayString fileList);
    void SetFileList(wxArrayString fileList);

    void OnViewSource(wxCommandEvent& event);

    void DrawMat(Mat image, wxDC& dc);

private:
    wxArrayString mFileList;
    wxPanel* mInputPanel;
    wxPanel* mOutputPanel;

};

