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
    map<string, double> b_a = {{"MoMo", -0.053}, {"MoRh26", -0.041}, {"MoRh27", -0.0466}, {"MoRh28", -0.053}, {"MoRh29", -0.046}, {"RhRh29", -0.047}, {"RhRh30", -0.042}, {"RhRh31", -0.041}};
    map<string, double> b_b = {{"MoMo", 4.402}, {"MoRh26", 4.321}, {"MoRh27", 4.827}, {"MoRh28", 5.173}, {"MoRh29", 5.123}, {"RhRh29", 5.39}, {"RhRh30", 5.313}, {"RhRh31", 5.444}};
        phantomCalibration(){
        };
        ~phantomCalibration() {}
        // GET THE phantomCalibration CLASS OBJECT WITH dataArr CONTAINING THE DATA FOR GIVEN TOTAL THICKNESS
        void getThicknessData(const string filTar, const int kV);
        // CALIBRATION DATA CORRECTION ACCORDING TO THE QC DATA
        map<int, pair<double, double>> dataCorrection(const double x0, const double y0);
};

