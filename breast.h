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
	double breastVol, fibroVol;
	int iColourMAX;
	std::vector<cv::Mat> bgr_planes;
	std::vector<cv::Point> pEdgeContour;
	std::vector<std::vector<cv::Point>> pContours;
	std::vector<cv::Point> vecContCents;
	std::vector<float> vecDistBright;
	std::vector<float> vecDistBrightBrightest;
    public:
	cv::Mat mMammo;
	cv::Mat mHistB;
	cv::Mat mMammoThreshed;
	cv::Mat mMammoDist;
	cv::Mat mCorner;
	cv::Mat mMammo8Bit;
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
        std::vector<cv::Mat> separate3channels();
        static void drawHist(const int histSize);
	std::pair<float, float> findPeak(const int histSize);
        float findWidth(const int iBinMax, const int iNMax);
        std::vector<cv::Point> distanceTransform();
        bool leftOrRight(std::vector<cv::Point> pEdgeContour);
        float findIMax();
        std::vector<std::vector<cv::Point>> findCorners(float iDivisor, const int iMax, const int iCOLOUR_MAX);
        std::vector<cv::Point> findCornerCentre();
	std::pair<int, int> pickCornerCutOff(const bool bLeft);
        void deleteUnneeded(const bool bLeft, cv::Mat mMammoThreshedCopy, const std::vector<cv::Point> pEdgeContourCopy, const int iContPosY);
        std::vector<float> getDistBright();
        std::vector<float> brestThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy);
        static std::vector<float> normalBreastThickness(std::vector<float> vecDistBrightBrightestm, const cv::Mat distImage);
        void drawImages(std::string fileName, const cv::Mat distImage, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize);
        static double fibrogland(const cv::Mat imDat, const int thickness, const int exposure, calibData calibration);
        double totalBreast(const mammography mammData);
        static double glandpercent(const double tg, const double t);
        void thicknessMap(const std::pair<double,double> coeff3, const int exposure, const mammography mammData);
};


