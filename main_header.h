#pragma once

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "mammography.h"
#include "various.h"
#include "phantomCalibration.h"
#include "dailyCalibration.h"
#include "scanner.h"

/* #define KSENIA_STUFF */

using namespace std;

class phantomCalibration;

typedef map< string, phantomCalibration> calibData;


template <typename T> string various::ToString(T input){
    stringstream ss;
    ss << input;
    string output = ss.str();
    return output;
}

