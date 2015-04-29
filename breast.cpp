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
#define _USE_MATH_DEFINES

breast::breast(std::string t_strFileName): mammography(t_strFileName){
    this->strFileName = breast::fileNameErase(t_strFileName);
    this->pixelVec2Mat();
    this->mChenFatClass = cv::imread(this->strFileName+"Label.tiff");
    cv::resize(mChenFatClass,mChenFatClass,this->mMammo.size());
    cv::minMaxLoc(mMammo,&this->dMinPixelValue,&this->dMaxPixelValue);
    this->getBreastROI();
    this->getBreastDistMap();
    this->getBreastEdge();
    // NEED TO REPLACE THIS WITH LEFT/RIGHT FROM DCM HEADER
    // Then just flip all images to be left.
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
    if(this->ImageLaterality == 'R') cv::flip(mMammo,mMammo,1); // If right breast, flip. 
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
    cv::imwrite(strFileName + "cornerTest.png", mMammoROITest);
    this->mMammoROISmaller = mMammoROITest.clone();
    cv::imwrite(strFileName + "mammoTest.png", mMammo8BitNorm);
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
	/* std::vector<cv::Point> pEdgeContourCopy = pEdgeContour; */
	sort(pEdgeContour.begin(),pEdgeContour.end(),[](const cv::Point &l, const cv::Point &r){return l.x < r.x;});
	/* sort(pEdgeContourCopy.begin(),pEdgeContourCopy.end(),[](const cv::Point &l, const cv::Point &r){return l.y < r.y;}); */
	/* int iXLast = -1; */
	/* int iYLast = -1; */
	/* int i2LastGap = 0; */
	/* int iCurrGap = 0; */
	/* int iTotalGap = 0; */
	/* for(auto i:pEdgeContour){ */
	/* 	if (i.x == iXLast){ */
	/* 		iCurrGap = abs(iYLast-i.y); */
	/* 		if(iCurrGap > 0.1*mMammo.rows){ */
	/* 			if(i2LastGap != 0){ */
	/* 				iTotalGap += i2LastGap - iCurrGap; */
	/* 			} */
	/* 		} */
	/* 	} else { */
	/* 		iXLast = i.x; */
	/* 		iYLast = i.y; */
	/* 		i2LastGap = iCurrGap; */
	/* 	} */
	/* } */
	/* this->bLeft = iTotalGap < 0; */
	/* this->bLeft = (this->ImageLaterality == 'L'); */
	this->bLeft = true;
	this->bLeftNEW = (this->ImageLaterality == 'L');
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
         //cv::imwrite(fileName+"_thresh.png", mMammoThreshedCopy);
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

Uint16 breast::getHeight(int x, int y){
    // Equivalent-log-fat model
    return this->mHeightMap16.at<Uint16>(y,x);
}

bool breast::isFat(int x, int y){
    return (mMammoFatROI.at<uchar>(y,x) == 128);
}

void breast::makeXinROIMap(){
    cv::Mat HRROIMap = this->mChenFatClass.clone();
    cv::Mat HeightMap(HRROIMap.rows,HRROIMap.cols,CV_32F,cv::Scalar(0));
    for(int i = 0; i < HRROIMap.cols; i++){
	for(int j = 0; j < HRROIMap.rows; j++){
	    HeightMap.at<float>(j,i) = -1*(log(float(mMammo.at<Uint16>(j,i))/float(this->dMeanBackgroundValue)));
	}
    }
    double minVal;
    double maxVal;

    // Rescale the image to make it lie between 0 and 255
    cv::minMaxLoc(HeightMap, &minVal, &maxVal);
    HeightMap-=minVal;
    HeightMap.convertTo(HeightMap,CV_8U,255.0/(maxVal-minVal));
/* cv::minMaxLoc(HeightMap, &minVal, &maxVal); */
    /* HeightMap = HeightMap*256/maxVal; */

    /* HRROIMap = mChenFatClass*(256/5); // Convert between our 14 bit mammograms and 256 */
    cv::imwrite(strFileName+"FatLogTransform.png",HeightMap);

    // FILLING IN HOLES IN FAT MAP
    // Morphological opening: remove noise, fill in small holes
    cv::Mat mCircSE = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(25,25));
    cv::Mat HeightMapFilled = HeightMap.clone();
    cv::morphologyEx(HeightMap,HeightMapFilled,cv::MORPH_OPEN,mCircSE);

    for(int i = 0; i < HeightMap.cols; i++){
	for(int j = 0; j < HeightMap.rows; j++){
	    if(this->getPixelType(i,j)!=XIN_FAT){ // 2 = fat
		HeightMapFilled.at<uchar>(j,i) = 0;

	    }
	}
    }

    /* cv::GaussianBlur(HeightMapFilled,HeightMapFilled,cv::Size(5,5),10); */
    /* cv::imwrite(strFileName+"FatLogFilled.png",HeightMapFilled); */

    // Next step: identify remaining holes and fill them in, one by one.
    // Proposed Procedure:
    // For each "zero height" glandular pixel:
    // Take weighted average of neighbouring true non-zero pixels lying on distance transform.
    // Can find true non-zero via making copy of image and writing to original
    //
    cv::Mat HeightMapCopy = HeightMapFilled.clone();
    /* mCircSE = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(25,25)); */
    /* cv::morphologyEx(HeightMapFilled,HeightMapFilled,cv::MORPH_CLOSE,mCircSE); */
    /* cv::medianBlur(HeightMapFilled,HeightMapFilled,25); */
    /* cv::imwrite(strFileName+"FatLogClosed.png",HeightMapFilled); */
    /* this->mHeightMap = HeightMapFilled; */

    /* Next step: Distance transform stuff */
    /* Want to create a vector of points at each distance i.e. vecDist[100] is all the points 100 away */


    /*
     *
     *	    AVERAGING PARALLEL TO THE BREAST EDGE
     *
     *
     */

    cv::Mat_<uchar> mMammoDistChar = mMammoDist.clone(); // At one point this was a matrix of ints - don't know why.
    vector<vector<cv::Point>> pDistContours;
    cv::Mat_<uchar> mMammoDistThresh;
    for(int k = 0; k < 256; k++){
	cv::threshold(mMammoDistChar, mMammoDistThresh, k, 255, 1);
	cv::findContours(mMammoDistThresh.clone(),pDistContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);
    for(auto &vPatD:pDistContours){
	bool bEmpty;
	uchar lastFilled = 0;
	for(auto &p:vPatD){
	    int pType = this->getPixelType(p.x,p.y);
	    if((pType != XIN_BACKGROUND) && (pType != XIN_PECTORAL_MUSCLE)){
		bEmpty = (HeightMapCopy.at<uchar>(p) == 0);
		if(bEmpty){
			HeightMapFilled.at<uchar>(p) = lastFilled;
		} else {
			/* lastFilled = HeightMapCopy.at<uchar>(p); */
		    lastFilled = breast::getAvNhood8(HeightMapCopy, p, 2);
		}
	    }
	}
	lastFilled = 0;
	for(auto it = vPatD.rbegin(); it != vPatD.rend(); it++){
	    auto p = *it;
	    bEmpty = (HeightMapCopy.at<uchar>(p) == 0);
	    int pType = this->getPixelType(p.x,p.y);
	    if((pType != XIN_BACKGROUND) && (pType != XIN_PECTORAL_MUSCLE)){
		if(bEmpty){
		    uchar cCurrent = HeightMapFilled.at<uchar>(p);
		    if(cCurrent != 255){
			HeightMapFilled.at<uchar>(p) = (lastFilled+HeightMapFilled.at<uchar>(p))/2;
		    } else {
			HeightMapFilled.at<uchar>(p) = lastFilled;
		    }
		} else {
		    lastFilled = breast::getAvNhood8(HeightMapCopy, p, 2);
		}
	    }
	}
	/* cout << float(100*k/255) << "%\r" << flush; */
    }
    }
    HeightMapCopy = HeightMapFilled.clone();

    /* Linear interpolation along x-axis */

    for(int j = 0; j < HeightMap.rows; j++){
	uchar lastFilled = 0;
	float distFromHole = 0;
	for(int i = 0; i < HeightMap.cols; i++){
	    int pType = this->getPixelType(i,j);
	    /* if(pType == XIN_DENSER_GLAND){ */
	    if((pType != XIN_BACKGROUND) && (pType != XIN_PECTORAL_MUSCLE)){
		uchar cCurrent = HeightMapFilled.at<uchar>(j,i);
		if(cCurrent == 0){
		    distFromHole++;
		} else {
		    uchar thisSide = cCurrent;
		    for(int ii = distFromHole; ii > 0; ii--){
			HeightMapFilled.at<uchar>(j,i-ii) = uchar(float(lastFilled*(ii/distFromHole)+(1-(ii/distFromHole))*thisSide));
		    }
		    lastFilled = cCurrent;
		    distFromHole = 0;
		}
	    }
	}
    }

    cv::Mat mCircSE2 = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(2,2));
    cv::morphologyEx(HeightMapFilled,HeightMapFilled,cv::MORPH_CLOSE,mCircSE2);
    cv::medianBlur(HeightMapFilled,HeightMapFilled,25);
    cv::medianBlur(HeightMapFilled,HeightMapFilled,25);
    cv::medianBlur(HeightMapFilled,HeightMapFilled,25);
    cv::imwrite(strFileName+"FatLogFilled.png",HeightMapFilled);
    this->mHeightMap = HeightMapFilled.clone();

    /* Undoing all the transformations */
    HeightMapFilled.convertTo(HeightMapFilled,CV_16U);
    for(int i = 0; i < HeightMap.cols; i++){
	for(int j = 0; j < HeightMap.rows; j++){
	    float fCurrentShade = float(HeightMapFilled.at<Uint16>(j,i));
	    fCurrentShade = fCurrentShade*(float(maxVal-minVal)/255.0) + minVal;
	    fCurrentShade = exp(-fCurrentShade)*this->dMeanBackgroundValue;
	    HeightMapFilled.at<Uint16>(j,i) = Uint16(fCurrentShade);
	}
    }
    this->mHeightMap16 = HeightMapFilled.clone();
}

uchar breast::getAvNhood8(cv::Mat &mat, cv::Point &p, int nhood){
    float av=0;
    int n = 0;
    for(int i = 0; i < nhood; i++){
	for(int j = 0; j < nhood; j++){
	    float cC = float(mat.at<uchar>(p.y-1+j,p.x-1+i));
	    if(cC > 0.5){
		n++;
		av+=cC;
	    }
	}
    }
    av = av / n;
    return uchar(av);
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

void breast::thicknessMapRedValBorder(const pair<double,double> coeff3, const int exposure, const vector<cv::Point> contactBorderShapeVal){
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
    int vecLength = contactBorderShapeVal.size();
    cv::Mat dst;
    cvtColor(tg,dst,CV_GRAY2RGB);
    cv::Vec3b color;
    color.val[0] = 0;
    color.val[1] = 0;
    color.val[2] = 255;
        for(int j = 0; j < vecLength; j++){
            dst.at<cv::Vec3b>(contactBorderShapeVal[j]) = color;
        }
    cv::imwrite("test_thickMapRedBorder.png",dst);
}

vector<pair<int,int>> breast::pixelOfInterestExposure(){
    vector<pair<int,int>> pixelOfInterestExposureVec;
    pair<float, float> rightPeak = this->findHistPeakRight();
    pair<float, float> leftPeak = this->findHistPeakLeft();
    float minVal = leftPeak.first;
    int rightLim = 0;
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
    //this->drawHist();
    ofstream myfile;
    myfile.open ("pointsChosen.txt");
    for(vector<pair<int,int>>::iterator it = pixelOfInterestExposureVec.begin(); it != pixelOfInterestExposureVec.end(); it++)
        myfile << it->first << " " << it->second << " " << this->mMammo.at<Uint16>(it->first,it->second) << "\n";
    myfile.close();
    return pixelOfInterestExposureVec;
}

map<int,vector<pair<double,pair<int,int>>>> breast::distMap(vector<pair<int,int>> pixelOfInterestExposureVec){
    map<int,vector<pair<double,pair<int,int>>>> breastDistMap;
    int distVal, vecLength, leftBorder, rightBorder, difference;
    double MPVsum, countPix;
    for(vector<pair<int,int>>::iterator it = pixelOfInterestExposureVec.begin() ; it != pixelOfInterestExposureVec.end(); ++it){
        distVal = float(this->mMammoDist.at<uchar>(it->first,it->second));
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
            difference = i-100;
            if(difference < 0){
                leftBorder = 0;
            } else{
                leftBorder = i - 100;
            }
            difference = i+100;
            if(difference > vecLength){
                rightBorder = vecLength;
            } else{
                rightBorder = i + 100;
            }
            for(int j = leftBorder; j < rightBorder; j++){
                MPVsum += this->mMammo.at<Uint16>(it->second[j].second.first,it->second[j].second.second);
                countPix++;
            }
            it->second[i].first = MPVsum/countPix;
        }
    }
//    vector<pair<double,pair<int,int>>> breastDistMapVec2;
//    for(int i = 0; i < this->mMammoDist.cols; i++){
//        for(int j = 0; j < this->mMammoDist.rows; j++){
//            distVal = float(this->mMammoDist.at<uchar>(j,i));
//            if(distVal == breastDistMap.rbegin()->first+1 && distVal != 256)
//                breastDistMapVec2.push_back(make_pair(0,make_pair(j,i)));
//        }
//    }
//    distVal = breastDistMap.rbegin()->first+1;
//    vecLength = breastDistMapVec2.size();
//    for(int i = 0; i < vecLength; i++){
//        MPVsum = 0; countPix = 0;
//            difference = i-1000;
//            if(difference < 0){
//                leftBorder = 0;
//            } else{
//                leftBorder = i - 1000;
//            }
//            difference = i+1000;
//            if(difference > vecLength){
//                rightBorder = vecLength;
//            } else{
//                rightBorder = i + 1000;
//            }
//        for(int j = leftBorder; j < rightBorder; j++){
//            MPVsum += this->mMammo.at<Uint16>(breastDistMapVec2[i].second.first,breastDistMapVec2[i].second.second);
//            countPix++;
//        }
//        breastDistMapVec2[i].first = MPVsum/countPix;
//    }
//    breastDistMap[distVal] = breastDistMapVec2;
    return breastDistMap;
}

void breast::applyExposureCorrection(map<int,vector<pair<double,pair<int,int>>>> breastDistMap){
    /* double distAvNext; */
    int /*distVal,*/ vecLength1, vecLength2, pixVal;
    map<int,vector<pair<double,pair<int,int>>>>::iterator it;
    vector<pair<pair<int,int>,double>> correctedMPVs;
    it = breastDistMap.begin();
    int minKey = it->first;
    it = breastDistMap.end();
    int maxKey = it->first;
    for(int i = (maxKey-1); i >= minKey; i--){
        if(i != 255){
        vecLength1 = breastDistMap[i].size();
        vecLength2 = breastDistMap[i+1].size();
        for(int j =  0; j < vecLength1; j++){
            if(vecLength2 != 0){
                pixVal = this->mMammo.at<Uint16>(breastDistMap[i][j].second.first, breastDistMap[i][j].second.second);
                if(j < vecLength2){
                    if(breastDistMap[i+1][j].first != 0)
                        this->mMammo.at<Uint16>(breastDistMap[i][j].second.first, breastDistMap[i][j].second.second) =  pixVal*(breastDistMap[i+1][j].first/breastDistMap[i][j].first);
                } else{
                    if(breastDistMap[i+1][vecLength2].first != 0)
                        this->mMammo.at<Uint16>(breastDistMap[i][j].second.first, breastDistMap[i][j].second.second) =  pixVal*(breastDistMap[i+1][vecLength2].first/breastDistMap[i][vecLength2].first);
                }
            }
        }
        }
    }
    ofstream myfile;
    myfile.open ("pointsChosen2.txt");
    for(map<int,vector<pair<double,pair<int,int>>>>::iterator it = breastDistMap.begin(); it != breastDistMap.end(); it++){
        for(vector<pair<double,pair<int,int>>>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++){
            myfile << it2->second.first << " " << it2->second.second << " " << this->mMammo.at<Uint16>(it2->second.first,it2->second.second) << "\n";
        }
    }
    myfile.close();
}


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

string breast::heightRowArray(vector<cv::Point> contactBorder2Vec, const string xOrY){
    string RowArray = "[";
    int counter(0);
    for(vector<cv::Point>::iterator it = contactBorder2Vec.begin(); it != contactBorder2Vec.end(); it++){
        if(this->mHeightMap.at<float>(it->y,it->x) != 0){
            if(xOrY == "x"){
                RowArray.append(to_string(it->x));
            } else{
                RowArray.append(to_string(double(this->mHeightMap.at<uchar>(it->y,it->x))));
            }
            RowArray.append(",");
            counter++;
        }
    }
    RowArray = RowArray.substr(0, RowArray.size()-1);
    if(RowArray != "")
        RowArray.append("]");
    if(counter >= 3){
        return RowArray;
    } else{
        return "";
    }
}

pair<cv::Point,float> breast::straightLevel(const int row){
   pair<cv::Point,float> straightLevelPoint;
   int counter(0);
        for(int i = 0; i < this->mHeightMap.cols; i++){
            if(this->getPixelType(i,row) == XIN_FAT){
                if(double(this->mHeightMap.at<uchar>(row,i)) != 0){
                    counter++;
                    straightLevelPoint = make_pair(cv::Point(i,row),float(this->mHeightMap.at<uchar>(row,i)));
                    if(counter >1 )
                        break;
                }
            }
        }
    return straightLevelPoint;
}

vector<cv::Point> breast::breastBorder(){
    vector<cv::Point> breastBorder;
    for(int j = 0; j < this->mMammo.rows; j++){
        for(int i = 0; i < this->mMammo.cols; i++){
            if(!this->isBreast(i,j)){
                breastBorder.push_back(cv::Point(i,j));
                break;
            }
        }
    }
    return breastBorder;
}

double breast::averageDiffBorder(const vector<cv::Point> contactBorder, const vector<cv::Point> breastBorder){
    cv::Mat_<uchar> mMammoDistChar = this->mMammoDist;
    double sum(0), dist;
    int countVal(0);
    for(int i = 0; i < 501; i++){
        dist = double(mMammoDistChar(contactBorder[i].y,contactBorder[i].x))-double(mMammoDistChar(breastBorder[i].y,breastBorder[i].x));
        sum += dist;
        countVal++;
    }
    return sum/double(countVal)+20;
}

vector<cv::Point> breast::contactBorder(){
    vector<cv::Point> contactBorderPoints;
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    double thickness = atoi(bodyThickness.c_str());
    float straightLevelConst;
    double deltaY;
    //double coeff[4];
    //alglib::real_1d_array x;
    //alglib::real_1d_array y;
    //alglib::spline1dinterpolant s;
    //string RowArrayX, RowArrayY;
    int counter(0);
    for(int i = 0; i < this->mHeightMap.rows; i++){
        //RowArrayX = this->heightRowArray(i, "x");
        //RowArrayY = this->heightRowArray(i, "y");
        //if(RowArrayX != "" && RowArrayX != ""){
            //x =  alglib::real_1d_array(RowArrayX.c_str());
            //y =  alglib::real_1d_array(RowArrayY.c_str());
            //spline1dbuildcubic(x, y, s);
            straightLevelConst = this->straightLevel(i).second*thickness/255;
                for(int j = 0; j < this->mHeightMap.cols; j++){
                    if(this->getPixelType(j,i) == XIN_FAT){
                    if(float(this->mHeightMap.at<uchar>(i,j)) != 0){
                    counter++;
                    //deltaY = (straightLevelConst - spline1dcalc(s, j))/10.0;
                    deltaY = (straightLevelConst - float(this->mHeightMap.at<uchar>(i,j))*thickness/255)/10.0;
                    if(deltaY > 0.5  && counter > 1){
                        contactBorderPoints.push_back(cv::Point(j,i));
                        break;
                    }
                    }
                    }
                }
            counter = 0;
    }
    return contactBorderPoints;
}

vector<cv::Point> breast::contactBorder2(const vector<float> plane, vector<cv::Point> contactBorderFin, const double averageDiffVal){
    vector<cv::Point> contactBorderPoints;
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    double thickness = atoi(bodyThickness.c_str());
    double numerator, denominator, temp;
    double distance(0);
    int counter(0);
    for(vector<cv::Point>::iterator it = contactBorderFin.begin(); it != contactBorderFin.end(); it++){
        for(int  i = 0; i < mHeightMap.cols; i++){
            if(this->getPixelType(i,it->y) == XIN_FAT){
                counter++;
                if(it->y < mHeightMap.cols/2 && (counter > 45) ){
                    if(this->mMammoDist.at<float>(it->y,i) > (averageDiffVal+45)){
                        numerator = plane[0]*it->y+plane[1]*i+plane[2]*float(this->mHeightMap.at<uchar>(it->y,i))*thickness/255+plane[3];
                        denominator = pow(pow(plane[0],2)+pow(plane[1],2)+pow(plane[2],2),0.5);
                        temp = numerator/denominator;
                        if(temp < 0)
                            temp = -temp;
                        if(temp > distance)
                            distance = temp;
                    }
                }else if(it->y > mHeightMap.cols/2 && (counter > 55) ){
                    if(this->mMammoDist.at<float>(it->y,i) > (averageDiffVal+55)){
                        numerator = plane[0]*it->y+plane[1]*i+plane[2]*float(this->mHeightMap.at<uchar>(it->y,i))*thickness/255+plane[3];
                        denominator = pow(pow(plane[0],2)+pow(plane[1],2)+pow(plane[2],2),0.5);
                        temp = numerator/denominator;
                        if(temp < 0)
                            temp = -temp;
                        if(temp > distance)
                            distance = temp;
                    }
                }
            }
        }
        for(int  i = 0; i < mHeightMap.cols; i++){
            if(this->getPixelType(i,it->y) == XIN_FAT){
                numerator = plane[0]*it->y+plane[1]*i+plane[2]*float(this->mHeightMap.at<uchar>(it->y,i))*thickness/255+plane[3];
                denominator = pow(pow(plane[0],2)+pow(plane[1],2)+pow(plane[2],2),0.5);
                temp = numerator/denominator;
                if(temp < 0){
                    temp = -temp;
                    if(temp > distance){
                        contactBorderPoints.push_back(cv::Point(it->y,i));
                        break;
                    }
                }
            }
        }
        counter = 0; distance = 0;
    }
    return contactBorderPoints;
}

vector<cv::Point> breast::contactBorderFinal(const vector<cv::Point> contactBorderVal, const double averageDiffBorderVal){
    cv::Mat_<uchar> mMammoDistChar = this->mMammoDist;
    vector<cv::Point> contactBorderFin;
    double borderDiff;
    int vecLength = contactBorderVal.size();
    for(int i = 0; i < vecLength; i++){
        borderDiff = double(mMammoDistChar(contactBorderVal[i].y,contactBorderVal[i].x));
        if(contactBorderVal[i].x != 1 && borderDiff < averageDiffBorderVal)
            contactBorderFin.push_back(contactBorderVal[i]);
    }
    return contactBorderFin;
}

vector<cv::Point> breast::pointsFatForPlane(vector<cv::Point> contactBorderFin, const double averageDiffVal){
    vector<cv::Point> pointFatDiscarded;
    int counter(0);
    for(vector<cv::Point>::iterator it = contactBorderFin.begin(); it != contactBorderFin.end(); it++){
        for(int  i = 1; i < mHeightMap.cols; i++){
            if(this->getPixelType(i,it->y) == XIN_FAT){
                counter++;
                if(it->y < mHeightMap.cols/2 && (counter > 45) ){
                    if(this->mMammoDist.at<float>(it->y,i) > (averageDiffVal+45)){
                        pointFatDiscarded.push_back(cv::Point(it->y,i));
                    }
                }else if(it->y > mHeightMap.cols/2 && (counter > 45) ){
                    if(this->mMammoDist.at<float>(it->y,i) > (averageDiffVal+55)){
                        pointFatDiscarded.push_back(cv::Point(it->y,i));
                    }
                }
            }
        }
        counter = 0;
    }
    return pointFatDiscarded;
}

vector<cv::Point> breast::getContact(){
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    /* double thickness = atoi(bodyThickness.c_str()); */
    vector<cv::Point> contactBorderPoints =  this->contactBorder();
    vector<cv::Point> breastBorderPoints  = this->breastBorder();
    double averageDiffVal = this->averageDiffBorder(contactBorderPoints, breastBorderPoints);
    vector<cv::Point> contactBorderFin = this->contactBorderFinal(contactBorderPoints, averageDiffVal);
    vector<cv::Point> breastFatDiscarded = this->pointsFatForPlane(contactBorderFin, averageDiffVal);
    vector<float> m = this->fitPlane(breastFatDiscarded);
    //cout << "m: " << m[0] << " " << m[1] << " " << m[2] << " " << m[3] << endl;
    vector<cv::Point> contactBorder2Vec = this->contactBorder2(m, contactBorderFin, averageDiffVal);
    return contactBorderFin;
}

vector<float> breast::fitPlane(vector<cv::Point> breastFatDiscarded){
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    double thickness = atoi(bodyThickness.c_str());
    int vecLength = breastFatDiscarded.size();
    vector<float> temp, resultPlane;
    //ofstream myfile;
    //myfile.open ("pointsChosen.txt");
    for(int  i = 0; i < vecLength; i++){
        if(float(this->mHeightMap.at<uchar>(breastFatDiscarded[i].y,breastFatDiscarded[i].x)) != 0){
        temp.push_back(float(breastFatDiscarded[i].x));
        temp.push_back(float(breastFatDiscarded[i].y));
        temp.push_back(float(this->mHeightMap.at<uchar>(breastFatDiscarded[i].x,breastFatDiscarded[i].y)*thickness/255));
        //myfile << float(breastFatDiscarded[i].x) << " " << float(breastFatDiscarded[i].y) << " " << float(this->mHeightMap.at<uchar>(breastFatDiscarded[i].y,breastFatDiscarded[i].x))*thickness/255 << "\n";
        }
    }
    REAL points[temp.size()];
    for(int  i = 0; i < temp.size(); i++){
        points[i] = temp[i];
    }
    REAL plane[4];
    //myfile.close();
    unsigned int PCOUNT = sizeof(points)/(sizeof(REAL)*3);
    bf_computeBestFitPlane(PCOUNT,points,sizeof(REAL)*3,NULL,0,plane);
    if(plane[0] < 0)
        plane[0] = -plane[0];
    if(plane[1] > 0)
        plane[1] = -plane[1];
    if(plane[2] < 0)
        plane[2] = -plane[2];
    if(plane[3] > 0)
        plane[3] = -plane[3];
    for(int i = 0; i < 4; i++)
        resultPlane.push_back(plane[i]);
    //printf("Best Fit plane: %0.9f,%0.9f,%0.9f,%0.9f\r\n", plane[0], plane[1], plane[2], plane[3] );
    //cout << "Plane angle: " << breast::getPlaneAngle(plane) << endl;
    //cout << breast::getPlaneAngle(resultPlane) << endl;
    //cout << "Average Distance to plane: " << this->averageDistance(plane,breastFatDiscarded) << endl;
    //cout << this->averageDistance(plane,breastFatDiscarded) << endl;
    return resultPlane;
}

void  breast::fatRow(const int row){
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    double thickness = atoi(bodyThickness.c_str());
    ofstream myfile;
    myfile.open ("fatRow.txt");
    for(int i = 0; i < this->mHeightMap.cols; i++){
        if(this->getPixelType(i,row) == XIN_FAT){
            if(float(this->mHeightMap.at<uchar>(row,i)) != 0)
                myfile << i << " " << float(this->mHeightMap.at<uchar>(row,i))*thickness/255 << "\n";
        }
    }
    myfile.close();
}

double breast::getPlaneAngle(const vector<float> plane){
    double arg = plane[2]/pow(pow(plane[0],2)+pow(plane[1],2)+pow(plane[2],2),0.5);
    return acos(arg)*180/M_PI;
}

double breast::averageDistance(const vector<float> plane, vector<cv::Point> breastFatDiscarded){
    string bodyThickness = various::ToString<OFString>(this->BodyPartThickness);
    double thickness = atoi(bodyThickness.c_str());
    double avTemp(0);
    int counter(0);
    int vecLength = breastFatDiscarded.size();
    double numerator, denominator;
    vector<cv::Point> breastPointsUsed;
    for(int  i = 0; i < vecLength; i++){
        if(float(this->mHeightMap.at<uchar>(breastFatDiscarded[i].y,breastFatDiscarded[i].x)) != 0){
        breastPointsUsed.push_back(cv::Point(breastFatDiscarded[i].x, breastFatDiscarded[i].y));
        }
    }
    vecLength = breastPointsUsed.size();
    counter++;
    for(int i = 0; i < vecLength; i++){
       numerator = plane[0]*breastPointsUsed[i].x+plane[1]*breastPointsUsed[i].y+plane[2]*float(this->mHeightMap.at<uchar>(breastPointsUsed[i].x,breastPointsUsed[i].y))*thickness/255+plane[3];
        denominator = pow(pow(plane[0],2)+pow(plane[1],2)+pow(plane[2],2),0.5);
        avTemp += numerator/denominator;
        counter++;
    }
    //myfile.close();
    return avTemp/double(counter);
}
//#endif

double breast::breastThickAtPixel(const int i, const int j, const phantomCalibration calib, const string filTar, const string strKVP, const double t, const double exposure){
    pair<double,double> coeff3 = breast::glandpercent(calib, filTar, strKVP, t);
    return (log(double(this->mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
}
