#include "phantomCalibration.h"

using namespace std;

void phantomCalibration::getThicknessData(const string filTar, const int kV){
    // load calibration data
    map<int, vector<pair<double,double>>> rawData;
    for(int i = 20; i <= 70; i+= 10){
        rawData = various::getThicknessDataFound(rawData, i, filTar, kV);
    }

    if(kV == 29 && filTar == "RhRh"){
        for(int i = 5; i <= 10; i+= 5){
            rawData = various::getThicknessDataFound(rawData, i, filTar, kV);
        }
        int i = 65;
            rawData = various::getThicknessDataFound(rawData, i, filTar, kV);
    }
    // perform linear fitting intp the calibration data set
    int sizeArr(0);
    for(auto it:rawData){
        sizeArr = it.second.size();
        double* x;
        x = new double[sizeArr];
        double* y;
        y = new double[sizeArr];
        int var(0);
        for(auto it1:it.second){
                x[var] = it1.first;
                y[var] = it1.second;
                var++;
        }
        calibSet[it.first] = scanner::linearfit(x,y,sizeArr);
        delete[] x;
        delete[] y;
    }
}

map<int, pair<double, double>> phantomCalibration::dataCorrection(const double x0, const double y0){
    double shift, x;
    map<int, pair<double, double>> calibCopy;
    pair<double,double> copyPair;
    for(auto it:calibSet){
            shift = scanner::calcShift(it.second, x0, y0);
            x = (y0 - it.second.second)/it.second.first;
            copyPair = {it.second.first, it.second.second+((x+shift)*it.second.second)/x-it.second.second};
            calibCopy[it.first] = copyPair;
    }
    return calibCopy;
}
