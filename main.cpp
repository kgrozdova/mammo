#include "main_header.h"
#include "breast.h"

#include <map>
#include <iterator>
#include <vector>
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
    int histSize = 255;

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
	 bool bLeft = breastDat.leftOrRight();
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

    // Pick a corner to cut off.
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
    //

    cout << "Is fat? " << (breastDat.getPixelType(400,1900) == XIN_FAT) << endl << endl;
    cout << "Thickness: " << breastDat.getHeight(100,100) << endl << endl;
    breastDat.drawImages(strFileName, mCornerThresh, mMammoThreshedCopy, histSize);

    string KVP = various::ToString<OFString>(breastDat.KVP);
    string bodyThickness = various::ToString<OFString>(breastDat.BodyPartThickness);
    string Exposure = various::ToString<long>(breastDat.Exposure);
    double thickness = atoi(bodyThickness.c_str());
    int exposure = atoi(Exposure.c_str());
//    int thickArr[2];
//    div_t divresult;
//    divresult = div(thickness,10);
//    thickArr[0] = divresult.quot*10;
//    thickArr[1] = (divresult.quot+1)*10;
    dailyCalibration dcalib;
    dcalib.insertFilTar(breastDat);
    dcalib.InserQcTTg(breastDat, "qc.dat");
    phantomCalibration calib;
    calib.getThicknessData(dcalib.filTar, atoi(KVP.c_str()));
    //calib.calibSet = calib.dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);

//    calibData dat;
//    phantomCalibration calib1;
//    calib1 = phantomCalibration::getThicknessData(dcalib.filTar, atoi(KVP.c_str()), thickArr[0]);
//    calib1.phantomCalibration::dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs, atoi(KVP.c_str()), dcalib.filTar, dcalib.t);
//    dat["lower"] = calib1;
//    phantomCalibration calib2;
//    calib2 = phantomCalibration::getThicknessData(dcalib.filTar, atoi(KVP.c_str()), thickArr[1]);
//    calib2.phantomCalibration::dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs, atoi(KVP.c_str()), dcalib.filTar, dcalib.t);
//    dat["higher"] = calib2;
    //int tg = breast::fibrogland(breastDat.mMammo, thickness, exposure, dat);

//    phantomCalibration temp1 = dat["lower"];
//    int sizeArr = temp1.dataArr.size();
//    double x1[sizeArr];
//    double y1[sizeArr];
//    size_t var = 0;
//    for(auto it:temp1.dataArr){
//        x1[var] = it.first;
//        y1[var] = it.second;
//        var++;
//    }
//    pair<double,double> coeff1 = scanner::linearfit(x1,y1,var);
//    phantomCalibration temp2 = dat["higher"];
//    sizeArr = temp2.dataArr.size();
//    double x2[sizeArr];
//    double y2[sizeArr];
//    var = 0;
//    for(auto it:temp2.dataArr){
//        x2[var] = it.first;
//        y2[var] = it.second;
//        var++;
//    }
//    pair<double,double> coeff2 = scanner::linearfit(x2,y2,var);
//    double yStep;
//    if((coeff2.second - coeff1.second) == 0){
//        yStep = 0;
//    } else{
//        yStep = (coeff2.second - coeff1.second)/10;
//    }
//    pair<double,double> coeff3;
//    divresult = div(thickness,10);
//    coeff3.first = (coeff1.first+coeff2.first)/2;
//    coeff3.second = coeff1.second+yStep*divresult.rem;

    pair<double,double> coeff3;
    double tg(0);
    double tgTemp;
    /* double refer(0); */
//    double breastMaxVal;
//    for(int i = 0; i < breastDat.mMammo.cols; i++){
//        for(int j = 0; j < breastDat.mMammo.rows; j++){
//            if(breastDat.mMammoROI.at<Uint8>(j,i) != 0){
//                if(breastDat.mMammo.at<Uint16>(j,i) > refer){
//                    refer = breastDat.mMammo.at<Uint16>(j,i);
//                    breastMaxVal = breastDat.mMammo.at<Uint16>(j,i);
//                }
//            }
//        }
//    }
    /* double intercept = log(breastMaxVal/double(breastDat.Exposure)); */
    /* ofstream myfile; */
    /* myfile.open ("example2.txt"); */
    /* int num(0); */
    /* for(int i = 0; i < breastDat.mMammo.cols; i++){ */
    /*     for(int j = 0; j < breastDat.mMammo.rows; j++){ */
    /*         if(breastDat.isBreast(j,i)){ */
    /*             //thickness = breastDat.getHeight(j, i); */
    /*             coeff3 = breast::glandpercent(calib, dcalib.filTar, KVP, thickness); */
    /*             if(int(breastDat.mMammo.at<Uint16>(j,i)) == 0){ */
    /*                 tgTemp = thickness; */
    /*             } else{ */
    /*                    tgTemp = (log(double(breastDat.mMammo.at<Uint16>(j,i))/exposure)-coeff3.second)/coeff3.first; */
    /*             } */
    /*             if(tgTemp >= 0 && tgTemp <= thickness){ */
    /*                 tg += tgTemp; */
    /*             }else if(tgTemp > thickness){ */
    /*                 tg += thickness; */
    /*             } else{ */
    /*                 tg += 0; */
    /*             } */
    /*             myfile << int(breastDat.mMammo.at<Uint16>(j,i))<< " " << tgTemp << "\n"; */
    /*             num++; */
    /*         } */
    /*     } */
    /* } */
    /* cout << num << endl; */
    /* myfile.close(); */
    /* double t = breastDat.totalBreast(); */
    /* breastDat.thicknessMap(coeff3, exposure); */
    /* breastDat.thicknessMapRedVal(coeff3, exposure); */
    /* cout << tg/t*100 << endl; */

    /* pair<double, pair<int, int> > fatPixel = breastDat.findFatCompressed(bLeft); */
    /* vector<pair<int,int>> borderShape = breastDat.contactBorderShape(fatPixel, thickness); */
    /* breastDat.thicknessMapRedValBorder(coeff3, exposure, borderShape); */
}
