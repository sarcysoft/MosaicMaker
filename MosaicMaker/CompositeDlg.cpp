#include "CompositeDlg.h"
#include "GlobalConfig.h"

#include <thread>
#include <vector>

#include "wx/thread.h"
#include "wx/wfstream.h"

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

    wxButton* btnSaveOutput = new wxButton(panel, ID_BuildOutput, wxT("Save Output"));
    hbox1->Add(btnSaveOutput);

    wxButton* btnViewOutput = new wxButton(panel, ID_ViewOutput, wxT("View Output"));
    hbox1->Add(btnViewOutput);

    vbox1->Add(hbox1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);


    mInputPanel = new wxImagePanel(panel);
    hbox2->Add(mInputPanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mOutputPanel = new wxImagePanel(panel);
    hbox2->Add(mOutputPanel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    vbox1->Add(hbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

    mProgressBar = new wxGauge(panel, wxID_ANY, 100);
    vbox1->Add(mProgressBar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 0);

    panel->SetSizer(vbox1);

    Bind(wxEVT_BUTTON, &CompositeDlg::OnViewSource, this, ID_ViewSource);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnMapOutput, this, ID_MapOutput);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnBuildOutput, this, ID_BuildOutput);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnSaveOutput, this, ID_SaveOutput);
    Bind(wxEVT_BUTTON, &CompositeDlg::OnViewOutput, this, ID_ViewOutput);

    SetFileList(fileList);

    if (mFileList.Count() > 0)
    {
        Mat image;
        image = imread(mFileList[0].ToStdString(), IMREAD_COLOR);
        mInputPanel->SetNewImage(image);
    }

    mOutputScale = 16;
    mThreshold = 1000;
}

void CompositeDlg::SetFileList(wxArrayString fileList)
{
    mFileList = fileList;
}

void CompositeDlg::SetImportList(wxString path, wxArrayString fileList)
{
    mImportList = fileList;
    mRgbMap.clear();

    mConfigPath = path + wxT("/config.ini");
    wxFileInputStream is(mConfigPath);
    if (is.IsOk())
    {
        mConfig = new wxFileConfig(is);
    }
    else
    {
        mConfig = new wxFileConfig();
    }

    for (int i = 0; i < mImportList.Count(); i++)
    {
        Mat image;
        mImportFileList.push_back(mImportList[i].ToStdString());
    }
    
    mUnsavedConfigs = 0;

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
    int count = mImportFileList.size();

    mProgressBar->SetValue(0);

    mImportMutex.lock();

    while (!mImportFileList.empty())
    {
        std::string filename = "";
        filename = mImportFileList.back();
        mImportFileList.pop_back();
    
        mImportMutex.unlock();

        RgbValue rgb;

        wxString hashCode = mConfig->Read(filename);
        if (hashCode == wxEmptyString)
        {
            Mat image = LoadMat(filename);
            Vec3b bgrPixel = image.at<Vec3b>(0, 0);
            rgb = RgbValue(bgrPixel.val);

            mConfigMutex.lock();

            mConfig->Write(filename, rgb.GetHashCode());
            mUnsavedConfigs++;

            if ((mUnsavedConfigs >= 250) || mImportFileList.empty())
            {
                wxFileOutputStream os(mConfigPath);
                if (os.IsOk())
                {
                    mConfig->Save(os);
                    os.Close();
                }
                mUnsavedConfigs = 0;
            }

            mConfigMutex.unlock();
        }
        else
        {
            rgb.SetFromHashCode(hashCode);
        }

        int progress = (100 * (count - mImportFileList.size())) / count;

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

        namedWindow("Source Image", WINDOW_NORMAL);
        imshow("Source Image", image);
    }
}

void CompositeDlg::OnViewOutput(wxCommandEvent& event)
{
    namedWindow("Output Image", WINDOW_NORMAL);
    imshow("Output Image", mOutputMat);
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

        if (dist < mThreshold)
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
    double scaleY = (double)image.rows / 600.0;
    double scale = (scaleX > scaleY) ? scaleX : scaleY;

    int sizeX = (int)((double)image.cols / scale);
    int sizeY = (int)((double)image.rows / scale);

    resize(image, image, Size(sizeX, sizeY), 0, 0, INTER_AREA);

    //mOutputMat = Mat3d(image.rows * mOutputScale, image.cols * mOutputScale);
    resize(image, mOutputMat, Size(sizeX * mOutputScale, sizeY * mOutputScale), 0, 0, INTER_AREA);

    cvtColor(mOutputMat, mOutputMat, COLOR_BGR2GRAY);
    cvtColor(mOutputMat, mOutputMat, COLOR_GRAY2BGR);

    for (int y = 0; y < image.rows; y++)
    {
        for (int x = 0; x < image.cols; x++)
        {
            Vec3b bgrPixel = image.at<Vec3b>(y, x);

            RgbValue rgb(bgrPixel.val);

            mBuildInputList.push_back(std::make_tuple(x, y, rgb));
        }
    }

    mProgressBar->SetValue(0);

    mBuildTileMap.clear();
    mUpdatedOutput = 0;

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

    int count = mBuildInputList.size();
    
    mBuildMutex.lock();

    while (!mBuildInputList.empty())
    {
        std::tuple<int, int, RgbValue> value = mBuildInputList.back();
        mBuildInputList.pop_back();

        mBuildMutex.unlock();

        int x = std::get<0>(value);
        int y = std::get<1>(value);
        std::string filename = FindClosest(std::get<2>(value));

#if true
        int progress = (100 * (count - mBuildInputList.size())) / count;
        mProgressBar->SetValue(progress);

        mBuildMutex.lock();

        if (mBuildTileMap.count(filename) == 0)
        {
            mBuildTileMap[filename] = {};
        }
        mBuildTileMap[filename].push_back(std::make_tuple(x, y));
#else
        Mat image = LoadMat(filename, mOutputScale);

        Mat dstRoi = mOutputMat(Rect(x * mOutputScale, y * mOutputScale, mOutputScale, mOutputScale));
        image.copyTo(dstRoi);

        mUpdatedOutput++;
        if ((mUpdatedOutput > 50) || mBuildTileList.empty())
        {
            mUpdatedOutput = 0;

            mOutputMutex.lock();
            mOutputPanel->SetNewImage(mOutputMat);
            mOutputMutex.unlock();
        }

        int progress = (100 * (count - mBuildInputList.size())) / count;
        mProgressBar->SetValue(progress);

        mBuildMutex.lock();
#endif
    }

    mBuildMutex.unlock();
}

bool TileSortFunc(BuildTile i, BuildTile j) { return (i.coords.size() <  j.coords.size()); }

void CompositeDlg::OnBuildOutput(wxCommandEvent& event)
{
    for (std::map<std::string, std::vector<std::tuple<int, int>>>::const_iterator it = mBuildTileMap.begin(); it != mBuildTileMap.end(); ++it)
    {
        BuildTile tile;
        tile.filename = it->first;
        tile.coords = it->second;
        mBuildTileList.push_back(tile);
    }

    std::sort(mBuildTileList.begin(), mBuildTileList.end(), TileSortFunc);

    mProgressBar->SetValue(0);
    mOutputPanel->SetNewImage(mOutputMat);

    mUpdatedOutput = 0;

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

    int count = mBuildTileList.size();

    mBuildMutex.lock();

    while (!mBuildTileList.empty())
    {
        BuildTile value = mBuildTileList.back();
        mBuildTileList.pop_back();

        mBuildMutex.unlock();

        Mat image = LoadMat(value.filename, mOutputScale);

        while (!value.coords.empty())
        {
            std::tuple<int, int> coords = value.coords.back();
            value.coords.pop_back();

            int x = std::get<0>(coords);
            int y = std::get<1>(coords);

            Mat dstRoi = mOutputMat(Rect(x * mOutputScale, y * mOutputScale, mOutputScale, mOutputScale));
            image.copyTo(dstRoi);
        }
       
        mUpdatedOutput++;
        if ((mUpdatedOutput > 50) || mBuildTileList.empty())
        {
            mUpdatedOutput = 0;
         
            mOutputMutex.lock();
            mOutputPanel->SetNewImage(mOutputMat);
            mOutputMutex.unlock();

            int progress = (100 * (count - mBuildTileList.size())) / count;
            mProgressBar->SetValue(progress);
        }
     
        mBuildMutex.lock();
    }

    mBuildMutex.unlock();
}

void CompositeDlg::OnSaveOutput(wxCommandEvent& event)
{
    imwrite("composite.png", mOutputMat);
}
