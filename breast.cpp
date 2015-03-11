#include "breast.h"

#define OL_WIDTH 512
#define OL_EXT_LENGTH 4
#define OL_DRAW
#ifdef OL_DRAW
//	#define OL_DRAW_HIST
#define OL_DRAW_THRESH
#define OL_DRAW_DIST
	#define OL_DRAW_DISTMAP
	/* #define OL_DRAW_CORNER */
#endif

typedef map< string, phantomCalibration> calibData;

breast::breast(std::string t_strFileName): mammography(t_strFileName){
    this->strFileName = breast::fileNameErase(t_strFileName);
    this->pixelVec2Mat();
    this->mChenFatClass = cv::imread(this->strFileName+"Label.tiff");
    cv::resize(mChenFatClass,mChenFatClass,this->mMammo.size());
    cv::minMaxLoc(mMammo,&this->dMinPixelValue,&this->dMaxPixelValue);
    this->getBreastROI();
    this->getBreastDistMap();
    this->getBreastEdge();
    this->leftOrRight();    // Needs to go after getBreastEdge
			    // We need to replace all these things with
			    // functions that automatically calculate
			    // the things they need when the are first
			    // accessed
    if(bLeft){
    this->getBreastBottom();
    this->getRadialThickness();
    this->makeXinROIMap();
    }
}

void breast::pixelVec2Mat(){
    this->mMammo = cv::Mat((int)this->Rows, (int)this->Columns, CV_16UC1, cv::Scalar(1));
    for(int i = 0; i < this->Columns; i++){
	for(int j = 0; j < this->Rows; j++){
	    this->mMammo.at<Uint16>(j,i) = this->pixelVec[i+(int)this->Columns*j];
	}
    }
    cv::normalize(mMammo,mMammoNorm,0,65536, cv::NORM_MINMAX, -1, cv::Mat()); // NB: absolute pixel values meaningless in norm'd images
    this->mMammo.convertTo(mMammo8Bit, CV_8U, 1./256); // Leaves pixel values proportional to 16 bit / 8 bit but loses a lot of resolution
    this->mMammoNorm.convertTo(mMammo8BitNorm, CV_8U, 1./256);
    this->pixelVec.clear();
    this->getBitDepth();
}

void breast::getBreastROI(){
    // Establish the number of bins
    // Set the ranges (for B,G,R) )
    float range[] = {static_cast<float>(dMinPixelValue), static_cast<float>(dMaxPixelValue)} ;
    const float* fHistRange = {range};
    // Set histogram behaviour.
    bool bUniform = true; bool bAccumulate = false;
    // Compute the histogram.
    cv::calcHist(&mMammo, 1, 0, cv::Mat(), this->mHist, 1, &this->iHistSize, &fHistRange, bUniform, bAccumulate );
    this->drawHist();
    pair<float,float> pPeakValAndLoc = this->findHistPeak();
    double dRangeActual = dMaxPixelValue - dMinPixelValue;
    double dHistActualRangeRatio = dRangeActual/double(this->iHistSize);
    int iPeakVal = dHistActualRangeRatio * pPeakValAndLoc.second + this-> dMinPixelValue;
    int iPeakVal8Bit = iPeakVal/256;
    /* cv::Mat mMammoROICopy = this->mMammoROI.clone(); */
    cv::threshold(this->mMammo8Bit, this->mMammoROI, iPeakVal8Bit, 255, 1);


    // Calculate the average intensity of the background
    int iBackgroundSize = 0;
    for(int i = 0; i < mMammoROI.cols; i++){
	for(int j = 0; j < mMammoROI.rows; j++){
	    if(mMammoROI.at<uchar>(j,i) == 0){
		dMeanBackgroundValue += mMammo.at<Uint16>(j,i);
		iBackgroundSize++;
	    }
	}
    }
    dMeanBackgroundValue/=iBackgroundSize;

}

void breast::getBreastDistMap(){
    cv::distanceTransform(this->mMammoROI.clone(), this->mMammoDist, cv::DIST_L2, cv::DIST_MASK_PRECISE, CV_32F);
}

void breast::getBreastEdge(){
    cv::Mat mMammoThreshedCont;
    this->mMammoROI.convertTo(mMammoThreshedCont, CV_8U, 1./256);
    std::vector<std::vector<cv::Point>> pEdgeContours;
    std::vector<std::vector<cv::Point>> pEdgeContoursCompressed;
    //cv::findContours(this->mMammoROI.clone(),pEdgeContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);


    // Use .clone of ROI as this function would otherwise write to ROI.
    cv::findContours(this->mMammoROI.clone(),pEdgeContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);  // This works with the density calculation but ruins the thresholded image...
    cv::findContours(this->mMammoROI.clone(),pEdgeContoursCompressed,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);  // This works with the density calculation but ruins the thresholded image...

    int iContSize = 0;
    for(auto i:pEdgeContours){
	if(iContSize < (int)i.size()){
	    iContSize = i.size();
	    this->pEdgeContour = i;
	}
    }

    iContSize = 0;
    for(auto i:pEdgeContoursCompressed){
	if(iContSize < (int)i.size()){
	    iContSize = i.size();
	    this->pEdgeContourCompressed = i;
	}
    }
}

int breast::getBitDepth(){
    // Find bit depth of image and store white value.
    switch(mMammo.depth()){
	case CV_8U:
	    this->iColourMAX = 255;
	    break;
	case CV_8S:
	    this->iColourMAX = 127;
	    break;
	case CV_16U:
	    this->iColourMAX = 65535;
	    break;
	case CV_16S:
	    this->iColourMAX = 32767;
	    break;
	case CV_32S:
	    this->iColourMAX = 2147483647;
	    break;
	case CV_32F:
	case CV_64F:
	    exit(1); // Can't cope with that many colours.
	    break;
    }

    return this->iColourMAX;
}

void breast::getBreastBottom(){
    cv::cornerHarris(this->mMammoROI, this->mCorner, 10, 1, 0.01); // This used to work - now it doesn't seem to pick corners out at all.
    cv::Mat mCornerThresh;
    /* float iDivisor = 1.24; */
    float iDivisor = 1;
    double iMax,iMin;
    cv::minMaxLoc(mCorner, &iMin, &iMax);
    int iColourMax = this->getBitDepth();
    vector<vector<cv::Point>> pContours = this->findCorners(iDivisor, iMax, iColourMax);
    /* for (auto &i:pContours) { */
	/* cout << i[0] << "\t"; */
    /* } */
    /* cout << endl; */
    vector<cv::Point> vecContCents = this->findCornerCentre();
    /* for (auto &i:vecContCents){ */
	/* cout << i << "\t"; */
    /* } */
    /* cout << endl << endl; */
    /* pair<int, int> iContPos = this->pickCornerCutOff(bLeft); */
    /* cout << iContPos.first << "\t" << iContPos.second << endl; */
    cv::Mat mMammoROITest = mMammoROI.clone();
    /* this->deleteUnneeded(bLeft, mMammoROITest, pEdgeContour, iContPos.second); */
    /*
     *
     *  NEED TO TRANSFER ALL THESE FUNCTIONS TO OOO ONES, AND MAKE IT WORK...
     *
     *
     */

    /*
     *
     *
     *	CHAIN TEST FROM CONTOURS
     *	    NEED TO LOOK FOR CORNERS IN UPPER REGION OF IMAGE. SOME BREASTS BADLY BEHAVED..
     *
     */

    std::vector<cv::Point> pEdgePolyApprox;
    cv::approxPolyDP(this->pEdgeContourCompressed, pEdgePolyApprox, 3, 0);

    std::vector<double> pEdgeAngles; // The "angle" of the line between i and i-1
    for(int i = 1; i < pEdgePolyApprox.size(); i++){
	int x2 = pEdgePolyApprox[i].x;
	int y2 = pEdgePolyApprox[i].y;
	int x1 = pEdgePolyApprox[i-1].x;
	int y1 = pEdgePolyApprox[i-1].y;

	double dAngle = atan2(y2-y1,x2-x1) * 180 / CV_PI;
	pEdgeAngles.push_back(dAngle);
    }
    std::vector<double> pEdgeAngleDeltas; // The change in angle between i and i-1
    for(int i = 1; i < pEdgeAngles.size(); i++){
	pEdgeAngleDeltas.push_back(abs(pEdgeAngles[i]-pEdgeAngles[i-1]));
	/* cout << pEdgeAngleDeltas[i-1] << endl; */
    }
    // pEdgeAngleDeltas[i] = the angle of the corner at pEdgeCont...[i+1]

    for(int i = 1; i < pEdgePolyApprox.size() ; i++){
	double iRadius = pEdgeAngleDeltas[i-1];
	cv::Point pTemp = pEdgePolyApprox[i];
	if( (pTemp.x > 10) && (pTemp.x < mMammo.cols - 10) && (pTemp.y > 10) && (pTemp.y < mMammo.rows - 10)) {
	    cv::circle(mMammoROITest, pTemp, iRadius, 120, -1);
	}
    }
    int iContPosY = mMammo.rows;
    int iContPosX;
    for(int i = 1; i < (int)pEdgePolyApprox.size(); i++){
	if(pEdgeAngleDeltas[i-1] > 10){
	    if(pEdgePolyApprox[i].y > 2*float(mMammo.rows)/3){
		if(bLeft){
		    if(pEdgePolyApprox[i].x < float(0.25*mMammo.cols)){
			iContPosY = std::min(pEdgePolyApprox[i].y,iContPosY);
			iContPosX = pEdgePolyApprox[i].x;
		    }
		} else {
		    if(pEdgePolyApprox[i].x > float(0.75*mMammo.cols)){
			iContPosY = std::min(pEdgePolyApprox[i].y,iContPosY);
			iContPosX = pEdgePolyApprox[i].x;
		    }
		}
	    }
	}
    }
    cv::circle(mMammoROITest, cv::Point(iContPosX,iContPosY), 10, 0, -1);
    this->deleteUnneeded(bLeft, mMammoROITest, pEdgeContour, iContPosY);
    // Need to make "deleteUneeded" work from the polyApprox contour and/or pass it the whole point.
    // Otherwise, this works reasonably well.
    /* cv::imwrite(strFileName + "cornerTest.png", mMammoROITest); */
    this->mMammoROISmaller = mMammoROITest.clone();
    /* cv::imwrite(strFileName + "mammoTest.png", mMammo8BitNorm); */
}

void breast::drawHist(){
    // Draw it
    int histSize = this->iHistSize;
    int hist_w = histSize; int hist_h = 512;
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0,0,0));

    // Normalize the result to [ 0, histImage.rows ].
    cv::normalize(this->mHist, mHist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

    float imMax = 0;
    int bin_w = cvRound(double(hist_w/histSize));
    float iMax = 0;
    for(int i = 1; i < histSize; i++){
	line(histImage, cv::Point(bin_w*(i-1), hist_h - cvRound(mHist.at<float>(i-1))) ,
	    cv::Point(bin_w*(i), hist_h - cvRound(mHist.at<float>(i))),
	    cv::Scalar(255, 255, 255), 2, 8, 0);
	if(imMax < mHist.at<float>(i)){
	    imMax = mHist.at<float>(i);
	    iMax = i;
	}
    }
    /* cv::imwrite(strFileName+"_hist.jpg", histImage ); */
}

pair<float, float> breast::findHistPeak(){
    float iNMax = 0;
    float iBinMax = 0;
    for(int i = this->iHistSize - 1; i > this->iHistSize*0.5; i--){
	if(iNMax < mHist.at<float>(i)){
	    iNMax = mHist.at<float>(i);
	    iBinMax = i;
	}
    }
    for(int i = iBinMax; i > this->iHistSize*0.5; i--){
	if(mHist.at<float>(i) < 1){
	    iBinMax = i;
	}
    }
    return make_pair(iNMax, iBinMax);
}

pair<float, float> breast::findHistPeakLeft(){
    float iNMax = 0;
    float iBinMax = 0;
    for(int i = this->iHistSize*0.5; i > 0; i--){
	if(iNMax < mHist.at<float>(i)){
	    iNMax = mHist.at<float>(i);
	    iBinMax = i;
	}
	}
    return make_pair(iNMax, iBinMax);
}

pair<float, float> breast::findHistPeakRight(){
    float iNMax = 0;
    float iBinMax = 0;
    for(int i = this->iHistSize*0.5; i < this->iHistSize; i++){
	if(iNMax < mHist.at<float>(i)){
	    iNMax = mHist.at<float>(i);
	    iBinMax = i;
	}
	}
    return make_pair(iNMax, iBinMax);
}


float breast::findWidth(const int iBinMax, const int iNMax){
    // Find the width at the value with PEAK_VALUE/OL_WIDTH.
    float iQuartMax = 0;
    for(int i = iBinMax; i > 0; i--){
	if(mHist.at<float>(i) < iNMax/OL_WIDTH){
	    iQuartMax = i;
	    break;
	}
    }
    return iQuartMax;
}



bool breast::leftOrRight(){
	std::vector<cv::Point> pEdgeContourCopy = pEdgeContour;
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
	this->bLeft = iTotalGap < 0;
	return bLeft;
}

std::vector<std::vector<cv::Point>> breast::findCorners(float iDivisor, const int iMax, const int iCOLOUR_MAX){
	cv::Mat mCornerThresh;
	int iCorners;
	do{
		cv::threshold(mCorner,mCornerThresh,iMax/iDivisor,iCOLOUR_MAX,0);
		cv::Mat mCornerT8U;
		mCornerThresh.convertTo(mCornerT8U, CV_8U);
		cv::findContours(mCornerT8U,pContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE); // This seems to yield many, many contours regardless of threshold.
		iCorners = pContours.size();
		/* cout << iCorners << "\t"; */
		iDivisor+=0.05;

	} while ((iCorners < 5) && (iDivisor < 10));
	/* cout << endl; */
        return pContours;
}

std::vector<cv::Point> breast::findCornerCentre(){
	for(auto &i:pContours){
		cv::Moments momCont = cv::moments(i);
		if (momCont.m00 > 0) vecContCents.push_back(cv::Point(momCont.m10/momCont.m00,momCont.m01/momCont.m00));
	}
	return vecContCents;
}

pair<int, int> breast::pickCornerCutOff(const bool bLeft){
	int iContPosY = mMammo.rows;
	int iContPosX;
	for(int i = 0; i < (int)pContours.size(); i++){
		if(vecContCents[i].y > 2*float(mMammo.rows)/3){
			if(bLeft){
				if(vecContCents[i].x < float(0.25*mMammo.cols)){
					iContPosY = std::min(vecContCents[i].y,iContPosY);
					iContPosX = vecContCents[i].x;
				}
			} else {
				if(vecContCents[i].x > float(0.75*mMammo.cols)){
					iContPosY = std::min(vecContCents[i].y,iContPosY);
					iContPosX = vecContCents[i].x;
				}
			}
		}
	}
	return make_pair(iContPosX, iContPosY);
}

void breast::deleteUnneeded(const bool bLeft, cv::Mat mMammoThreshedCopy, const std::vector<cv::Point> pEdgeContourCopy, const int iContPosY){
    if (iContPosY < mMammo.rows){
            int extremalX=pEdgeContourCopy[0].x;
            int lastY=pEdgeContourCopy[0].y;
            //int lastX=pEdgeContourCopy[0].x;
            std::vector<cv::Point> pEdgeThrowAway;
            for(auto i:pEdgeContourCopy){
                if(lastY == i.y){
                        extremalX = bLeft?
                                ((i.x>1)?std::min(extremalX,i.x):extremalX)
                            :
                                ((i.x<mMammo.cols-5)?std::max(extremalX,i.x):extremalX);
                } else {
                    if(lastY >= iContPosY){
                        pEdgeThrowAway.push_back(cv::Point(extremalX,lastY));
                    }
                    extremalX = bLeft?((i.x>1)?i.x:extremalX):((i.x<mMammo.cols-5)?i.x:extremalX);
                }
                lastY = i.y;
            }

            /* cv::cvtColor(mMammoThreshedCopy, mMammoThreshedCopy, cv::COLOR_BGR2GRAY); */
            for(auto i:pEdgeThrowAway){
                if(bLeft){
                    for(int x = 0; x <= i.x+2; x++){
                        mMammoThreshedCopy.at<uchar>(cv::Point(x, i.y)) = 0;
                    }
                } else {
                    for(int x = i.x-2; x < mMammoROI.cols; x++){
                        mMammoThreshedCopy.at<uchar>(i.y, x) = 0;
                    }
                }
            }
    }
}

std::vector<float> breast::getDistBright(){ // Find the average brightness of pixels a certain distance from the breast
    std::vector<int> vecDistAv; // Number of pixels at distance from black.
    cv::Mat_<uchar> mMammoDistChar = mMammoDist; // At one point this was a matrix of ints - don't know why.
    vecDistBright.resize(256);
    vecDistAv.resize(256);
    cv::Mat mMammoCopy = mMammo8BitNorm;
    for(int i = 0; i < (int)vecDistBright.size(); ++i){ vecDistBright[i] = uchar(0);}	// Initialise vectors
    for(int i = 0; i < (int)vecDistAv.size(); ++i){ vecDistAv[i] = 0;}			// ...continued.

    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            int iDist = int(mMammoDistChar(j,i));
            vecDistBright[iDist]+=float(mMammoCopy.at<uchar>(j,i));
            vecDistAv[iDist]++;
        }
    }

    for(int i = 0; i < (int)vecDistBright.size(); ++i){ if(vecDistAv[i] != 0){vecDistBright[i]/= float(vecDistAv[i]);}}
    return vecDistBright;
}

std::vector<float> breast::breastThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy){ // Find the average brightness of the brightest half of pixels at a certain distance from the breast
    std::vector<int> vecDistAvBrightest; // Number of pixels at distance from black.
    this->vecDistBrightBrightest.resize(256);
    vecDistAvBrightest.resize(256);


    // This could well be the problem - need to find a more sensible way of sorting out duff values; maybe not all of vector is populated?
    for(int i = 0; i < (int)vecDistBrightBrightest.size(); ++i){ vecDistBrightBrightest[i] = uchar(0);}
    for(int i = 0; i < (int)vecDistAvBrightest.size(); ++i){ vecDistAvBrightest[i] = 0;}


    // Find the total brightness of the brightest half of pixels at each distance,
    // and draw the fat ROI for the image.
    /* this->mMammoFatROI = cv::Mat((int)mMammo8BitNorm.rows, (int)mMammo8BitNorm.cols, CV_8UC1, cv::Scalar(1)); */
    this->mMammoFatROI = this->mMammoROI.clone();
    for(int i = 0; i < mMammo8Bit.cols; i++){
        for(int j = 0; j < mMammo8BitNorm.rows; j++){
	    if(mMammoROI.at<uchar>(j,i) != 0){
            int iDist = int(mMammoDistChar(j,i));
            if(int(mMammo8BitNorm.at<uchar>(j,i)) >= this->vecDistBright[iDist]){
	    mMammoFatROI.at<uchar>(j,i) = 128;
            vecDistBrightBrightest[iDist]+=float(mMammo8BitNorm.at<uchar>(j,i));
            vecDistAvBrightest[iDist]++;}
	    }
        }
    }
    /* cv::imwrite(this->strFileName+"FatROI.png", mMammoFatROI); */

    // Find the average brightness by dividing the total brightness by the number of pixels.
    for(int i = 0; i < (int)vecDistBrightBrightest.size(); ++i){
	if(vecDistAvBrightest[i] != 0){
	    vecDistBrightBrightest[i]/= float(vecDistAvBrightest[i]);
	}
    }
    return vecDistBrightBrightest;
}

// It would be a very good idea to make this into a more general thing, i.e stop hard coding bin sizes....
void breast::getRadialThickness(){
    cv::normalize(this->mMammoDist, this->mMammoDist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
    vector<float> vecDistBright = this->getDistBright();
    int dist_w = 255*2; int dist_h = 256;
    cv::Mat distImage(dist_h, dist_w, CV_8UC1, cv::Scalar(0));
    cv::Mat_<int> mMammoDistChar = this->mMammoDist;
    cv::Mat mMammoCopy;
    int histSize = 255;
    vector<float> vecDistBrightBrightest = this->breastThickness(histSize, mMammoDistChar, mMammoCopy);
    for(int i = 1; i < vecDistBrightBrightest.size(); i++){
	if(vecDistBrightBrightest[i-1] == 0){
	    vecDistBrightBrightest[i-1] = vecDistBrightBrightest[i];
	}
    }
    for(auto &i:vecDistBrightBrightest){
	i = i*256*256;
	i = log(i/this->dMeanBackgroundValue);
    }
    cv::normalize(vecDistBrightBrightest, vecDistBrightBrightest, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());

    for(auto &i:vecDistBrightBrightest){
	i = 256 - i;
    }
    int bin_w = cvRound(double(dist_w/histSize));
    for(int i = 1; i < histSize; i++){
	    line(distImage, cv::Point(bin_w*(i-1), dist_h - cvRound(vecDistBrightBrightest[i-1])) ,
					    cv::Point(bin_w*(i), dist_h - cvRound(vecDistBrightBrightest[i])),
					    cv::Scalar(255, 255, 255), 1, 8, 0);
    }
    this->mMammoDistImage = distImage;
}

// What the dickens is this? It appears to be unused.
std::vector<float> breast::normalBreastThickness(std::vector<float> vecDistBrightBrightest, const cv::Mat distImage){
    // Normalize the result to [ 0, histImage.rows ].
    vecDistBrightBrightest[255]=0;
    vecDistBrightBrightest[0]=0;
    cv::normalize(vecDistBrightBrightest, vecDistBrightBrightest, 0, distImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
    return vecDistBrightBrightest;
}

void breast::drawImages(string fileName, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize){
    /* fileName.pop_back(); */
    /* int dist_w = histSize*2; */
    /* int dist_h = 512; */
    #ifdef OL_DRAW_DIST
    /* int bin_w = cvRound(double(dist_w/histSize)); */
    /* for(int i = 1; i < histSize; i++){ */
    /*     line(distImage, cv::Point(bin_w*(i), dist_h - cvRound(vecDistBrightBrightest[i])) , */
    /*                     cv::Point(bin_w*(i), dist_h - cvRound(vecDistBrightBrightest[i])), */
    /*                     cv::Scalar(255, 255, 255), 2, 8, 0); */
    /* } */
    /* cv::imwrite(fileName+"_dist.png", this->mMammoDistImage ); */
    #endif

    #ifdef OL_DRAW_CORNER
        /* cv::imwrite(fileName+"_corner.png", mCornerThresh); */
    #endif

    #ifdef OL_DRAW_DISTMAP
        /* cv::imwrite(fileName+"_distmap.png", mMammoDist); */
    #endif

    #ifdef OL_DRAW_THRESH
        /* cv::imwrite(fileName+"_thresh.png", mMammoThreshedCopy); */
    #endif

    #ifdef OL_DRAW_ALTERED
        /* cv::imwrite(fileName+"_alt.png", mMammo); */
    #endif
}

double breast::totalBreast(){
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    int counter(0);
    for(int i = 0; i < mMammoROI.cols; i++){
        for(int j = 0; j < mMammoROI.rows; j++){
	    // This used to be " == 1" - I don't understand why?
            if(mMammoROI.at<Uint8>(j,i) != 0)
               counter++;
        }
    }
    return double(counter*thickness);
}

pair<double,double> breast::glandpercent(const phantomCalibration calib, const string filTar, const string kV, const double t){
    map<string, double> b_a = {{"MoMo", -0.053}, {"MoRh26", -0.041}, {"MoRh27", -0.0466}, {"MoRh28", -0.053}, {"MoRh29", -0.046}, {"RhRh29", -0.047}, {"RhRh30", -0.042}, {"RhRh31", -0.041}};
    map<string, double> b_b = {{"MoMo", 4.402}, {"MoRh26", 4.321}, {"MoRh27", 4.827}, {"MoRh28", 5.173}, {"MoRh29", 5.123}, {"RhRh29", 5.39}, {"RhRh30", 5.313}, {"RhRh31", 5.444}};
    div_t divresult;
    divresult = div(t,10);
    pair<int,int> thick = {divresult.quot*10,divresult.quot*10+10};
    double a = (calib.calibSet.find(thick.first)->second.first+calib.calibSet.find(thick.second)->second.first)/2;
    string lookFor = filTar+kV;
    double a_calc = b_a.find(lookFor)->second;
    double b_calc = b_b.find(lookFor)->second;
    double b = a_calc*t+b_calc;
    pair<double,double> ret = {a,b};
    return ret;
}

void breast::thicknessMap(const pair<double,double> coeff3, const int exposure){
    cv::Mat tg = cv::Mat(mMammo.rows, mMammo.cols, CV_8UC1, cvScalar(0));
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    double tgTemp;
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoROI.at<Uint8>(j,i) != 0){
                if(int(mMammo.at<Uint16>(j,i)) == 0){
                    tgTemp = thickness;
                } else{
                    tgTemp = (log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
                }
                if(tgTemp >= 0 && tgTemp <= thickness){
                    tg.at<Uint8>(j,i) = tgTemp*double(255/thickness);
                } else if(tgTemp > thickness){
                    tg.at<Uint8>(j,i) = thickness*double(255/thickness);
                } else{
                    tg.at<Uint8>(j,i) = 0;
                }
        }
    }
    }
    /* cv::imwrite("test_thickMap.png",tg); */
}

void breast::thicknessMapRedVal(const pair<double,double> coeff3, const int exposure){
    cv::Mat tg = cv::Mat(mMammo.rows, mMammo.cols, CV_8UC1, cvScalar(0));
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    double tgTemp;
    double maxPixValCurve = exp(coeff3.second)*exposure;
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoROI.at<Uint8>(j,i) != 0){
                    if(int(mMammo.at<Uint16>(j,i)) == 0){
                        tgTemp = thickness;
                    } else{
                        tgTemp = (log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
                    }
                    if(tgTemp >= 0 && tgTemp <= thickness){
                        tg.at<Uint8>(j,i) = tgTemp*double(255/thickness);
                    } else if(tgTemp > thickness){
                        tg.at<Uint8>(j,i) = thickness*double(255/thickness);
                    } else{
                        tg.at<Uint8>(j,i) = 0;
                    }
        }
    }
    }
    cv::Mat dst;
    cvtColor(tg,dst,CV_GRAY2RGB);
    cv::Vec3b color;
    color.val[0] = 0;
    color.val[1] = 0;
    color.val[2] = 255;
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoROI.at<Uint8>(j,i) != 0){
                if(int(mMammo.at<Uint16>(j,i)) > maxPixValCurve){
                    dst.at<cv::Vec3b>(cv::Point(i,j)) = color;
                }
            }
        }
    }
    /* cv::imwrite("test_thickMapRed.png",dst); */
}

double breast::getHeight(int x, int y){
    // Radial thickness model:
    // First, we need to find the distance of the pixel from the breast edge.
    int iDistance = this->mMammoDist.at<uchar>(y,x);
    return double(vecDistBrightBrightest[iDistance]);
}

bool breast::isFat(int x, int y){
    return (mMammoFatROI.at<uchar>(y,x) == 128);
}

void breast::makeXinROIMap(){
    cv::Mat HRROIMap = this->mChenFatClass.clone();
    cv::Mat HeightMap(HRROIMap.rows,HRROIMap.cols,CV_32F,cv::Scalar(0));
    for(int i = 0; i < HRROIMap.cols; i++){
	for(int j = 0; j < HRROIMap.rows; j++){
	    if(this->getPixelType(i,j)==XIN_FAT){ // 2 = fat
		/* HRROIMap.at<Uint8>(j,i) = 255; */
		HeightMap.at<float>(j,i) = -1*(log(float(mMammo.at<Uint16>(j,i))/float(this->dMeanBackgroundValue)));

	    }
	}
    }
    double minVal;
    double maxVal;

    // Rescale the image to make it lie between 0 and 255
    cv::minMaxLoc(HeightMap, &minVal, &maxVal);
    for(int i = 1; i < HRROIMap.cols; i++){
	for(int j = 0; j < HRROIMap.rows; j++){
	    if(this->getPixelType(i,j)!=XIN_FAT){ // 2 = fat
		HeightMap.at<float>(j,i)=float(minVal);
	    }
	}
    }
    cv::minMaxLoc(HeightMap, &minVal, &maxVal);
    HeightMap-=minVal;
    HeightMap.convertTo(HeightMap,CV_8U,255.0/(maxVal-minVal));
/* cv::minMaxLoc(HeightMap, &minVal, &maxVal); */
    /* HeightMap = HeightMap*256/maxVal; */

    HRROIMap = mChenFatClass*(256/5); // Convert between our 14 bit mammograms and 256
    cv::imwrite(strFileName+"FatLogTransform.png",HeightMap);

    /* Make a second, three channel, map that distinguishes between background / not background */
    cv::Mat HeightMapRGB;
    /* HeightMap.convertTo(HeightMapRGB, CV_8UC3); */
    cv::cvtColor(HeightMap,HeightMapRGB,CV_GRAY2RGB);
    for(int i = 0; i < HeightMapRGB.cols; i++){
	for(int j = 0; j < HeightMapRGB.rows; j++){
	    HeightMapRGB.at<uchar>(j,i,1) = ((this->getPixelType(i,j) == XIN_BACKGROUND) || (this->getPixelType(i,j) == XIN_FAT))?255:0;
	}
    }
    cv::imwrite(strFileName+"FatLogTransformRGB.png",HeightMapRGB);
}

int breast::getPixelType(int x, int y){
    int iType = this->mChenFatClass.at<Uint8>(y,x,0);
    if (iType == 43) return XIN_BACKGROUND;
    if (iType == 85) return XIN_FAT;
    if (iType == 128) return XIN_PECTORAL_MUSCLE;
    if (iType == 170) return XIN_GLAND;
    if (iType == 213) return XIN_NIPPLE;
    if (iType == 255) return XIN_DENSER_GLAND;
    return 0;
}

bool breast::isBreast(int x, int y){
    return (mMammoROI.at<uchar>(y,x) > 0);
}

void breast::thicknessMapRedValBorder(const pair<double,double> coeff3, const int exposure, const vector<pair<int,int>> contactBorderShapeVal){
    cv::Mat tg = cv::Mat(mMammo.rows, mMammo.cols, CV_8UC1, cvScalar(0));
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    double tgTemp;
    /* double maxPixValCurve = exp(coeff3.second)*exposure; */
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoROI.at<Uint8>(j,i) != 0){
                    if(int(mMammo.at<Uint16>(j,i)) == 0){
                        tgTemp = thickness;
                    } else{
                        tgTemp = (log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
                    }
                    if(tgTemp >= 0 && tgTemp <= thickness){
                        tg.at<Uint8>(j,i) = tgTemp*double(255/thickness);
                    } else if(tgTemp > thickness){
                        tg.at<Uint8>(j,i) = thickness*double(255/thickness);
                    } else{
                        tg.at<Uint8>(j,i) = 0;
                    }
        }
    }
    }
    cv::Mat dst;
    cvtColor(tg,dst,CV_GRAY2RGB);
    cv::Vec3b color;
    color.val[0] = 0;
    color.val[1] = 0;
    color.val[2] = 255;
        for(int j = 0; j < 1075; j++){
            dst.at<cv::Vec3b>(cv::Point(j,contactBorderShapeVal[j].second)) = color;
        }
    /* cv::imwrite("test_thickMapRedBorder.png",dst); */
}

#ifdef KSENIA_STUFF
vector<pair<int,int>> breast::pixelOfInterestExposure(){
    vector<pair<int,int>> pixelOfInterestExposureVec;
    pair<float, float> rightPeak = this->findHistPeakRight();
    pair<float, float> leftPeak = this->findHistPeakLeft();
    float minVal = leftPeak.first;
    int rightLim;
    for(int  i = rightPeak.second; i > leftPeak.second*3.5; i--){
        if(this->mHist.at<float>(i) < minVal)
            minVal = this->mHist.at<float>(i);
    }
    for(int  i = rightPeak.second; i > leftPeak.second*3.5; i--){
        if(this->mHist.at<float>(i) == minVal){
            rightLim = i;
            break;
        }
    }
    int MPVRangeUpperLimit = (this->LargestImagePixelValue/512)*(rightLim);
    int MPVRangeLowerLimit = (this->LargestImagePixelValue/512)*(leftPeak.second*3.5);
    for(int i = 0; i < this->mMammo.cols; i++){
         for(int j = 0; j < this->mMammo.rows; j++){
            if(this->mMammo.at<Uint16>(j,i) < MPVRangeUpperLimit && this->mMammo.at<Uint16>(j,i) > MPVRangeLowerLimit)
                pixelOfInterestExposureVec.push_back(make_pair(j,i));
         }
    }
    this->drawHist();
    return pixelOfInterestExposureVec;
}

map<int,vector<pair<double,pair<int,int>>>> breast::distMap(vector<pair<int,int>> pixelOfInterestExposureVec){
    map<int,vector<pair<double,pair<int,int>>>> breastDistMap;
    int distVal, vecLength, leftBorder, rightBorder, difference;
    double MPVsum, countPix;
    for(vector<pair<int,int>>::iterator it = pixelOfInterestExposureVec.begin() ; it != pixelOfInterestExposureVec.end(); ++it){
        distVal = this->mMammoDist.at<Uint16>(it->first,it->second);
        if(!breastDistMap.count(distVal)){
            vector<pair<double,pair<int,int>>> breastDistMapVec;
            breastDistMap[distVal] = breastDistMapVec;
        }
        breastDistMap[distVal].push_back(make_pair(0,make_pair(it->first,it->second)));
    }
    for(map<int,vector<pair<double,pair<int,int>>>>::iterator it = breastDistMap.begin() ; it != breastDistMap.end(); ++it){
        vecLength = it->second.size();
        for(int i = 0; i < vecLength; i++){
            MPVsum = 0; countPix = 0;
            difference = i-90;
            if(difference < 0){
                leftBorder = 0;
            } else{
                leftBorder = i - 90;
            }
            difference = i+90;
            if(difference > vecLength){
                rightBorder = vecLength;
            } else{
                rightBorder = i + 90;
            }
            for(int j = leftBorder; j < rightBorder; j++){
                MPVsum += this->mMammo.at<Uint16>(it->second[j].second.first,it->second[j].second.second);
                countPix++;
            }
            it->second[i].first = MPVsum/countPix;
        }
    }
    vector<pair<double,pair<int,int>>> breastDistMapVec2;
    for(int i = 0; i < this->mMammoDist.cols; i++){
        for(int j = 0; j < this->mMammoDist.rows; j++){
            distVal = this->mMammoDist.at<Uint16>(j,i);
            if(distVal == breastDistMap.rbegin()->first+1)
                breastDistMapVec2.push_back(make_pair(0,make_pair(j,i)));
        }
    }
    vecLength = breastDistMapVec2.size();
    for(int i = 0; i < vecLength; i++){
        MPVsum = 0; countPix = 0;
        difference = i-90;
        if(difference < 0){
            leftBorder = 0; rightBorder = i + 90 - difference;
        } else{
            leftBorder = i - 90; rightBorder = i + 90;
        }
        for(int j = leftBorder; j < rightBorder; j++){
            MPVsum += this->mMammo.at<Uint16>(breastDistMapVec2[i].second.first,breastDistMapVec2[i].second.second);
            countPix++;
        }
        breastDistMapVec2[i].first = MPVsum/countPix;
    }
    breastDistMap[distVal] = breastDistMapVec2;
    return breastDistMap;
}

void breast::applyExposureCorrestion(map<int,vector<pair<double,pair<int,int>>>> breastDistMap){
    double distAvNext;
    int distVal, vecLength, pixVal;
    for(map<int,vector<pair<double,pair<int,int>>>>::iterator it = breastDistMap.begin() ; it != breastDistMap.end(); ++it){
        vecLength = it->second.size();
        for(int j =  0; j < vecLength; j++){
            pixVal = this->mMammo.at<Uint16>(it->second[j].second.first, it->second[j].second.second);
            this->mMammo.at<Uint16>(it->second[j].second.first, it->second[j].second.second) =  pixVal*(it->second[j-1].first/it->second[j].first);
        }
    }
}
#endif

void breast::exposureMap(const pair<double,double> coeff3, const int exposure, const vector<pair<int,int>> pixelOfInterestExposureVec){
    cv::Mat tg = cv::Mat(mMammo.rows, mMammo.cols, CV_8UC1, cvScalar(0));
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    double tgTemp;
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoROI.at<Uint8>(j,i) != 0){
                    if(int(mMammo.at<Uint16>(j,i)) == 0){
                        tgTemp = thickness;
                    } else{
                        tgTemp = (log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
                    }
                    if(tgTemp >= 0 && tgTemp <= thickness){
                        tg.at<Uint8>(j,i) = tgTemp*double(255/thickness);
                    } else if(tgTemp > thickness){
                        tg.at<Uint8>(j,i) = thickness*double(255/thickness);
                    } else{
                        tg.at<Uint8>(j,i) = 0;
                    }
        }
    }
    }
    cv::Mat dst;
    cvtColor(tg,dst,CV_GRAY2RGB);
    cv::Vec3b color;
    color.val[0] = 0;
    color.val[1] = 0;
    color.val[2] = 255;
    int vecLength  = pixelOfInterestExposureVec.size();
        for(int j = 0; j < vecLength; j++){
            dst.at<cv::Vec3b>(cv::Point(pixelOfInterestExposureVec[j].second,pixelOfInterestExposureVec[j].first)) = color;
        }
    cv::imwrite("test_exposureMap.png",dst);
}
