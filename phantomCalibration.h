#pragma once

#include <vector>
#include <utility>
#include <string>
#include <fstream>
#include <map>

#include "scanner.h"
#include "various.h"

using namespace::std;

class phantomCalibration{
    public:
	//std::vector< std::pair<double, double> > dataArr;
	map<int, pair<double, double>> calibSet;
        phantomCalibration(){
        };
        ~phantomCalibration() {}
        // input the fibroglandular tissue thickness value into dataArr
        void inputData(map<int, vector<pair<double,double>>> rawData, const int i, const double tg, const double ln_MPV_mAs);
        // get the phantomCalibration class object with dataArr containing the data for given total thickness
        void getThicknessData(const string filTar, const int kV);
        void applyShift(const double shift);
        map<int, pair<double, double>> dataCorrection(const double x0, const double y0);
};

