#pragma once

#include <vector>
#include <string>
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
/* #include "dcmtk/dcmjpeg/djencode.h" */
/* #include "dcmtk/dcmjpeg/djrplol.h" */
#include "various.h"


class mammography: public various{
    public:
        long XRayTubeCurrent, Exposure, Rows, Columns, BitsAllocated, PixelRepresentation, SmallestImagePixelValue, LargestImagePixelValue, BitsStored;
        OFString KVP, BodyPartThickness, CompressionForce, Filter, Target, ViewPosition, ImageLaterality;
        std::vector<Uint16> pixelVec;
        std::string strFileName, strKVP;
        double numThickness, numExposure;
    public:
        mammography(std::string t_strFileName): strFileName(t_strFileName){
            this->loadHeaderData(strFileName);
            this->loadPixelData(strFileName);
            strKVP = various::ToString<OFString>(this->KVP);
            std::string strBodyThickness = various::ToString<OFString>(this->BodyPartThickness);
            std::string strExposure = various::ToString<long>(this->Exposure);
            numThickness = atoi(strBodyThickness.c_str());
            numExposure = atoi(strExposure.c_str());
        };
        ~mammography() {}
        void loadHeaderData(const std::string fileName);
        void loadPixelData(const std::string fileName);
};

