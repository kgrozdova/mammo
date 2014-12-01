#include "mammography.h"

void mammography::loadHeaderData(const std::string fileName){
     DcmFileFormat fileformat;
     const char* fName = fileName.c_str();
     OFCondition status = fileformat.loadFile(fName);
     if(status.good()){
        if (fileformat.getDataset()->findAndGetLongInt(DCM_XRayTubeCurrent, XRayTubeCurrent).good()){
            // log
        }else
            std::cerr << "Error: cannot access XRayTubeCurrent!" << std::endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_KVP, KVP).good()){
        }else
            std::cerr << "Error: cannot access KVP!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Exposure, Exposure).good()){
        }else
            std::cerr << "Error: cannot access Exposure!" << std::endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_BodyPartThickness, BodyPartThickness).good()){
        }else
            std::cerr << "Error: cannot access BodyPartThickness!" << std::endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_CompressionForce, CompressionForce).good()){
        }else
            std::cerr << "Error: cannot access CompressionForce!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Rows, Rows).good()){
        }else
            std::cerr << "Error: cannot access Rows!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_Columns, Columns).good()){
        }else
            std::cerr << "Error: cannot access Columns!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_BitsAllocated, BitsAllocated).good()){
        }else
            std::cerr << "Error: cannot access BitsAllocated!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_PixelRepresentation, PixelRepresentation).good()){
        }else
            std::cerr << "Error: cannot access PixelRepresentation!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_SmallestImagePixelValue, SmallestImagePixelValue).good()){
        }else
            std::cerr << "Error: cannot access SmallestImagePixelValue!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_LargestImagePixelValue, LargestImagePixelValue).good()){
        }else
            std::cerr << "Error: cannot access LargestImagePixelValue!" << std::endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_FilterMaterial, Filter).good()){
        }else
            std::cerr << "Error: cannot access FilterMaterial!" << std::endl;
        if (fileformat.getDataset()->findAndGetOFString(DCM_AnodeTargetMaterial, Target).good()){
        }else
            std::cerr << "Error: cannot access AnodeTargetMaterial!" << std::endl;
        if (fileformat.getDataset()->findAndGetLongInt(DCM_BitsStored, BitsStored).good()){
        }else
            std::cerr << "Error: cannot access BitsStored!" << std::endl;
     }
}

void mammography::loadPixelData(const std::string fileName){
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
        std::cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << std::endl;
    }
    delete image;
}
