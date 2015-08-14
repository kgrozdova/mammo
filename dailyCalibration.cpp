#include "dailyCalibration.h"

using namespace::std;

void dailyCalibration::insertFilTar(mammography mammData){
    string filter = various::ToString<OFString>(mammData.Filter);
    string target = various::ToString<OFString>(mammData.Target);
    if(filter == "RHODIUM" && target == "RHODIUM"){
        this->filTarQC = "RhRh";
    } else if(filter == "MOLYBDENUM" && target == "MOLYBDENUM"){
        this->filTarQC = "MoMo";
    } else if(filter == "MOLYBDENUM" && target == "RHODIUM"){
        this->filTarQC = "MoRh";
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
        MPV = various::ToDouble<string>(MPVStr);
        qc_ln_MPV_mAs = log(MPV/(double)mammData.Exposure);
        tStr = str.substr(found+1,(int)found2-(int)found-1);
        this->t = various::ToDouble<string>(tStr);
        tgStr = str.substr(found2+1, sizeStr-found2);
        this->tg = various::ToDouble<string>(tgStr);
    }
}
