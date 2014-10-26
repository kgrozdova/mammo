#ifndef MAIN_HEADER_H //header guard
#define MAIN_HEADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <libxls/xls.h>
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */


using namespace std;

class mammography{
    public:
        long XRayTubeCurrent, ExposureInuAs, Rows, Columns, BitsAllocated, PixelRepresentation, SmallestImagePixelValue, LargestImagePixelValue;
        OFString BodyPartThickness, CompressionForce;
        Uint16* pixelArr;
    public:
        mammography(){
            cout << "mammography constructor is called" << endl;
        };
        ~mammography() {cout << "mammography destructor is called" << endl;}
        void loadHeaderData(const string fileName);
        void loadPixelData(const string fileName);
};

class phantomCalibration{
    public:
        vector< pair<double, double> > dataArr;
    public:
        phantomCalibration(){
            cout << "phantomCalibration constructor is called" << endl;
        };
        ~phantomCalibration() {cout << "phantomCalibration destructor is called" << endl;}
        // get parsed sheet from an .xls file
        xlsWorkSheet* getParsedSheet(const string fileName, int pageNum);
        // get the data from given row of parsed .xls fie sheet
        st_row::st_row_data* getRow(const xlsWorkSheet* pWS, const int row);
        // get the data from a given cell from a parsed .xls workbook
        double getCell(xlsWorkBook* pWB, const st_row::st_row_data* row, const int column);
        // input the fibroglandular tissue thickness value into dataArr
        void inputDataTg(const double cellVal);
        // input the MPV into dataArr
        void inputDataMPV(const double cellVal);
        // get the phantomCalibration class object with dataArr containing the data for given total thickness
        phantomCalibration* getThicknessData(const string fileName, const string filTar, const int kV, const double row1, const double row2);
        // return sheet number in .xls workbook corresponding to the kV and filter/target combination
        int getPageNum(const string filTar, const double kV);
        void applyShift(const double shift);
        void dataCorrection(double x0, double y0);
        xlsWorkBook* xls_open(char *file,char* charset);
};

class dailyCalibration{
    public:
        double qc_ln_MPV_mAs;
        double t;
        double tg;
        string filTar;
};

class scanner: public phantomCalibration, dailyCalibration{
    public:
        double shift;
    public:
        scanner(const int shiftIn): shift(shiftIn) { cout << "scanner constructor is called" << endl; };
        ~scanner() {cout << "scanner destructor is called" << endl;}
        // fit linear function into data
        static double* linearfit(const double* x, const double* y);
        // calculate shift from the QC data point
        static double calcShift(double* coeff, double x0, double y);
};

#endif // MAIN_HEADER_H
