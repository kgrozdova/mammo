/*
    Sample file to show main features, which will be used from the DCMTK library
*/

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmimgle/dcmimage.h" /* for DicomImage */
using namespace std;

int main(){
    // load test file data
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile("/home/ksenia/Documents/Man_4/MPhys_mammography/Week_2/FFDM1/Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm");
    // output sample data from header into txt file
    if(status.good()){
        ofstream testfile("header_testData.txt");
        long test;
        long rows;
        long columns;
        OFString test1;
        if (testfile.is_open()){
            if (fileformat.getDataset()->findAndGetLongInt(DCM_XRayTubeCurrent, test).good()){
                testfile << "XrayTubeCurrent: " << test << endl;
            }else
                cerr << "Error: cannot access XrayTubeCurrent!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_ExposureInuAs, test).good()){
                testfile << "ExposureInuAs: " << test << endl;
            }else
                cerr << "Error: cannot access ExposureInuAs!" << endl;
            if (fileformat.getDataset()->findAndGetOFString(DCM_BodyPartThickness, test1).good()){
                testfile << "BodyPartThickness: " << test1 << endl;
            }else
                cerr << "Error: cannot access BodyPartThickness!" << endl;
            if (fileformat.getDataset()->findAndGetOFString(DCM_CompressionForce, test1).good()){
                testfile << "CompressionForce: " << test1 << endl;
            }else
                cerr << "Error: cannot access CompressionForce!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_Rows, rows).good()){
                testfile << "Rows: " << rows << endl;
            }else
                cerr << "Error: cannot access Rows!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_Columns, columns).good()){
                testfile << "Columns: " << columns << endl;
            }else
                cerr << "Error: cannot access Columns!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_BitsAllocated, test).good()){
                testfile << "BitsAllocated: " << test << endl;
            }else
                cerr << "Error: cannot access BitsAllocated!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_PixelRepresentation, test).good()){
                testfile << "PixelRepresentation: " << test << endl;
            }else
                cerr << "Error: cannot access PixelRepresentation!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_SmallestImagePixelValue, test).good()){
                testfile << "SmallestImagePixelValue: " << test << endl;
            }else
                cerr << "Error: cannot access SmallestImagePixelValue!" << endl;
            if (fileformat.getDataset()->findAndGetLongInt(DCM_LargestImagePixelValue, test).good()){
                testfile << "LargestImagePixelValue: " << test << endl;
            }else
                cerr << "Error: cannot access LargestImagePixelValue!" << endl;
        }
        testfile.close();
        // get pixel data
        const Uint16* pixelArr = NULL;
        fileformat.getDataset()->findAndGetUint16Array(DCM_PixelData, pixelArr);

    }else
        cerr << "Error: cannot read DICOM file (" << status.text() << ")" << endl;
}
