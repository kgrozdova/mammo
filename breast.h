#pragma once

/* #include "mammography.h" */
/* #include "phantomCalibration.h" */
#include "main_header.h"
#include <vector>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
/* #include "stdafx.h" */
/* #include "interpolation.h" */


#define XIN_BACKGROUND 1
#define XIN_FAT 2
#define XIN_PECTORAL_MUSCLE 3
#define XIN_GLAND 4
#define XIN_NIPPLE 5
#define XIN_DENSER_GLAND 6

class breast: public mammography, phantomCalibration{
    private:

	// Containers
	/* double breastVol, fibroVol; */
	int iColourMAX;
	std::vector<cv::Mat> bgr_planes;
	std::vector<cv::Point> pEdgeContour;
	std::vector<cv::Point> pEdgeContourCompressed;
	std::vector<std::vector<cv::Point>> pContours;
	std::vector<cv::Point> vecContCents;
	std::vector<float> vecDistBright;
	std::vector<float> vecDistBrightBrightest;
	std::vector<std::vector<cv::Point>> vecPointsAtDist;
	cv::Point findNeighboursOnDistance(cv::Point p);
	cv::Mat mChenFatClass;
	double dMaxPixelValue;
	double dMinPixelValue;
	bool bLeft;	// "is the breast coming from the left of the image (always true)"
	bool bLeftNEW;	// "is this the left or right hand breast?"
	int iHistSize = 512;

	// Functions
	void pixelVec2Mat();
	void getBreastROI();
	void getBreastBottom();
	void getBreastDistMap();
	void getBreastEdge();
	std::pair<float, float> findHistPeak();
	std::pair<float, float> findHistPeakLeft();
	std::pair<float, float> findHistPeakRight();
	void getDensityROI();

    public:

	// Containers
	cv::Mat mMammo;
	cv::Mat mMammoNorm;
	cv::Mat mHist;
	cv::Mat mMammoROI;
	cv::Mat mMammoROISmaller; // ROI without skin folds
	cv::Mat mMammoDist;
	cv::Mat mCorner;
	cv::Mat mMammo8Bit;
	cv::Mat mMammo8BitNorm;
	cv::Mat mMammoDistImage;
	cv::Mat mMammoFatROI;
	cv::Mat mHeightMap;
	cv::Mat mHeightMap16;
	Uint32 dMeanBackgroundValue = 0;

	// Functions
	breast(std::string t_strFileName);
        int getBitDepth();
        void drawHist();
        float findWidth(const int iBinMax, const int iNMax);
        bool leftOrRight();
        std::vector<std::vector<cv::Point>> findCorners(float iDivisor, const int iMax, const int iCOLOUR_MAX);
        std::vector<cv::Point> findCornerCentre();
        std::pair<int, int> pickCornerCutOff(const bool bLeft);
        void deleteUnneeded(const bool bLeft, cv::Mat mMammoThreshedCopy, const std::vector<cv::Point> pEdgeContourCopy, const int iContPosY);
        void drawImages(std::string fileName, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize);
        static double fibrogland(const cv::Mat imDat, const int thickness, const int exposure, calibData calibration);
        double totalBreast(const phantomCalibration calib, const string filTar, const string kV, const double exposure);
        static pair<double,double> glandpercent(const phantomCalibration calib, const string filTar, const string kV, const double t);
        static double glandpercentInverse(const double MPV, const phantomCalibration calib, const string filTar, const string kV, const double exposure);
        double breastThickAtPixel(const int i, const int j, const phantomCalibration calib, const string filTar, const string kV, const double t, const double exposure);
        void thicknessMapRedVal(const pair<double,double> coeff3, const int exposure);

	// THICKNESS RELATED STUFF
        std::vector<float> getDistBright();
        std::vector<float> breastThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy);
	void getRadialThickness();

	// FRIENDLY FUNCTIONS FOR FINDING INFORMATION ABOUT BREAST (consider making other functions private!)
	Uint16 getHeight(int x, int y); // Returns the height of the pixel in units to be determined.
	bool isBreast(int x, int y);	// Is a pixel is breast or background?
	bool isBreastROI(int x, int y); // Is a pixel within our ROI? (i.e. breast, and not pectoral muscle / skin fold)
	bool isFat(int x, int y);	// Is a pixel is fat?
	void makeXinROIMap();
	uchar getAvNhood8(cv::Mat &mat, cv::Point &p, int nhood);
	int getPixelType(int x, int y); // What type is a pixel?

    void thicknessMapRedValBorder(const pair<double,double> coeff3, const int exposure, const vector<cv::Point> contactBorderShapeVal);

    vector<pair<int,int>> pixelOfInterestExposure();
    map<int,vector<pair<double,pair<int,int>>>> distMap(vector<pair<int,int>> pixelOfInterestExposureVec);
    void applyExposureCorrection(map<int,vector<pair<double,pair<int,int>>>> breastDistMap);
    void exposureMap(const pair<double,double> coeff3, const int exposure, const vector<pair<int,int>> pixelOfInterestExposureVec);

    string heightRowArray(vector<cv::Point> contactBorder2Vec, const string xOrY);
    pair<cv::Point,float> straightLevel(const int row);
    vector<cv::Point> contactBorder();
    double averageDiffBorder(const vector<cv::Point> contactBorder, const vector<cv::Point> breastBorder);
    vector<cv::Point> breastBorder();
    vector<cv::Point> contactBorderFinal(const vector<cv::Point> contactBorderVal, const double averageDiffBorderVal);
    vector<cv::Point> contactBorderFinalIt2(vector<cv::Point> contactBorderValFin, const double averageDiffBorderVal);
    vector<cv::Point> getContact();
    vector<cv::Point> pointsFatForPlane(vector<cv::Point> breastFatPoints, const double averageDiffVal);
    vector<float> fitPlane(vector<cv::Point> breastFatDiscarded);
    void fatRow(const int row);
    static double getPlaneAngle(const vector<float> plane);
    double averageDistance(const vector<float> plane, vector<cv::Point> breastFatDiscarded);
    vector<cv::Point> contactBorder2(const vector<float> plane, vector<cv::Point> contactBorderFin, const double averageDiffVal);
    vector<cv::Point> contactBorderCorrection(vector<cv::Point> contactBorder2Vec);

    double fibrogland(const phantomCalibration calib, const string strKVP, const double exposure, const dailyCalibration dcalib);
	// DEPRECATED STUFF?
        static std::vector<float> normalBreastThickness(std::vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
};


