#include "CompositeDlg.h"
#include "GlobalConfig.h"

#include <thread>
#include <vector>

#include "wx\thread.h"

using namespace cv;

CompositeDlg::CompositeDlg(wxArrayString fileList)
    : wxDialog(NULL, wxID_ANY, "Composite Maker", wxDefaultPosition, wxSize(1024, 600))
{
    wxPanel* panel = new wxPanel(this, -1);

    wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnViewSource = new wxButton(panel, ID_ViewSource, wxT("View Image"));
    hbox1->Add(btnViewSource);

    wxButton* btnMapOutput = new wxButton(panel, ID_MapOutput, wxT("Map Output"));
    hbox1->Add(btnMapOutput);

    wxButton* btnBuildOutput = new wxButton(panel, ID_BuildOutput, wxT("Build Output"));
    hbox1->Add(btnBuildOutput);

    vbox1->Add(hbox1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    wxColour col1, col2;
    col1.Set(wxT("#4f5049"));
    col2.Set(wxT("#ededed"));

    mInputPanel = new wxImagePanel(panel);
    mInputPanel->SetBackgroundColour(col1);
    hbox2->Add(mInputPanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mOutputPanel = new wxImagePanel(panel);
    mOutputPanel->SetBackgroundColour(col2);
    hbox2->Add(mOutputPanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    vbox1->Add(hbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mProgressBar = new wxGauge(panel, wxID_ANY, 100);
    vbox1->Add(mProgressBar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 0);

    panel->SetSizer(vbox1);

    Bind(wxEVT_BUTTON, &CompositeDlg::OnViewSource, this, ID_ViewSource);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnMapOutput, this, ID_MapOutput);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnBuildOutput, this, ID_BuildOutput);

    SetFileList(fileList);

    if (mFileList.Count() > 0)
    {
        Mat image;
        image = imread(mFileList[0].ToStdString(), IMREAD_COLOR);
        mInputPanel->SetNewImage(image);
    }
}

void CompositeDlg::SetFileList(wxArrayString fileList)
{
    mFileList = fileList;
}

void CompositeDlg::SetImportList(wxArrayString fileList)
{
    mImportList = fileList;
    mRgbMap.clear();

    for (int i = 0; i < mImportList.Count(); i++)
    {
        Mat image;
        mImportQueue.push(mImportList[i].ToStdString());
    }

    int threads = wxThread::GetCPUCount() - 1;
    threads = (threads > 0) ? threads : 1;
    
    for (int i = 0; i < threads; i++)
    {
        std::thread importThread(ImportThreadHandler, this);
        importThread.detach();
    }
}

void CompositeDlg::ImportThreadHandler(CompositeDlg* pInst)
{
    pInst->ImportFilesThread();
}

void CompositeDlg::ImportFilesThread()
{
    int count = mImportList.Count();

    mProgressBar->SetValue(0);

    mImportMutex.lock();

    while (!mImportQueue.empty())
    {
        std::string filename = "";
        filename = mImportQueue.front();
        mImportQueue.pop();
    
        mImportMutex.unlock();

        Mat image = LoadMat(filename);
        Vec3b bgrPixel = image.at<Vec3b>(0, 0);
        RgbValue rgb(bgrPixel.val);

        int progress = (100 * (count - mImportQueue.size())) / count;

        mProgressBar->SetValue(progress);
    
        mImportMutex.lock();
        mRgbMap[filename] = RgbValue(rgb);
    }

    mImportMutex.unlock();
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


Mat CompositeDlg::LoadMat(std::string path, int zoom)
{
    Mat tempMat = imread(path);

    int minsize = (tempMat.cols < tempMat.rows) ? tempMat.cols : tempMat.rows;

    Rect roi((tempMat.cols - minsize) >> 1, (tempMat.rows - minsize) >> 1, minsize, minsize);
    tempMat = Mat(tempMat, roi);

    resize(tempMat, tempMat, Size(zoom, zoom), 0, 0, INTER_AREA);

    return tempMat;
}

std::string CompositeDlg::FindClosest(RgbValue value)
{
    std::string best;
    int closest = -1;

    std::vector<std::string> bestSet;

    for (std::map<std::string, RgbValue>::const_iterator it = mRgbMap.begin(); it != mRgbMap.end(); ++it)
    {

        int rr = it->second.r - value.r;
        int gg = it->second.g - value.g;
        int bb = it->second.b - value.b;

        int dist = (rr * rr) + (gg * gg) + (bb * bb);

        if (dist < 100)
        {
            bestSet.push_back(it->first);
        }

        if ((closest < 0) || (dist < closest))
        {
            closest = dist;
            best = it->first;
        }
    }

    if (bestSet.size() > 0)
    {
        best = bestSet[rand() % bestSet.size()];
    }

    return best;
}


void CompositeDlg::OnMapOutput(wxCommandEvent& event)
{
    if (mFileList.Count() > 0)
    {
        MapOutput(mFileList[0].ToStdString());
    }
}


void CompositeDlg::MapOutput(std::string filename)
{
    Mat image;
    image = imread(filename, IMREAD_COLOR);

    double scaleX = (double)image.cols / 600.0;
    double scaleY = (double)image.rows / 400.0;
    double scale = (scaleX > scaleY) ? scaleX : scaleY;

    int sizeX = (int)((double)image.cols / scale);
    int sizeY = (int)((double)image.rows / scale);

    resize(image, image, Size(sizeX, sizeY), 0, 0, INTER_AREA);

    mOutputMat = Mat1f(image.rows, image.cols);

    for (int y = 0; y < image.rows; y++)
    {
        for (int x = 0; x < image.cols; x++)
        {
            Vec3b bgrPixel = image.at<Vec3b>(y, x);

            RgbValue rgb(bgrPixel.val);

            mBuildInputQueue.push(std::make_tuple(x, y, rgb));
        }
    }

    mProgressBar->SetValue(0);

    mBuildTileMap.clear();

    int threads = wxThread::GetCPUCount() - 1;
    threads = (threads > 0) ? threads : 1;

    for (int i = 0; i < threads; i++)
    {
        std::thread mapThread(MapThreadHandler, this);
        mapThread.detach();
    }
}


void CompositeDlg::MapThreadHandler(CompositeDlg* pInst)
{
    pInst->MapThread();
}

void CompositeDlg::MapThread()
{

    int count = mBuildInputQueue.size();
    
    mBuildMutex.lock();

    while (!mBuildInputQueue.empty())
    {
        std::tuple<int, int, RgbValue> value = mBuildInputQueue.front();
        mBuildInputQueue.pop();

        mBuildMutex.unlock();

        int x = std::get<0>(value);
        int y = std::get<1>(value);
        std::string filename = FindClosest(std::get<2>(value));

        int progress = (100 * (count - mBuildInputQueue.size())) / count;

        mProgressBar->SetValue(progress);

        mBuildMutex.lock();

        if (mBuildTileMap.count(filename) == 0)
        {
            mBuildTileMap[filename] = {};
        }
        mBuildTileMap[filename].push_back(std::make_tuple(x, y));
    }

    mBuildMutex.unlock();
}

void CompositeDlg::OnBuildOutput(wxCommandEvent& event)
{
    for (std::map<std::string, std::list<std::tuple<int, int>>>::const_iterator it = mBuildTileMap.begin(); it != mBuildTileMap.end(); ++it)
    {
        BuildTile tile;
        tile.filename = it->first;
        tile.coords = it->second;
        mBuildTileQueue.push(tile);
    }

    mProgressBar->SetValue(0);

    int threads = wxThread::GetCPUCount() - 1;
    threads = (threads > 0) ? threads : 1;

    for (int i = 0; i < threads; i++)
    {
        std::thread buildThread(BuildOutputThreadHandler, this);
        buildThread.detach();
    }
}

void CompositeDlg::BuildOutputThreadHandler(CompositeDlg* pInst)
{

    pInst->BuildOutputThread();
}

void CompositeDlg::BuildOutputThread()
{

    int count = mBuildTileQueue.size();

    mBuildMutex.lock();

    while (!mBuildTileQueue.empty())
    {
        BuildTile value = mBuildTileQueue.front();
        mBuildTileQueue.pop();

        mBuildMutex.unlock();

        Mat image = LoadMat(value.filename);

        while (!value.coords.empty())
        {
            std::tuple<int, int> coords = value.coords.front();
            value.coords.pop_front();

            int x = std::get<0>(coords);
            int y = std::get<1>(coords);
            Rect roi(x, y, 1, 1);
            Mat localTarget = Mat(mOutputMat, roi);
            image.copyTo(localTarget);
        }

        int progress = (100 * (count - mBuildTileQueue.size())) / count;

        mProgressBar->SetValue(progress);

        mBuildMutex.lock();
    }

    mBuildMutex.unlock();

    mOutputPanel->SetNewImage(mOutputMat);
}
