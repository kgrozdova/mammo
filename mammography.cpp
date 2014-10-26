#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
#include "main_header.h"

void mammography::loadHeaderData(const string fileName){
     DcmFileFormat fileformat;
     const char* fName = fileName.c_str();
     OFCondition status = fileformat.loadFile(fName);
     if(status.good()){
        if (fileformat.getDataset()->findAndGetLongInt(DCM_XRayTubeCurrent, XRayTubeCurrent).good()){
            // log
        }else
            cerr << "Error: cannot access XrayTubeCurrent!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_ExposureInuAs, ExposureInuAs).good()){
        }else
            cerr << "Error: cannot access ExposureInuAs!" << endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_BodyPartThickness, BodyPartThickness).good()){
        }else
            cerr << "Error: cannot access BodyPartThickness!" << endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_CompressionForce, CompressionForce).good()){
        }else
            cerr << "Error: cannot access CompressionForce!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Rows, Rows).good()){
        }else
            cerr << "Error: cannot access Rows!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Columns, Columns).good()){
        }else
            cerr << "Error: cannot access Columns!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_BitsAllocated, BitsAllocated).good()){
        }else
            cerr << "Error: cannot access BitsAllocated!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_PixelRepresentation, PixelRepresentation).good()){
        }else
            cerr << "Error: cannot access PixelRepresentation!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_SmallestImagePixelValue, SmallestImagePixelValue).good()){
        }else
            cerr << "Error: cannot access SmallestImagePixelValue!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_LargestImagePixelValue, LargestImagePixelValue).good()){
        }else
            cerr << "Error: cannot access LargestImagePixelValue!" << endl;
     }
}

void mammography::loadPixelData(const string fileName){
    DcmFileFormat fileformat;
    const char* fName = fileName.c_str();
    OFCondition status = fileformat.loadFile(fName);
    fileformat.getDataset()->findAndGetUint16Array(DCM_PixelData, (const Uint16*&)pixelArr);
}
