#include "wxImagePanel.h"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/mstream.h>

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)
// some useful events
/*
 EVT_MOTION(wxImagePanel::mouseMoved)
 EVT_LEFT_DOWN(wxImagePanel::mouseDown)
 EVT_LEFT_UP(wxImagePanel::mouseReleased)
 EVT_RIGHT_DOWN(wxImagePanel::rightClick)
 EVT_LEAVE_WINDOW(wxImagePanel::mouseLeftWindow)
 EVT_KEY_DOWN(wxImagePanel::keyPressed)
 EVT_KEY_UP(wxImagePanel::keyReleased)
 EVT_MOUSEWHEEL(wxImagePanel::mouseWheelMoved)
 */

 // catch paint events
EVT_PAINT(wxImagePanel::paintEvent)
//Size event
EVT_SIZE(wxImagePanel::OnSize)
END_EVENT_TABLE()


// some useful events
/*
    void wxImagePanel::mouseMoved(wxMouseEvent& event) {}
    void wxImagePanel::mouseDown(wxMouseEvent& event) {}
    void wxImagePanel::mouseWheelMoved(wxMouseEvent& event) {}
    void wxImagePanel::mouseReleased(wxMouseEvent& event) {}
    void wxImagePanel::rightClick(wxMouseEvent& event) {}
    void wxImagePanel::mouseLeftWindow(wxMouseEvent& event) {}
    void wxImagePanel::keyPressed(wxKeyEvent& event) {}
    void wxImagePanel::keyReleased(wxKeyEvent& event) {}
    */

wxImagePanel::wxImagePanel(wxPanel* parent, wxString file, wxBitmapType format) : wxPanel(parent)
{
    SetNewImage(file, format);
}

void wxImagePanel::SetNewImage(wxString file, wxBitmapType format)
{
    if (file != "")
    {
        // load the file... ideally add a check to see if loading was successful
        mImage.LoadFile(file, format);
        w = -1;
        h = -1;

        paintNow();
    }
}

void wxImagePanel::SetNewImage(Mat image)
{
    std::vector<uchar> buf;
    imencode(".bmp", image, buf);
    wxMemoryInputStream stream((void*)buf.data(), buf.size());
    mImage = wxImage(stream, wxBITMAP_TYPE_ANY);
    w = -1;
    h = -1;

    paintNow();
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void wxImagePanel::paintEvent(wxPaintEvent& evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void wxImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void wxImagePanel::render(wxDC& dc)
{
    if (mImage.IsOk())
    {
        int tempw, temph;
        dc.GetSize(&tempw, &temph);

        double wscale = (double)mImage.GetWidth() / (double)tempw;
        double hscale = (double)mImage.GetHeight() / (double)temph;

        double tempScale = (wscale > hscale) ? wscale : hscale;

        int neww = (int)((double)mImage.GetWidth() / tempScale);
        int newh = (int)((double)mImage.GetHeight() / tempScale);

        if (neww != w || newh != h)
        {
            resized = wxBitmap(mImage.Scale(neww, newh /*, wxIMAGE_QUALITY_HIGH*/));
            w = neww;
            h = newh;
            dc.DrawBitmap(resized, 0, 0, false);
        }
        else {
            dc.DrawBitmap(resized, 0, 0, false);
        }
    }
}

/*
 * Here we call refresh to tell the panel to draw itself again.
 * So when the user resizes the image panel the image should be resized too.
 */
void wxImagePanel::OnSize(wxSizeEvent& event) {
    Refresh();
    //skip the event.
    event.Skip();
}