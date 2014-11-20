#include <string>
#include <sstream>
#include "main_header.h"

using namespace std;

void various::iterateVectorToFile(vector<pair<double, double>> vectorInput, string fileName){
    ofstream myfile;
    myfile.open (fileName);
    for(vector<pair<double, double>>::iterator it = vectorInput.begin(); it != vectorInput.end(); ++it){
        myfile << it->first << " " << it->second << "\n";
    }
    myfile.close();
}

string various::fileNameErase(string fileName){
    #define OL_EXT_LENGTH 4
    return fileName.erase(fileName.length()-OL_EXT_LENGTH, fileName.length()); // Remove file extension.
}

