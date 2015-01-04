#include "phantomCalibration.h"

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
    cout << fileName << endl;
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
    return thicknessData;
}

void phantomCalibration::applyShift(const double shift){
    for(vector<pair<double, double>>::iterator it = dataArr.begin(); it != dataArr.end(); ++it){
        it->first += shift;
    }
}

void phantomCalibration::dataCorrection(const double x0, const double y0, const int kV, const string filTar, const int t){
    int thickArr[2];
    div_t divresult;
    divresult = div(t,10);
    thickArr[0] = divresult.quot*10;
    thickArr[1] = (divresult.quot+1)*10;
    calibData dat;
    phantomCalibration calib1;
    calib1 = phantomCalibration::getThicknessData(filTar, kV, thickArr[0]);
    dat["lower"] = calib1;
    phantomCalibration calib2;
    calib2 = phantomCalibration::getThicknessData(filTar, kV, thickArr[1]);
    dat["higher"] = calib2;
    phantomCalibration temp1 = dat["lower"];
    int sizeArr = temp1.dataArr.size();
    double x1[temp1.dataArr.size()];
    double y1[temp1.dataArr.size()];
    size_t var = 0;
    for(auto it:temp1.dataArr){
        x1[var] = it.first;
        y1[var] = it.second;
        var++;
    }
    pair<double,double> coeff1 = scanner::linearfit(x1,y1,var);
    phantomCalibration temp2 = dat["higher"];
    sizeArr = temp2.dataArr.size();
    double x2[temp2.dataArr.size()];
    double y2[temp2.dataArr.size()];
    var = 0;
    for(auto it:temp2.dataArr){
        x2[var] = it.first;
        y2[var] = it.second;
        var++;
    }
    pair<double,double> coeff2 = scanner::linearfit(x2,y2,var);
    double yStep;
    if((coeff2.second - coeff1.second) == 0){
        yStep = 0;
    } else{
        yStep = (coeff2.second - coeff1.second)/10;
    }
    pair<double,double> coeff3;
    divresult = div(t,10);
    coeff3.first = (coeff1.first+coeff2.first)/2;
    coeff3.second = coeff1.second+yStep*divresult.rem;
    double shift = scanner::calcShift(coeff3, x0, y0);
    this->applyShift(shift);
}
