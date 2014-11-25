#ifndef MAIN_HEADER_H //header guard
#define MAIN_HEADER_H

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
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
        static string fileNameErase(string fileName);
};

class mammography: public various{
    public:
        long XRayTubeCurrent, Exposure, Rows, Columns, BitsAllocated, PixelRepresentation, SmallestImagePixelValue, LargestImagePixelValue;
        OFString KVP, BodyPartThickness, CompressionForce, Filter, Target;
        vector<Uint16> pixelVec;
    public:
        mammography(){
        };
        ~mammography() {}
        void loadHeaderData(const string fileName);
        void loadPixelData(const string fileName);
};

class phantomCalibration: public various{
    public:
        vector< pair<double, double> > dataArr;
    public:
        phantomCalibration(){
        };
        ~phantomCalibration() {}
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
        scanner(const int shiftIn): shift(shiftIn) { };
        ~scanner() {}
        // fit linear function into data
        static pair<double,double> linearfit(const double* x1, const double* y1, const int sizeArr);
        // calculate shift from the QC data point
        static double calcShift(const pair<double,double> coeff, const double x0, const double y);
};

class breast: public mammography, phantomCalibration{
    public:
        cv::Mat mMammo;
        cv::Mat mHistB;
        cv::Mat mMammoThreshed;
        cv::Mat mMammoDist;
        cv::Mat mCorner;
        cv::Mat mMammo8Bit;
    private:
        double breastVol, fibroVol;
        int iColourMAX;
        vector<cv::Mat> bgr_planes;
        vector<cv::Point> pEdgeContour;
        vector<vector<cv::Point>> pContours;
        vector<cv::Point> vecContCents;
        vector<float> vecDistBright;
        vector<float> vecDistBrightBrightest;
    public:
        breast(mammography mammData){
            mMammo = cv::Mat((int)mammData.Rows, (int)mammData.Columns, CV_16UC1, cv::Scalar(1));
            for(int i = 0; i < mammData.Columns; i++){
                for(int j = 0; j < mammData.Rows; j++){
                    mMammo.at<Uint16>(j,i) = mammData.pixelVec[i+(int)mammData.Columns*j];
                }
            }
	    mMammo.convertTo(mMammo8Bit, CV_8U, 1./256);
        };
        int getBitDepth();
        vector<cv::Mat> separate3channels();
        static void drawHist(const int histSize);
        pair<float, float> findPeak(const int histSize);
        float findWidth(const int iBinMax, const int iNMax);
        vector<cv::Point> distanceTransform();
        bool leftOrRight(vector<cv::Point> pEdgeContour);
        float findIMax();
        vector<vector<cv::Point>> findCorners(float iDivisor, const int iMax, const int iCOLOUR_MAX);
        vector<cv::Point> findCornerCentre();
        pair<int, int> pickCornerCutOff(const bool bLeft);
        void deleteUnneeded(const bool bLeft, cv::Mat mMammoThreshedCopy, const vector<cv::Point> pEdgeContourCopy, const int iContPosY);
        vector<float> getDistBright();
        vector<float> brestThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy);
        static vector<float> normalBreastThickness(vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
        void drawImages(string fileName, const cv::Mat distImage, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize);
        static double fibrogland(const cv::Mat imDat, const int thickness, const int exposure, calibData calibration);
        double totalBreast(const mammography mammData);
        static double glandpercent(const double tg, const double t);
        void thicknessMap(const pair<double,double> coeff3, const int exposure, const mammography mammData);
};

template <typename T> string various::ToString(T input){
    stringstream ss;
    ss << input;
    string output = ss.str();
    return output;
}

#endif // MAIN_HEADER_H
