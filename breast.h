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
        double totalBreast(const string filTar);
        pair<double,double> glandpercent(const phantomCalibration calib, const string filTar, const string kV, const double t);
        double glandpercentInverse(const double MPV, const string filTar, const string kV, const double exposure);
        double breastThickAtPixel(const phantomCalibration calib, const int i, const int j, const string filTar, const double t);
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

    /* CREATE THICKNESS MAP*/
    cv::Mat thicknessMap(const std::string bodyThickness, const std::pair<double,double> coeff3, const int exposure);

    /* FUNCTIONS FOR THE OVEREXPOSURE CORRECTION*/
    /* FIND OVEREXPOSED PIXELS*/
    vector<pair<int,int>> pixelOfInterestExposure();
    /* CREATE DISTANCE MAP OF THE OVEREXPOSED PIXELS*/
    map<int,vector<pair<double,pair<int,int>>>> distMap(vector<pair<int,int>> pixelOfInterestExposureVec);
    /* APPLY OVEREXPOSURE CORRECTION */
    void applyExposureCorrection(map<int,vector<pair<double,pair<int,int>>>> breastDistMap);
    /* CREATE A MAP OF THE OVEREXPOSED PIXELS*/
    void exposureMap(const pair<double,double> coeff3, const int exposure, const vector<pair<int,int>> pixelOfInterestExposureVec);

    /* FOLLOWIONG FUNCTIONS FUNCTIONS ARE USED TO GET THE BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE */
    /* FUNCTION TO GET THE BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE */
    vector<cv::Point> getContact(const string strKVP, const double exposure, const dailyCalibration dcalib);
    pair<cv::Point,float> straightLevel(const int row);
    /* FIRST ITERATION OF FINDING THE THE BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE  */
    vector<cv::Point> contactBorder(const string filTar);
    /* FIND BREAST BORDER */
    vector<cv::Point> breastBorder();
    /* CALCULATE AVERAGE DIFFERENCE BETWEEN THE BORDERLINE FIND IN THE FIRST ITERATION AND BREAST BORDER */
    double averageDiffBorder(const vector<cv::Point> contactBorder, const vector<cv::Point> breastBorder);
    /* SECOND ITERATION OF FINDING THE THE BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE */
    vector<cv::Point> contactBorderFinal(const vector<cv::Point> contactBorderVal, const double averageDiffBorderVal);
    /* FINDING FAT PIXELS IN THE FULLY COMPRESSED REGION OF A BREAST */
    vector<cv::Point> pointsFatForPlane(vector<cv::Point> breastFatPoints, const double averageDiffVal);
    /* FIT A PLANE INTO FAT PIXELS */
    vector<float> fitPlane(vector<cv::Point> breastFatDiscarded, const string filTar);
    /* CALCULATE AVERAGE DISTANCE BETWEEN FITTED PLANE AND STRAIGHT PLANE */
    double averageDistance(const vector<float> plane, vector<cv::Point> breastFatDiscarded, const string filTar);
    /* FINAL ITERATION OF FINDING THE BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE */
    vector<cv::Point> contactBorder2(const vector<float> plane, vector<cv::Point> contactBorderFin, const double averageDiffVal, const string filTar);
    /* CORRECTED BORDERLINE, WHERE A BREAST LOSES THE CONTACT WITH THE COMPRESSION PADDLE */
    vector<cv::Point> contactBorderCorrection(vector<cv::Point> contactBorder2Vec);
    /* CALCULATE THE ANGLE OF COMPRESSION PADDLE TILT */
    static double getPlaneAngle(const vector<float> plane);

    /* CALCULATE FIBROGLANDULAR TISSUE AMOUNT*/
    double fibrogland(const phantomCalibration calib, const string filTar, double &totalThickness);

    void dailyCorrectionTVsB(const string filTar, const string kV, phantomCalibration calib);
	// DEPRECATED STUFF?
        static std::vector<float> normalBreastThickness(std::vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
};


