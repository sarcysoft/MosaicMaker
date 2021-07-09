#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <queue>
#include <mutex>
#include <map>
#include <list>

#include "wxImagePanel.h"

#include "wx\bitmap.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

class RgbValue
{
public:
    RgbValue() {
        r = g = b = 0;
    }

    RgbValue(uchar* pData) {
        b = pData[0];
        g = pData[1];
        r = pData[2];
    }

    uchar r;
    uchar g;
    uchar b;
};

class BuildTile
{
public:
    std::string filename;
    std::list<std::tuple<int, int>> coords;
};

class CompositeDlg :
    public wxDialog
{
public:
    CompositeDlg(wxArrayString fileList);
    void SetFileList(wxArrayString fileList);
    void SetImportList(wxArrayString fileList);

    void OnViewSource(wxCommandEvent& event);
    void OnMapOutput(wxCommandEvent& event);
    void OnBuildOutput(wxCommandEvent& event);

private:
    std::string FindClosest(RgbValue value);

    static void ImportThreadHandler(CompositeDlg* pInst);
    void ImportFilesThread();

    void MapOutput(std::string filename);
    static void MapThreadHandler(CompositeDlg* pInst);
    void MapThread();

    static void BuildOutputThreadHandler(CompositeDlg* pInst);
    void BuildOutputThread();

    Mat LoadMat(std::string path, int zoom = 1);

    std::map<std::string, RgbValue> mRgbMap;

    std::mutex mImportMutex;
    std::queue<std::string> mImportQueue;

    std::mutex mBuildMutex;
    std::queue<std::tuple<int, int, RgbValue>> mBuildInputQueue;
    std::map<std::string, std::list<std::tuple<int, int>>> mBuildTileMap;
    std::queue<BuildTile> mBuildTileQueue;

    wxArrayString mFileList;
    wxArrayString mImportList;
    wxImagePanel* mInputPanel;
    wxImagePanel* mOutputPanel;
    wxGauge* mProgressBar;

    Mat mOutputMat;
};

