#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include "main_header.h"

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

vector<cv::Mat> breast::separate3channels(){
	// Separate the image into 3 channels.
	vector<cv::Mat> bgr_planes;
	cv::split(mMammo, bgr_planes);
	return bgr_planes;
}
void breast::drawHist(const int histSize){
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
}

pair<float, float> breast::findPeak(const int histSize){
    float iNMax = 0;
	float iBinMax = 0;
	for(int i = histSize - 1; i > histSize*0.5; i--){
		if(iNMax < mHistB.at<float>(i)){
			iNMax = mHistB.at<float>(i);
			iBinMax = i;
		}
	}
	return make_pair(iNMax, iBinMax);
}

float breast::findWidth(const int iBinMax, const int iNMax){
	// Find the width at the value with PEAK_VALUE/OL_WIDTH.
	float iQuartMax = 0;
	for(int i = iBinMax; i > 0; i--){
		if(mHistB.at<float>(i) < iNMax/OL_WIDTH){
			iQuartMax = i;
			break;
		}
    }
    return iQuartMax;
}

vector<cv::Point> breast::distanceTransform(){
    vector<vector<cv::Point>> pEdgeContours;
	cv::findContours(mMammoThreshed,pEdgeContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);
	int iContSize = 0;
	vector<cv::Point> pEdgeContour;
	for(auto i:pEdgeContours){
		if(iContSize < (int)i.size()){
			iContSize = i.size();
			pEdgeContour = i;
		}
	}
	return pEdgeContour;
}

bool breast::leftOrRight(vector<cv::Point> pEdgeContour){
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
	return bLeft;
}

float breast::findIMax(){
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
	return iMax;
}

vector<vector<cv::Point>> breast::findCorners(float iDivisor, const int iMax, const int iCOLOUR_MAX){
	cv::Mat mCornerThresh;
	int iCorners;
	do{
		cv::threshold(mCorner,mCornerThresh,iMax/iDivisor,iCOLOUR_MAX,0);
		cv::Mat mCornerT8U;
		mCornerThresh.convertTo(mCornerT8U, CV_8U);
		cv::findContours(mCornerT8U,pContours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
		iCorners = pContours.size();
		iDivisor+=0.05;

	} while ((iCorners < 5) && (iDivisor < 10));
        return pContours;
}

vector<cv::Point> breast::findCornerCentre(){
	for(auto i:pContours){
		cv::Moments momCont = cv::moments(i);
		vecContCents.push_back(cv::Point(momCont.m10/momCont.m00,momCont.m01/momCont.m00));
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
	return make_pair(iContPosX, iContPosY);
}

void breast::deleteUnneeded(const bool bLeft, cv::Mat mMammoThreshedCopy, const vector<cv::Point> pEdgeContourCopy, const int iContPosY){
    if (iContPosY < mMammo.rows){
            int extremalX=pEdgeContourCopy[0].x;
            int lastY=pEdgeContourCopy[0].y;
            //int lastX=pEdgeContourCopy[0].x;
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

            /* cv::cvtColor(mMammoThreshedCopy, mMammoThreshedCopy, cv::COLOR_BGR2GRAY); */
            for(auto i:pEdgeThrowAway){
                if(bLeft){
                    for(int x = 0; x <= i.x+2; x++){
                        mMammoThreshedCopy.at<uchar>(cv::Point(x, i.y)) = 0;
                    }
                } else {
                    for(int x = i.x-2; x < mMammoThreshed.cols; x++){
                        mMammoThreshedCopy.at<uchar>(i.y, x) = 0;
                    }
                }
            }
    }
}

vector<float> breast::getDistBright(){
    vector<int> vecDistAv; // Number of pixels at distance from black.
    cv::Mat_<int> mMammoDistChar = mMammoDist;
    vecDistBright.resize(256);
    vecDistAv.resize(256);
    cv::Mat mMammoCopy = mMammo8Bit;
    /* cv::cvtColor(mMammo, mMammoCopy, cv::COLOR_BGR2GRAY); */
    for(int i = 0; i < (int)vecDistBright.size(); ++i){ vecDistBright[i] = uchar(0);}
    for(int i = 0; i < (int)vecDistAv.size(); ++i){ vecDistAv[i] = 0;}
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

vector<float> breast::brestThickness(const int histSize, const cv::Mat_<int> mMammoDistChar, const cv::Mat mMammoCopy){
    vector<int> vecDistAvBrightest; // Number of pixels at distance from black.
    vecDistBrightBrightest.resize(256);
    vecDistAvBrightest.resize(256);
    for(int i = 0; i < (int)vecDistBrightBrightest.size(); ++i){ vecDistBrightBrightest[i] = uchar(0);}
    for(int i = 0; i < (int)vecDistAvBrightest.size(); ++i){ vecDistAvBrightest[i] = 0;}
    for(int i = 0; i < mMammo8Bit.cols; i++){
        for(int j = 0; j < mMammo8Bit.rows; j++){
            int iDist = int(mMammoDistChar(j,i));
            if(int(mMammo8Bit.at<uchar>(j,i)) >= vecDistBright[iDist]){
            vecDistBrightBrightest[iDist]+=float(mMammo8Bit.at<uchar>(j,i));
            vecDistAvBrightest[iDist]++;}
        }
    }
    for(int i = 0; i < (int)vecDistBrightBrightest.size(); ++i){ if(vecDistAvBrightest[i] != 0){vecDistBrightBrightest[i]/= float(vecDistAvBrightest[i]);}}
    return vecDistBrightBrightest;
}
vector<float> breast::normalBreastThickness(vector<float> vecDistBrightBrightest, const cv::Mat distImage){
    // Normalize the result to [ 0, histImage.rows ].
    vecDistBrightBrightest[255]=0;
    vecDistBrightBrightest[0]=0;
    cv::normalize(vecDistBrightBrightest, vecDistBrightBrightest, 0, distImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
    return vecDistBrightBrightest;
}

void breast::drawImages(string fileName, const cv::Mat distImage, const cv::Mat mCornerTresh, const cv::Mat mMammoThreshedCopy, const int histSize){
    /* fileName.pop_back(); */
    int dist_w = histSize*2;
    int dist_h = 512;
    #ifdef OL_DRAW_DIST
    int bin_w = cvRound(double(dist_w/histSize));
    for(int i = 1; i < histSize; i++){
        line(distImage, cv::Point(bin_w*(i), dist_h - cvRound(vecDistBrightBrightest[i])) ,
                        cv::Point(bin_w*(i), dist_h - cvRound(vecDistBrightBrightest[i])),
                        cv::Scalar(255, 255, 255), 2, 8, 0);
    }
    cv::imwrite(fileName+"_dist.png", distImage );
    #endif

    #ifdef OL_DRAW_CORNER
        cv::imwrite(fileName+"_corner.png", mCornerThresh);
    #endif

    #ifdef OL_DRAW_DISTMAP
        cv::imwrite(fileName+"_distmap.png", mMammoDist);
    #endif

    #ifdef OL_DRAW_THRESH
        cv::imwrite(fileName+"_thresh.png", mMammoThreshedCopy);
    #endif

    #ifdef OL_DRAW_ALTERED
        cv::imwrite(fileName+"_alt.png", mMammo);
    #endif
}

double breast::fibrogland(const cv::Mat imDat, const int thickness, const int exposure, calibData calibration){
    phantomCalibration temp = calibration["lower"];
    int sizeArr = temp.dataArr.size();
    double x[sizeArr];
    double y[sizeArr];
    size_t var = 0;
    for(auto it:temp.dataArr){
        x[var] = it.first;
        y[var] = it.second;
        var++;
    }
    pair<double,double> coeff1 = scanner::linearfit(x,y,var);
    temp = calibration["higher"];
    for(auto it:temp.dataArr){
        x[var] = it.first;
        y[var] = it.second;
        var++;
    }
    pair<double,double> coeff2 = scanner::linearfit(x,y,var);
    double yStep;
    if((coeff2.second - coeff1.second) == 0){
        yStep = 0;
    } else{
        yStep = (coeff2.second - coeff1.second)/10;
    }
    pair<double,double> coeff3;
    div_t divresult;
    divresult = div(thickness,10);
    coeff3.first = (coeff1.first+coeff2.first)/2;
    coeff3.second = coeff1.second+ yStep*divresult.rem;
    double tg(0);
    double tgTemp;
    ofstream myfile;
    myfile.open ("example2.txt");
    cout << "ab" << endl;
    cout << imDat << endl;
    for(int i = 0; i < imDat.cols; i++){
        cout << i << endl;
        for(int j = 0; j < imDat.rows; j++){
            cout << j << endl;
            if(int(imDat.at<Uint16>(j,i)) == 0){
                tgTemp = -coeff3.second/coeff3.first;
                cout << "a" << endl;
            } else{
                tgTemp = (log(double(imDat.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first;
                cout << "a" << endl;
            }
            if(tgTemp >= 0){
                tg += tgTemp;
                myfile << imDat.at<Uint16>(j,i) << " " << tgTemp << "\n";
                cout << "a" << endl;
            }
        }
    }
    myfile.close();
    return tg;
}

double breast::totalBreast(const mammography mammData){
    string bodyThickness = various::ToString<OFString>(mammData.BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    int counter(0);
    for(int i = 0; i < mMammoThreshed.cols; i++){
        for(int j = 0; j < mMammoThreshed.rows; j++){
            if(mMammoThreshed.at<Uint8>(j,i) == 1)
                counter++;
        }
    }
    return double(counter*thickness);
}

double breast::glandpercent(const double tg, const double t){
    return tg*100/t;
}

void breast::thicknessMap(const pair<double,double> coeff3, const int exposure, const mammography mammData){
    cv::Mat tg = cv::Mat(mMammo.rows, mMammo.cols, CV_16UC1, cvScalar(0));
    string maxValue = various::ToString<long>(mammData.LargestImagePixelValue);
    int maxVal = atoi(maxValue.c_str());
    string bodyThickness = various::ToString<OFString>(mammData.BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    for(int i = 0; i < mMammo.cols; i++){
        for(int j = 0; j < mMammo.rows; j++){
            if(mMammoThreshed.at<Uint8>(j,i) > 0){
                    if(int(mMammo.at<Uint16>(j,i)) == 0){
                        tg.at<Uint16>(j,i) = thickness;
                    } else{
                        tg.at<Uint16>(j,i) = ((log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first)*double(maxVal/255);
                        //cout << (log(double(mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first << endl;
                    }
        }
    }
    }
    //cv::normalize(tg, tg, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
    cv::imwrite("test_thickMap.jpg",tg);
}
