#pragma once

#include <vector>
#include <utility>
#include <string>
#include <fstream>

#include "scanner.h"
#include "various.h"

class phantomCalibration {
    public:
	std::vector< std::pair<double, double> > dataArr;
        phantomCalibration(){
        };
        ~phantomCalibration() {}
        // input the fibroglandular tissue thickness value into dataArr
        void inputData(const double t, const double ln_MPV_mAs);
        // get the phantomCalibration class object with dataArr containing the data for given total thickness
        static phantomCalibration getThicknessData(const std::string filTar, const int kV, const int t);
        void applyShift(const double shift);
        void dataCorrection(const double x0, const double y0, const int kV, const std::string filTar, const int t);
};

