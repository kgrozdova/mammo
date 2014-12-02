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
        OFString KVP, BodyPartThickness, CompressionForce, Filter, Target;
	std::vector<Uint16> pixelVec;
	std::string strFileName;
    public:
        mammography(std::string t_strFileName): strFileName(t_strFileName){
	    this->loadHeaderData(strFileName);
	    this->loadPixelData(strFileName);
        };
        ~mammography() {}
        void loadHeaderData(const std::string fileName);
        void loadPixelData(const std::string fileName);
};

