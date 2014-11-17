#include "main_header.h"
#include <map>
#include <iterator>
#include <vector>
typedef map< string, phantomCalibration> calibData;

int main(){

    mammography mammData;
    mammData.mammography::loadHeaderData("Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm");
    // load data from processed DICOM file
    mammData.mammography::loadPixelData("Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm");
//    // convert dicom to jpeg
//    char* DICOMfile = "Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm";
//    char* JPEGfile = "Control_prior_anon-00001-LMLO111105-20110328-RAW.jpeg";
//    char command[50];
//    sprintf(command, "dcmj2pnm %s %s +oj", DICOMfile, JPEGfile);
//    system(command);

    //                  //
    // Image processing //
    //                  //
    #define OL_WIDTH 512
    #define OL_EXT_LENGTH 4
    #define OL_DRAW
    #ifdef OL_DRAW
    //	#define OL_DRAW_HIST
    #define OL_DRAW_THRESH
    #define OL_DRAW_DIST
    //	#define OL_DRAW_DISTMAP
    //	#define OL_DRAW_CORNER
    #endif
    //cv::Mat mMammo = breast::getMat(JPEGfile);
    cv::Mat mMammo((int)mammData.Rows, (int)mammData.Columns, CV_16UC1, 0);
    for(int i = 0; i < mammData.Rows; i++){
        for(int j = 0; j < mammData.Columns; j++){
             //mMammo.at<int>(i,j)= mammData.pixelArr[i+(int)mammData.Columns*j];
        }
    }
    string strFileName = breast::fileNameErase("Control_prior_anon-00001-LMLO111105-20110328-PROCESSED.dcm");

    int iCOLOUR_MAX = breast::getBitDepth(mMammo);

    //
    // SEPERATING THE BREAST FROM THE BACKGROUND
    //
    vector<cv::Mat> bgr_planes = breast::separate3channels(mMammo);
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
    breast::drawHist(histSize);

    pair<float, float> iNBin = breast::findPeak(mHistB, histSize);
    float iQuartMax = breast::findWidth(iNBin.second, iNBin.first, mHistB);

    //
    //FINDING THE CONTOUR WHICH THE DESRIBES THE EDGE OF THE BREAST
    //
    cv::Mat mMammoThreshed;
	// Threshold the image to 'cut off' the brighter peak from the histogram.
	cv::threshold(mMammo, mMammoThreshed, iCOLOUR_MAX*(iQuartMax/histSize), iCOLOUR_MAX, 1);
	// MAGIC
	cv::Mat	mMammoThreshedCopy = mMammoThreshed;
    //
    // DISTANCE TRANSFORM
    //

    // Convert the threshold into greyscale to stop the distance transform complaining. Consider moving this to the start of the programme.
	cv::cvtColor(mMammoThreshed,mMammoThreshed, cv::COLOR_BGR2GRAY);
	// Use a less accurate but smoother looking distance transform. More research needed here
	cv::Mat mMammoDist;
	cv::distanceTransform(mMammoThreshed, mMammoDist, cv::DIST_L2, cv::DIST_MASK_PRECISE, CV_32F);
    cv::Mat mMammoThreshedCont;
	mMammoThreshed.convertTo(mMammoThreshedCont, CV_8U);
	vector<cv::Point> pEdgeContour = breast::distanceTransform(mMammoThreshed);

	//
	// FIGURE OUT WHETHER WE ARE LOOKING AT A LEFT OR RIGHT BREAST
	//
	vector<cv::Point> pEdgeContourCopy = pEdgeContour;
	bool bLeft = breast::leftOrRight(pEdgeContour, mMammo);
	 //
	 // TRY TO FIND A CORNER NEAR THE BOTTOM OF THE BREAST
	 //

    // Make a probability map of likely corners in the mammogram.
	cv::Mat mCorner;
	cv::cornerHarris(mMammoThreshed, mCorner, 40, 3, 0.04);
	float iMax = breast::findIMax(mCorner);
	// Threshold this map repeatedly until we find at least three probable regions.
    cv::Mat mCornerThresh;
	float iDivisor = 1.25;
	vector<vector<cv::Point>> pContours = breast::findCorners(iDivisor, mCorner, iMax, iCOLOUR_MAX);
		// Find the most spatially spread out corner. Unused, currently - what does it mean?
	sort(pContours.begin(),pContours.end(),[]
		(const vector<cv::Point> &l, const vector<cv::Point> &r){
			return cv::contourArea(l) > cv::contourArea(r);
		});
    // Find the centers of the corners.
    vector<cv::Point> vecContCents = breast::findCornerCentre(pContours);

    // Pick a corner to cut off.
    pair<int, int> iContPos = breast::pickCornerCutOff(mMammo, pContours, vecContCents, bLeft);
    //
    // PAINT OVER UNINTERESTING PARTS OF THE BREAST ON THE THRESHOLDED IMAGE
    //

    breast::deleteUnneeded(mMammo, pEdgeContourCopy, bLeft, mMammoThreshed, mMammoThreshedCopy, iContPos.second);

    //
    // FINDING THE BREAST THICKNESS
    //

    // Normalise the distance map to fit onto our graph.
    cv::normalize(mMammoDist, mMammoDist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
    vector<float> vecDistBright = breast::getDistBright(mMammoDist, mMammo);

    int dist_w = histSize*2; int dist_h = 512;
    cv::Mat distImage(dist_h, dist_w, CV_8UC3, cv::Scalar(0,0,0));
    cv::Mat_<int> mMammoDistChar = mMammoDist;
    cv::Mat mMammoCopy;
    cv::cvtColor(mMammo, mMammoCopy, cv::COLOR_BGR2GRAY);
    vector<float> vecDistBrightBrightest = breast::brestThickness(mMammo, mMammoDist, vecDistBright, histSize,
                                                                    mMammoDistChar, mMammoCopy);

    //
    // DRAWING THE PICTURES
    //
    breast::drawImages(vecDistBrightBrightest, strFileName, distImage, mCornerThresh, mMammoDist, mMammoThreshedCopy, mMammo, histSize);
    string KVP = various::ToString<OFString>(mammData.KVP);
    string bodyThickness = various::ToString<OFString>(mammData.BodyPartThickness);
    int thickness = atoi(bodyThickness.c_str());
    int thickArr[2];
    div_t divresult;
    divresult = div(thickness,10);
    thickArr[0] = divresult.quot*10;
    thickArr[1] = (divresult.quot+1)*10;
    dailyCalibration dcalib;
    dcalib.insertFilTar(mammData);
    dcalib.InserQcTTg(mammData, "qc.dat");
    calibData dat;
    phantomCalibration calib1;
    calib1 = phantomCalibration::getThicknessData(dcalib.filTar, atoi(KVP.c_str()), thickArr[0]);
    calib1.phantomCalibration::dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs, atoi(KVP.c_str()), dcalib.filTar, dcalib.t);
    dat["lower"] = calib1;
    phantomCalibration calib2;
    calib2 = phantomCalibration::getThicknessData(dcalib.filTar, atoi(KVP.c_str()), thickArr[1]);
    calib2.phantomCalibration::dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs, atoi(KVP.c_str()), dcalib.filTar, dcalib.t);
    dat["higher"] = calib2;
    int tg = breast::fibrogland(mammData,dat);
    int t = breast::totalBreast(mammData);
    double glandPercent = breast::glandpercent(tg, t);
    cout << glandPercent << endl;
}
