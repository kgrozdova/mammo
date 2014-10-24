#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>

#define OL_WIDTH 512 
#define OL_EXT_LENGTH 4
//#define OL_DRAW
#ifdef OL_DRAW
	#define OL_DRAW_HIST
	#define OL_DRAW_THRESH
	#define OL_DRAW_DIST
#endif
//using namespace cv;
using namespace std;

int main(int argc, char** argv){

	string strFilename = argv[1];
	// Load image supplied through command line.
	cv::Mat mMammo = cv::imread(strFilename, 1);
	if(!mMammo.data){
		return -1;
	}
	strFilename.erase(strFilename.length()-OL_EXT_LENGTH, strFilename.length()); // Remove file extension.

	// Find bit depth of image and store white value.
	int iCOLOUR_MAX;
	switch(mMammo.depth()){
		case CV_8U:
			iCOLOUR_MAX = 255;
			break;
		case CV_8S:
			iCOLOUR_MAX = 127;
			break;
		case CV_16U:
			iCOLOUR_MAX = 65535;
			break;
		case CV_16S:
			iCOLOUR_MAX = 32767;
			break;
		case CV_32S:
			iCOLOUR_MAX = 2147483647;
			break;
		case CV_32F:
		case CV_64F:
			exit(1); // Can't cope with that many colours.
			break;
	}

	// Separate the image into 3 channels.
	vector<cv::Mat> bgr_planes;
	cv::split(mMammo, bgr_planes);

	// Establish the number of bins
	int histSize = iCOLOUR_MAX;

	// Set the ranges (for B,G,R) )
	float range[] = {0, float(iCOLOUR_MAX)+1} ;
	const float* histRange = {range};

	// Set histogram behaviour.
	bool uniform = true; bool accumulate = false;

	cv::Mat mHistB;

	// Compute the histogram.
	cv::calcHist(&bgr_planes[0], 1, 0, cv::Mat(), mHistB, 1, &histSize, &histRange, uniform, accumulate );

	// Draw it. 
#ifdef OL_DRAW_HIST
	int hist_w = histSize; int hist_h = 512;
	cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0,0,0));

	// Normalize the result to [ 0, histImage.rows ].
	cv::normalize(mHistB, mHistB, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

	float imMax = 0;
	int bin_w = cvRound(double(hist_w/histSize));
	float iMax = 0;
	for(int i = 1; i < histSize; i++){
		line(histImage, cv::Point(bin_w*(i-1), hist_h - cvRound(mHistB.at<float>(i-1))) ,
						cv::Point(bin_w*(i), hist_h - cvRound(mHistB.at<float>(i))),
						cv::Scalar(255, 255, 255), 2, 8, 0);
		if(imMax < mHistB.at<float>(i)){
			imMax = mHistB.at<float>(i);
			iMax = i;
		}
	}
	cv::imwrite(strFilename+"_hist.jpg", histImage );
#endif

	// Find the peak in the brightest half of the image.
	float iNMax = 0;
	float iBinMax = 0;
	for(int i = histSize - 1; i > histSize*0.5; i--){
		if(iNMax < mHistB.at<float>(i)){
			iNMax = mHistB.at<float>(i);
			iBinMax = i;
		}
	}

	// Find the width at the value with PEAK_VALUE/OL_WIDTH.
	float iQuartMax = 0;
	for(int i = iBinMax; i > 0; i--){
		if(mHistB.at<float>(i) < iNMax/OL_WIDTH){	
			iQuartMax = i;
			break;
		}
	}

	cv::Mat mMammoDist, mMammoThreshed;

	// Threshold the image to 'cut off' the brighter peak from the histogram.
	cv::threshold(mMammo, mMammoThreshed, iCOLOUR_MAX*(iQuartMax/histSize), iCOLOUR_MAX, 1);

	// Convert the threshold into greyscale to stop the distance transform complaining.
	cv::cvtColor(mMammoThreshed,mMammoThreshed, cv::COLOR_BGR2GRAY);

	// Use a less accurate but smoother looking distance transform. More research needed here.
	cv::distanceTransform(mMammoThreshed, mMammoDist, cv::DIST_L2, cv::DIST_MASK_PRECISE);

	cv::Mat mCorner;
	cv::cornerHarris(mMammoThreshed, mCorner, 40, 3, 0.04);
	float iMax = 0;
	float iiMax = 0;
	float ijMax = 0;
	for(int i = 0; i < mCorner.cols; i++){
		for(int j = 0; j < mCorner.rows; j++){
			if (mCorner.at<float>(i,j) != 0){
				//iMax = max(iMax,mCorner.at<float>(i,j));
				if (iMax < mCorner.at<float>(i,j)){
					iMax = mCorner.at<float>(i,j);
					iiMax = i;
					ijMax = j;
				}
			}
		}
	}
	cv::Mat mCornerThresh;
	int iCorners;
	float iDivisor = 1;
	do{
		cv::threshold(mCorner,mCornerThresh,iMax/iDivisor,iCOLOUR_MAX,0);
		cv::Mat mCornerT8U;
		mCornerThresh.convertTo(mCornerT8U, CV_8U);
		vector<vector<cv::Point>> pContours;
		cv::findContours(mCornerT8U,pContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
		iCorners = pContours.size();	
		iDivisor+=0.05;
	} while ((iCorners < 3) && (iDivisor < 10));

	cv::imwrite(strFilename+"_corner.jpg", mCornerThresh);

	// Write it to disk.

#ifdef OL_DRAW_DIST
	cv::imwrite(strFilename+"_dist.jpg", mMammoDist);
#endif

#ifdef OL_DRAW_THRESH
	cv::imwrite(strFilename+"_thresh.jpg", mMammoThreshed);
#endif


	return 0;
}
