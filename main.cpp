#include "main_header.h"
#include "breast.h"

#include <map>
#include <iterator>
#include <vector>

 #define KSENIA_STUFF
typedef map< string, phantomCalibration> calibData;


int main(int argc, char** argv){
    string strCmdName = argv[0];
    if(argc != 2){
        cerr << "Error: no DICOM file specified. Usage: " << strCmdName << " FILENAME.dcm" << endl;
        return -1;
    }
    string strFileName = argv[1];
    /* mammData.mammography::loadHeaderData(strFileName); */
    // load data from processed DICOM file
    /* mammData.mammography::loadPixelData(strFileName); */

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
    breast breastDat = breast(strFileName);
    strFileName = breast::fileNameErase(strFileName);
    /* int iColourMax = breastDat.getBitDepth(); */

    //
    // SEPERATING THE BREAST FROM THE BACKGROUND
    //
    //int histSize = 255;

    //
    //FINDING THE CONTOUR WHICH THE DESRIBES THE EDGE OF THE BREAST
    //

	// Threshold the image to 'cut off' the brighter peak from the histogram.
	// Deep copy.
	cv::Mat mMammoThreshedCopy = breastDat.mMammoROI.clone();
    //
    // DISTANCE TRANSFORM
    //
    // Convert the threshold into greyscale to stop the distance transform complaining. Consider moving this to the start of the programme.
	/* cv::cvtColor(breastDat.mMammoThreshed,breastDat.mMammoThreshed, cv::COLOR_BGR2GRAY); */
	// Use a less accurate but smoother looking distance transform. More research needed here

    // This line breaks the distance transform.
    // I'm pretty sure we don't need it...
	/* cv::distanceTransform(breastDat.mMammoROI, breastDat.mMammoDist, cv::DIST_L2, cv::DIST_MASK_PRECISE, CV_32F); */
	cv::Mat mMammoThreshedCont;
	breastDat.mMammoROI.convertTo(mMammoThreshedCont, CV_8U, 1./256);
	// The thresholded image gets broken here.
	//
	// FIGURE OUT WHETHER WE ARE LOOKING AT A LEFT OR RIGHT BREAST
	//
	 /* bool bLeft = breastDat.leftOrRight(); */
	 //
	 // TRY TO FIND A CORNER NEAR THE BOTTOM OF THE BREAST
	 //
    // Make a probability map of likely corners in the mammogram.
	cv::cornerHarris(breastDat.mMammoROI, breastDat.mCorner, 40, 3, 0.04);
	/* float iMax = breastDat.findIMax(); */
	// Threshold this map repeatedly until we find at least three probable regions.
	cv::Mat mCornerThresh;
	/* float iDivisor = 1.25; */
	/* vector<vector<cv::Point>> pContours = breastDat.findCorners(iDivisor, iMax, iColourMax); */
		// Find the most spatially spread out corner. Unused, currently - what does it mean?
	/* sort(pContours.begin(),pContours.end(),[] */
	/* 	(const vector<cv::Point> &l, const vector<cv::Point> &r){ */
	/* 		return cv::contourArea(l) > cv::contourArea(r); */
	/* 	}); */
    // Find the centers of the corners.
    /* vector<cv::Point> vecContCents = breastDat.findCornerCentre(); */

    // Pick a corner to cut off.#
    /* pair<int, int> iContPos = breastDat.pickCornerCutOff(bLeft); */

    //
    // PAINT OVER UNINTERESTING PARTS OF THE BREAST ON THE THRESHOLDED IMAGE
    //

    /* breastDat.deleteUnneeded(bLeft, mMammoThreshedCopy, pEdgeContourCopy, iContPos.second); */

    //
    // FINDING THE BREAST THICKNESS
    //
        // Normalise the distance map to fit onto our graph.
    //
    //	    This is dangerous... we're losing information here. 32 bit -> 8 bits = 16 million times less resolution...
    //		We desperately need to look at our classes etc. and make it saner...

    //
    // DRAWING THE PICTURES
#ifdef KSENIA_STUFF
    /* CORRECTION FOR OVEREXPOSURE */
    /* vector<pair<int,int>> pixelOfInterestExposureVec = breastDat.pixelOfInterestExposure();
    map<int,vector<pair<double,pair<int,int>>>> breastDistMap = breastDat.distMap(pixelOfInterestExposureVec);
    breastDat.applyExposureCorrection(breastDistMap); */

     /* IMPORT DAILY CALIBRATION DATA */
     dailyCalibration dcalib;
     dcalib.insertFilTar(breastDat);
     dcalib.InserQcTTg(breastDat, "qc.dat");

     /* LOAD CALIBRATION DATA */
     phantomCalibration calib;
     calib.getThicknessData(dcalib.filTarQC, atoi(breastDat.strKVP.c_str()));

    /* APPLY DAILY CALIBRATION CORRECTION */
    /* calib.calibSet = calib.dataCorrection(dcalib.tg,dcalib.qc_ln_MPV_mAs); */

    /* CALCULATE TOTAL BREAST THICKNESS AND TOTAL FIBROGLANDULAR TISSUE THICKNESS*/
    double t = breastDat.totalBreast(dcalib.filTarQC);
    double tg = breastDat.fibrogland(calib,dcalib.filTarQC, t);

    /* OUTPUT 2D BREAST THICKNESS MAP */
    /* breastDat.thicknessMap(calib, strKVP, exposure, dcalib); */

    /* FIND THE BORDER, WHERE A BREAST LOSES CONTACT WITH COMPRESSION PADDLE */
    /* vector<cv::Point> contactBorder = breastDat.getContact(); */

    /* OUTPUT RESULT TO res.txt */
    ostringstream strs;
    strs << tg/t*100;
    string str = strs.str();
    ofstream out;
    out.open("res.txt", ios::app);
    out << strFileName << " " << str << "\n";
    cout << str << endl;

#endif
}
