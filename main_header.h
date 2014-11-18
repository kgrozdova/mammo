#ifndef MAIN_HEADER_H //header guard
#define MAIN_HEADER_H

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

class various{
    public:
        template <typename T> static string ToString(T input);
        static void iterateVectorToFile(vector<pair<double, double>> vectorInput, string fileName);
};

class mammography: public various{
    public:
        long XRayTubeCurrent, Exposure, Rows, Columns, BitsAllocated, PixelRepresentation, SmallestImagePixelValue, LargestImagePixelValue;
        OFString KVP, BodyPartThickness, CompressionForce, Filter, Target;
        vector<Uint16> pixelVec;
    public:
        mammography(){
            cout << "mammography constructor is called" << endl;
        };
        ~mammography() {cout << "mammography destructor is called" << endl;}
        void loadHeaderData(const string fileName);
        void loadPixelData(const string fileName);
};

class phantomCalibration: public various{
    public:
        vector< pair<double, double> > dataArr;
    public:
        phantomCalibration(){
            cout << "phantomCalibration constructor is called" << endl;
        };
        ~phantomCalibration() {cout << "phantomCalibration destructor is called" << endl;}
        // input the fibroglandular tissue thickness value into dataArr
        void inputData(const double t, const double ln_MPV_mAs);
        // get the phantomCalibration class object with dataArr containing the data for given total thickness
        static phantomCalibration getThicknessData(const string filTar, const int kV, const int t);
        void applyShift(const double shift);
        void dataCorrection(const double x0, const double y0, const int kV, const string filTar, const int t);
};

typedef map< string, phantomCalibration> calibData;

class dailyCalibration: public mammography{
    public:
        double qc_ln_MPV_mAs;
        double t;
        double tg;
        string filTar;
    public:
        void insertFilTar(mammography mammData);
        void InserQcTTg(mammography mammData, const string fileName);
};

class scanner: public phantomCalibration, dailyCalibration{
    public:
        double shift;
    public:
        scanner(const int shiftIn): shift(shiftIn) { cout << "scanner constructor is called" << endl; };
        ~scanner() {cout << "scanner destructor is called" << endl;}
        // fit linear function into data
        static pair<double,double> linearfit(const double* x1, const double* y1, const int sizeArr);
        // calculate shift from the QC data point
        static double calcShift(const pair<double,double> coeff, const double x0, const double y);
};

class breast: public mammography, phantomCalibration{
    public:
        double breastVol, fibroVol;
    public:
        static cv::Mat getMat(const string fileName);
        static string fileNameErase(string fileName);
        static int getBitDepth(cv::Mat mMammo);
        static vector<cv::Mat> separate3channels(cv::Mat mMammo);
        static void drawHist(const int histSize);
        static pair<float, float> findPeak(const cv::Mat mHistB, const int histSize);
        static float findWidth(const int iBinMax, const int iNMax, const cv::Mat mHistB);
        static vector<cv::Point> distanceTransform(const cv::Mat mMammoThreshed);
        static bool leftOrRight(vector<cv::Point> pEdgeContour, const cv::Mat mMammo);
        static float findIMax(const cv::Mat mCorner);
        static vector<vector<cv::Point>> findCorners(float iDivisor, const cv::Mat mCorner, const int iMax, const int iCOLOUR_MAX);
        static vector<cv::Point> findCornerCentre(vector<vector<cv::Point>> pContours);
        static pair<int, int> pickCornerCutOff(const cv::Mat mMammo, const vector<vector<cv::Point>> pContours,
                                        vector<cv::Point> vecContCents, const bool bLeft);
        static void deleteUnneeded(const cv::Mat mMammo, const vector<cv::Point> pEdgeContourCopy, const bool bLeft, cv::Mat mMammoThreshed,
                            cv::Mat mMammoThreshedCopy, const int iContPosY);
        static vector<float> getDistBright(const cv::Mat_<int> mMammoDist, cv::Mat mMammo);
        static vector<float> brestThickness(const cv::Mat mMammo, const cv::Mat_<int> mMammoDist, const vector<float> vecDistBright,
                                        const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy);
        static vector<float> normalBreastThickness(vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
        static void drawImages(const vector<float> vecDistBrightBrightest, const string fileName, const cv::Mat distImage, const cv::Mat mCornerTresh,
                        const cv::Mat mMammoDist, const cv::Mat mMammoThreshedCopy, const cv::Mat mMammo, const int histSize);

        static double fibrogland(mammography mammData, calibData calibration);
        static double totalBreast(mammography mammData);
        static double glandpercent(const double tg, const double t);
};

template <typename T> string various::ToString(T input){
    stringstream ss;
    ss << input;
    string output = ss.str();
    return output;
}

#endif // MAIN_HEADER_H


