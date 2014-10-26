#include <utility>
#include "main_header.h"
#include "libxls/xls.h"

using namespace std;

xlsWorkSheet* phantomCalibration::getParsedSheet(const string fileName, int pageNum){
    char* fName = const_cast<char*>(fileName.c_str());
    xlsWorkBook* pWB;
    xlsWorkSheet* pWS;
    pWB = xls_open(fName,"UTF-8");
    pWS = xls_getWorkSheet(pWB,pageNum);
    xls_parseWorkSheet(pWS);
    return pWS;
}

st_row::st_row_data* phantomCalibration::getRow(const xlsWorkSheet* pWS, const int rowNum){
    struct st_row::st_row_data* row;
    row = &pWS->rows.row[(WORD)rowNum];
    return row;
}

double phantomCalibration::getCell(xlsWorkBook* pWB, const st_row::st_row_data* row, const int column){
    double cellVal;
    struct st_cell_data* cell;
    cell = (st_cell_data*)&row->cells.cell[(WORD)column];
    cellVal = atof(xls_getfcell(pWB, cell));
    return cellVal;
}

void phantomCalibration::inputDataTg(const double cellVal){
    dataArr.push_back(make_pair(cellVal, 0));
}

void phantomCalibration::inputDataMPV(const double cellVal){
    dataArr.back().second = cellVal;
}

phantomCalibration* phantomCalibration::getThicknessData(const string fileName, const string filTar, const int kV, const double row1, const double row2){
    char* fName = const_cast<char*>(fileName.c_str());
    phantomCalibration* thicknessData;
    thicknessData = new phantomCalibration;
    int pageNum = phantomCalibration::getPageNum(filTar, kV);
    xlsWorkBook* pWB;
    pWB = xls_open(fName,"UTF-8");
    xlsWorkSheet* pWS = phantomCalibration::getParsedSheet(fName, pageNum);
    st_row::st_row_data* row;
    double cellValTg, cellValMPVmAs;
    for(int i = row1; i <= row2; i++){
        row = phantomCalibration::getRow(pWS, i);
        cellValTg = phantomCalibration::getCell(pWB, row, 2);
        cellValMPVmAs = phantomCalibration::getCell(pWB, row, 3);
        thicknessData->inputDataTg(cellValTg);
        thicknessData->inputDataMPV(cellValMPVmAs);
    }
    return thicknessData;
}

int phantomCalibration::getPageNum(const string filTar, const double kV){
    int pageNum;
    if(filTar == "MoMo" && kV == 25){
        pageNum = 0;
    } else if(filTar == "MoMo" && kV == 26){
        pageNum = 1;
    } else if(filTar == "MoMo" && kV == 27){
        pageNum = 2;
    } else if(filTar == "MoMo" && kV == 28){
        pageNum = 3;
    } else if(filTar == "MoRh" && kV == 26){
        pageNum = 4;
    } else if(filTar == "MoRh" && kV == 27){
        pageNum = 5;
    } else if(filTar == "MoRh" && kV == 28){
        pageNum = 6;
    } else if(filTar == "MoRh" && kV == 29){
        pageNum = 7;
    } else if(filTar == "RhRh" && kV == 29){
        pageNum = 8;
    } else if(filTar == "RhRh" && kV == 30){
        pageNum = 9;
    } else if(filTar == "RhRh" && kV == 31){
        pageNum = 10;
    } else{
        // logger statement
    }
    return pageNum;
}

void phantomCalibration::applyShift(const double shift){
    for(vector<pair<double, double>>::iterator it = dataArr.begin(); it != dataArr.end(); ++it)
        it->first += shift;
}

void phantomCalibration::dataCorrection(double x0, double y0){
    double* x;
    double* y;
    pair<double,double>* temp = &dataArr[0];
    int temp_size = sizeof(temp)/sizeof(*temp);
    for(int i = 0; i < temp_size; i++){
        x[i] = temp[i].first;
        y[i] = temp[i].second;
    }
    double* ret = scanner::linearfit(x,y);
    double shift = scanner::calcShift(ret, x0, y0);
    this->applyShift(shift);
}
