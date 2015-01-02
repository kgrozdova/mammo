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

class breast: public mammography, phantomCalibration{
    private:
	
	// Containers
	/* double breastVol, fibroVol; */
	int iColourMAX;
	std::vector<cv::Mat> bgr_planes;
	std::vector<cv::Point> pEdgeContour;
	std::vector<std::vector<cv::Point>> pContours;
	std::vector<cv::Point> vecContCents;
	std::vector<float> vecDistBright;
	std::vector<float> vecDistBrightBrightest;
	double dMaxPixelValue;
	double dMinPixelValue;
	bool bLeft;
	int iHistSize = 512;

	// Functions
	void pixelVec2Mat();
	void getBreastROI();
	void getBreastBottom();
	void getBreastDistMap();
	void getBreastEdge();
	std::pair<float, float> findHistPeak();

    public:

	// Containers
	cv::Mat mMammo;
	cv::Mat mHist;
	cv::Mat mMammoROI;
	cv::Mat mMammoDist;
	cv::Mat mCorner;
	cv::Mat mMammo8Bit;
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
        std::vector<float> getDistBright();
        std::vector<float> brestThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy);
        static std::vector<float> normalBreastThickness(std::vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
        void drawImages(std::string fileName, const cv::Mat distImage, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize);
        static double fibrogland(const cv::Mat imDat, const int thickness, const int exposure, calibData calibration);
        double totalBreast();
        static double glandpercent(const double tg, const double t);
        void thicknessMap(const std::pair<double,double> coeff3, const int exposure);
        void thicknessMapRedVal(const pair<double,double> coeff3, const int exposure);

};


