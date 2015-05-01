#include "phantomCalibration.h"

using namespace std;

//void phantomCalibration::inputData(map<int, vector<pair<double,double>>> rawData, const int i, const double tg, const double ln_MPV_mAs){
//    rawData[i].push_back(make_pair(tg,ln_MPV_mAs));
//}

void phantomCalibration::getThicknessData(const string filTar, const int kV){
    map<int, vector<pair<double,double>>> rawData;
    ifstream ifs;
    string fileName;
    string str, gland, ln_MPV_mAs;
    size_t found,found2, found3, found4;
    string kVStr = various::ToString<int>(kV);
    int counter(0);
    for(int i = 20; i <= 70; i+= 10){
        rawData[i];
        fileName = "av_calib_data" +     filTar + kVStr + "_" + to_string(i) + ".csv";
        ifs.open(fileName, ifstream::in);
        while (ifs.good()){
            getline(ifs, str);
            counter++;
            if( ifs.eof() ) break;
            if(counter != 1){
            found = str.find(",");
            found2 = str.find(",",found+1);
            found3 = str.find(",",found2+1);
            found4 = str.find(",",found3+1);
            gland = str.substr(found2+1, found3-found2-1);
            ln_MPV_mAs = str.substr(found3+1, found4-found3-1);
            rawData[i].push_back(make_pair(atof(gland.c_str()), atof(ln_MPV_mAs.c_str())));
            //inputData(rawData, i, atof(gland.c_str()), atof(ln_MPV_mAs.c_str()));
            }
        }
        counter = 0;
        ifs.close();
    }
    counter = 0;
    if(kV == 29 && filTar == "RhRh"){
        for(int i = 5; i <= 10; i+= 5){
            rawData[i];
            fileName = "av_calib_data" + filTar + kVStr + "_" + to_string(i) + ".csv";
            ifs.open(fileName, ifstream::in);
            while (ifs.good()){
                getline(ifs, str);
                counter++;
                if( ifs.eof()) break;
                if(counter != 1){
                found = str.find(",");
                found2 = str.find(",",found+1);
                found3 = str.find(",",found2+1);
                found4 = str.find(",",found3+1);
                gland = str.substr(found2+1, found3-found2-1);
                ln_MPV_mAs = str.substr(found3+1, found4-found3-1);
                rawData[i].push_back(make_pair(atof(gland.c_str()), atof(ln_MPV_mAs.c_str())));
                //inputData(rawData, i, atof(gland.c_str()), atof(ln_MPV_mAs.c_str()));
                }
            }
            counter  = 0;
            ifs.close();
        }
        counter = 0;
        int i = 65;
        rawData[i];
        fileName = "av_calib_data" + filTar + kVStr + "_" + to_string(i) + ".csv";
        ifs.open(fileName, ifstream::in);
        while (ifs.good()){
            getline(ifs, str);
            counter++;
            if( ifs.eof() ) break;
            if(counter != 1){
            found = str.find(",");
            found2 = str.find(",",found+1);
            found3 = str.find(",",found2+1);
            found4 = str.find(",",found3+1);
            gland = str.substr(found2+1, found3-found2-1);
            ln_MPV_mAs = str.substr(found3+1, found4-found3-1);
            rawData[i].push_back(make_pair(atof(gland.c_str()), atof(ln_MPV_mAs.c_str())));
            //inputData(rawData, i, atof(gland.c_str()), atof(ln_MPV_mAs.c_str()));
            }
        }
        ifs.close();
    }

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

//void phantomCalibration::applyShift(const double shift){
//    for(vector<pair<double, double>>::iterator it = dataArr.begin(); it != dataArr.end(); ++it){
//        it->first += shift;
//    }
//}


//void phantomCalibration::dataCorrection(const double x0, const double y0, const int kV, const string filTar, const int t){
//    int thickArr[2];
//    div_t divresult;
//    divresult = div(t,10);
//    thickArr[0] = divresult.quot*10;
//    thickArr[1] = (divresult.quot+1)*10;
//    calibData dat;
//    phantomCalibration calib1;
//    calib1 = phantomCalibration::getThicknessData(filTar, kV, thickArr[0]);
//    dat["lower"] = calib1;
//    phantomCalibration calib2;
//    calib2 = phantomCalibration::getThicknessData(filTar, kV, thickArr[1]);
//    dat["higher"] = calib2;
//    phantomCalibration temp1 = dat["lower"];
//    int sizeArr = temp1.dataArr.size();
//    double x1[temp1.dataArr.size()];
//    double y1[temp1.dataArr.size()];
//    size_t var = 0;
//    for(auto it:temp1.dataArr){
//        x1[var] = it.first;
//        y1[var] = it.second;
//        var++;
//    }
//    pair<double,double> coeff1 = scanner::linearfit(x1,y1,var);
//    phantomCalibration temp2 = dat["higher"];
//    sizeArr = temp2.dataArr.size();
//    double x2[temp2.dataArr.size()];
//    double y2[temp2.dataArr.size()];
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
//    divresult = div(t,10);
//    coeff3.first = (coeff1.first+coeff2.first)/2;
//    coeff3.second = coeff1.second+yStep*divresult.rem;
//    double shift = scanner::calcShift(coeff3, x0, y0);
//    this->applyShift(shift);
//}

map<int, pair<double, double>> phantomCalibration::dataCorrection(const double x0, const double y0){
    double shift, x;
    map<int, pair<double, double>> calibCopy;
    pair<double,double> copyPair;
    for(auto it:calibSet){
            shift = scanner::calcShift(it.second, x0, y0);
            cout << shift << endl;
            x = (y0 - it.second.second)/it.second.first;
            copyPair = {it.second.first, it.second.second+((x+shift)*it.second.second)/x-it.second.second};
            calibCopy[it.first] = copyPair;
    }
    for(map<int, pair<double, double>>::iterator it = calibCopy.begin(); it != calibCopy.end(); ++it){
        cout << it->first << " " << it->second.first << " " << it->second.second << " " << endl;
    }
    return calibCopy;
}
