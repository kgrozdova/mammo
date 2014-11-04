#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <algorithm>

#define OL_WIDTH 512 
#define OL_EXT_LENGTH 4
//#define OL_DRAW
//#define OL_DRAW_ALTERED
#ifdef OL_DRAW
	#define OL_DRAW_HIST
	#define OL_DRAW_THRESH
	#define OL_DRAW_DIST
	#define OL_DRAW_CORNER
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

	cv::Mat	mMammoThreshedCopy = mMammoThreshed;
	// Convert the threshold into greyscale to stop the distance transform complaining.
	cv::cvtColor(mMammoThreshed,mMammoThreshed, cv::COLOR_BGR2GRAY);

	// Use a less accurate but smoother looking distance transform. More research needed here.
	cv::distanceTransform(mMammoThreshed, mMammoDist, cv::DIST_L2, cv::DIST_MASK_PRECISE);

	cv::Mat mMammoThreshedCont;
	mMammoThreshed.convertTo(mMammoThreshedCont, CV_8U);
	vector<vector<cv::Point>> pEdgeContours;
	cv::findContours(mMammoThreshed,pEdgeContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);

	int iContSize = 0;
	vector<cv::Point> pEdgeContour;
	for(auto i:pEdgeContours){
		if(iContSize < i.size()){
			iContSize = i.size();
			pEdgeContour = i;
		}
	}
	vector<cv::Point> pEdgeContourCopy = pEdgeContour;
	sort(pEdgeContour.begin(),pEdgeContour.end(),[](const cv::Point &l, const cv::Point &r){return l.x < r.x;});
	sort(pEdgeContourCopy.begin(),pEdgeContourCopy.end(),[](const cv::Point &l, const cv::Point &r){return l.y < r.y;});
	int iXLast = -1;
	int iYLast = -1;
	int i2LastGap = 0;
	int iCurrGap = 0;
	int iTotalGap = 0;
	for(auto i:pEdgeContour){
		if (i.x == iXLast){
			iCurrGap = abs(iYLast-i.y);
			if(iCurrGap > 0.1*mMammo.rows){
				if(i2LastGap != 0){
					iTotalGap += i2LastGap - iCurrGap;
				}
			}
		} else {
			iXLast = i.x;
			iYLast = i.y;
			i2LastGap = iCurrGap;
		}
	}
	bool bLeft = iTotalGap < 0;	

	// Make a probability map of likely corners in the mammogram.
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

	// Threshold this map repeatedly until we find at least three probable regions.
	// NEED TO FIND A BETTER WAY OF FINDING CORNERS
	cv::Mat mCornerThresh;
	int iCorners;
	float iDivisor = 1.25;
	vector<vector<cv::Point>> pContours;
	do{
		cv::threshold(mCorner,mCornerThresh,iMax/iDivisor,iCOLOUR_MAX,0);
		cv::Mat mCornerT8U;
		mCornerThresh.convertTo(mCornerT8U, CV_8U);
		
		cv::findContours(mCornerT8U,pContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
		iCorners = pContours.size();	
		iDivisor+=0.05;
	
	} while ((iCorners < 5) && (iDivisor < 10));
	
	// Find the most spatially spread out corner. Unused, currently - what does it mean?	
	sort(pContours.begin(),pContours.end(),[]
		(const vector<cv::Point> &l, const vector<cv::Point> &r){
			return cv::contourArea(l) > cv::contourArea(r);
		});

	// Find the centers of the corners.
	vector<cv::Point> vecContCents;
	for(auto i:pContours){
		cv::Moments momCont = cv::moments(i);
		vecContCents.push_back(cv::Point(momCont.m10/momCont.m00,momCont.m01/momCont.m00));
	}

	// Pick a corner to cut off.
	int iContPosY = mMammo.rows;
	int iContPosX;
	for(int i = 0; i < pContours.size(); i++){
		if(vecContCents[i].y > 2*float(mMammo.rows)/3){
			if(bLeft){
				if(vecContCents[i].x < float(0.25*mMammo.cols)){
					iContPosY = min(vecContCents[i].y,iContPosY);
					iContPosX = vecContCents[i].x;
				}
			} else {
				if(vecContCents[i].x > float(0.75*mMammo.cols)){
					iContPosY = min(vecContCents[i].y,iContPosY);
					iContPosX = vecContCents[i].x;
				}
			}
		}
	}
	
	if (iContPosY < mMammo.rows){
		int extremalX=pEdgeContourCopy[0].x;
		int lastY=pEdgeContourCopy[0].y;
		int lastX=pEdgeContourCopy[0].x;
		vector<cv::Point> pEdgeThrowAway;
		for(auto i:pEdgeContourCopy){
			if(lastY == i.y){
					extremalX = bLeft?
							((i.x>1)?min(extremalX,i.x):extremalX)
						:
							((i.x<mMammo.cols-5)?max(extremalX,i.x):extremalX);
			} else {
				if(lastY >= iContPosY){
					pEdgeThrowAway.push_back(cv::Point(extremalX,lastY));
				}
				extremalX = bLeft?((i.x>1)?i.x:extremalX):((i.x<mMammo.cols-5)?i.x:extremalX);
			}
			lastY = i.y;
		}

		// CURRENT PROBLEM:
		//		YOU DON'T UNDERSTAND WHAT at<> DOES.
		//		IT WORKS FOR LEFT, BUT NOT FOR RIGHT.
		//		I SUSPECT at<> IS DRAWING MORE PIXELS THAN WE EXPECT.
		for(auto i:pEdgeThrowAway){
			if(bLeft){
				for(int x = 0; x <= i.x; x++){
					mMammoThreshedCopy.at<int>(i.y, x) = 0;
				}
			} else {
				for(int x = i.x; x < mMammoThreshed.cols; x++){
					mMammoThreshedCopy.at<int>(i.y, x) = 0;
				}
			}
		}
	}

// Distance/brightness map
vector<float> vecDistBright;
cv::Mat_<uchar> mMammoDistChar = mMammoDist;
vecDistBright.resize(256);
for(int i = 0; i < vecDistBright.size(); ++i){ vecDistBright[i] = uchar(0);}
for(int i = 0; i < mMammo.cols; i++){
	for(int j = 0; j < mMammo.rows; j++){
		//vecDistBright[int(255*float(mMammoDist.at<int>(j,i))/2147483647)]+=float(mMammo.at<uchar>(j,i));
		vecDistBright[int(mMammoDistChar(j,i))]+=float(mMammo.at<uchar>(j,i));
	}
}
int dist_w = histSize*2; int dist_h = 512;
cv::Mat distImage(dist_h, dist_w, CV_8UC3, cv::Scalar(0,0,0));

// Normalize the result to [ 0, histImage.rows ].
vecDistBright[255]=0;
vecDistBright[0]=0;
cv::normalize(vecDistBright, vecDistBright, 0, distImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

int bin_w = cvRound(double(dist_w/histSize));
for(int i = 1; i < histSize; i++){
	line(distImage, cv::Point(bin_w*(i), dist_h - cvRound(vecDistBright[i])) ,
					cv::Point(bin_w*(i), dist_h - cvRound(vecDistBright[i])),
					cv::Scalar(255, 255, 255), 2, 8, 0);
}
cv::imwrite(strFilename+"_dist.jpg", distImage );

#ifdef OL_DRAW_CORNER
	cv::imwrite(strFilename+"_corner.jpg", mCornerThresh);
#endif 
	
#ifdef OL_DRAW_DIST
	cv::imwrite(strFilename+"_dist.jpg", mMammoDist);
#endif

#ifdef OL_DRAW_THRESH
	cv::imwrite(strFilename+"_thresh.jpg", mMammoThreshedCopy);
#endif

#ifdef OL_DRAW_ALTERED
	cv::imwrite(strFilename+"_alt.jpg", mMammo);
#endif


	return 0;
}
