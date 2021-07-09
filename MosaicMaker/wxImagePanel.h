#pragma once
#include <wx/wx.h>
#include <wx/sizer.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

class wxImagePanel : public wxPanel
{
    wxImage mImage;
    wxBitmap resized;
    int w, h;

public:
    wxImagePanel(wxPanel* parent, wxString file = "", wxBitmapType format = wxBITMAP_TYPE_JPEG);

    void SetNewImage(wxString file = "", wxBitmapType format = wxBITMAP_TYPE_JPEG);
    void SetNewImage(Mat image);

    void paintEvent(wxPaintEvent& evt);
    void paintNow();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);

    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseDown(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyPressed(wxKeyEvent& event);
     void keyReleased(wxKeyEvent& event);
     */

    DECLARE_EVENT_TABLE()
};