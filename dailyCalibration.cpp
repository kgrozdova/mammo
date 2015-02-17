#include "dailyCalibration.h"

using namespace::std;

void dailyCalibration::insertFilTar(mammography mammData){
    string filter = various::ToString<OFString>(mammData.Filter);
    string target = various::ToString<OFString>(mammData.Target);
    if(filter == "RHODIUM" && target == "RHODIUM"){
        filTar = "RhRh";
    } else if(filter == "MOLYBDENUM" && target == "MOLYBDENUM"){
        filTar = "MoMo";
    } else if(filter == "MOLYBDENUM" && target == "RHODIUM"){
        filTar = "MoRh";
    } else{
        cerr << "Error in filter/target combination";
    }
}

void dailyCalibration::InserQcTTg(mammography mammData, const string fileName){
    ifstream ifs;
    ifs.open(fileName, ifstream::in);
    double MPV;
    size_t found,found2;
    int sizeStr;
    string str, MPVStr, tStr, tgStr;
    if(ifs.good()){
        getline(ifs, str);
        sizeStr = str.length();
        found = str.find(",");
        found2 = str.find(",",found+1);
        MPVStr = str.substr(0, found);
        istringstream i(MPVStr);
        i >> MPV;
        qc_ln_MPV_mAs = log(MPV/(double)mammData.Exposure);
        tStr = str.substr(found+1,(int)found2-(int)found-1);
        istringstream k(tStr);
        k >> t;
        tgStr = str.substr(found2+1, sizeStr-found2);
        cout << tgStr << endl;
        istringstream l(tgStr);
        l >> tg;
        cout << tg << " " << qc_ln_MPV_mAs << endl;
    }
}


