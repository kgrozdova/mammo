#include <utility>
#include "main_header.h"
#include <fstream>
#include <string>

using namespace std;

void phantomCalibration::inputData(const double tg, const double ln_MPV_mAs){
    dataArr.push_back(make_pair(tg,ln_MPV_mAs));
}

phantomCalibration phantomCalibration::getThicknessData(const string filTar, const int kV, const int t){
    phantomCalibration thicknessData;
    ifstream ifs;
    string str, gland, ln_MPV_mAs;
    size_t found,found2, found3, found4;
    string kVStr = various::ToString<int>(kV);
    string strT = various::ToString<int>(t);
    string fileName = "av_calib_data" + filTar + kVStr + "_" + strT + ".csv";
    ifs.open(fileName, ifstream::in);
    while (ifs.good()){
        getline(ifs, str);
        if( ifs.eof() ) break;
        found = str.find(",");
        found2 = str.find(",",found+1);
        found3 = str.find(",",found2+1);
        found4 = str.find(",",found3+1);
        gland = str.substr(found2+1, found3-found2-1);
        ln_MPV_mAs = str.substr(found3+1, found4-found3-1);
        thicknessData.inputData(atof(gland.c_str()), atof(ln_MPV_mAs.c_str()));
    }
    ifs.close();
    various::iterateVectorToFile(thicknessData.dataArr, "myexample.txt");
    cout << "iterating through vec" << endl;
    for(auto i:thicknessData.dataArr){
	cout << i.first << endl;
    }
    cout << "done." << endl;
    return thicknessData;
}

void phantomCalibration::applyShift(const double shift){
    for(vector<pair<double, double>>::iterator it = dataArr.begin(); it != dataArr.end(); ++it){
        it->first += shift;
    }
}

void phantomCalibration::dataCorrection(const double x0, const double y0, const int kV, const string filTar, const int t){
    phantomCalibration calibrationData;
    calibrationData = phantomCalibration::getThicknessData(filTar, kV, t);
    size_t var = 0;
    double x[calibrationData.dataArr.size()];
    double y[calibrationData.dataArr.size()];
    for(auto it:calibrationData.dataArr){
        x[var] = it.first;
        y[var] = it.second;
        var++;
    }
    pair<double,double> ret = scanner::linearfit(x,y,(sizeof(x)/sizeof(*x)));
    double shift = scanner::calcShift(ret, x0, y0);
    this->applyShift(shift);
    various::iterateVectorToFile(calibrationData.dataArr, "example3.txt");
}
