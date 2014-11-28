#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
//#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "main_header.h"

void mammography::loadHeaderData(const string fileName){
     DcmFileFormat fileformat;
     const char* fName = fileName.c_str();
     OFCondition status = fileformat.loadFile(fName);
     if(status.good()){
        if (fileformat.getDataset()->findAndGetLongInt(DCM_XRayTubeCurrent, XRayTubeCurrent).good()){
            // log
        }else
            cerr << "Error: cannot access XRayTubeCurrent!" << endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_KVP, KVP).good()){
        }else
            cerr << "Error: cannot access KVP!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Exposure, Exposure).good()){
        }else
            cerr << "Error: cannot access Exposure!" << endl;
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
        if (fileformat.getDataset()->findAndGetOFString(DCM_FilterMaterial, Filter).good()){
        }else
            cerr << "Error: cannot access FilterMaterial!" << endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_AnodeTargetMaterial, Target).good()){
        }else
            cerr << "Error: cannot access AnodeTargetMaterial!" << endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_BitsStored, BitsStored).good()){
        }else
            cerr << "Error: cannot access BitsStored!" << endl;
     }
}

void mammography::loadPixelData(const string fileName){
    char strIn[1024];
    strncpy(strIn, fileName.c_str(), sizeof(strIn));
    strIn[sizeof(strIn) - 1] = 0;
    DicomImage *image = new DicomImage(strIn);
    if(image != NULL){
      if (image->getStatus() == EIS_Normal){
        if (image->isMonochrome()){
          image->setMinMaxWindow();
        const DiPixel* dipix = 0;
        void* pixels = 0;
        if(image){
            dipix = image->getInterData();
            pixels = (const_cast<DiPixel*>(dipix))->getDataPtr();
        }
        Uint16 *pixelData = (Uint16*)pixels;
          for(int i  = 0; i < int(image->getHeight())*int(image->getWidth()); i++){
            pixelVec.push_back(pixelData[i]);
            }
        }
      } else
        cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << endl;
    }
    delete image;
}
