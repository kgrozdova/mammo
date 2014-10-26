#include "main_header.h"

int main(){
    mammography mammData;
    mammData.mammography::loadHeaderData("/home/ksenia/Documents/Man_4/MPhys_mammography/Week_2/FFDM1/Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm");
    mammData.mammography::loadPixelData("/home/ksenia/Documents/Man_4/MPhys_mammography/Week_2/FFDM1/Control_prior_anon-00001-LMLO111105-20110328-RAW.dcm");
    int kV = (int)mammData.XRayTubeCurrent;
    dailyCalibration dcalib;
    dcalib.qc_ln_MPV_mAs = 3.5;
    dcalib.tg = 5;
    if(dcalib.filTar == "MoMo"){
        phantomCalibration* t20_MoMo;
        t20_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 2, 6);
        t20_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t30_MoMo;
        t30_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 7, 13);
        t30_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t40_MoMo;
        t40_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 14, 20);
        t40_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t50_MoMo;
        t50_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 21, 23);
        t50_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t60_MoMo;
        t60_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 24, 26);
        t60_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t70_MoMo;
        t70_MoMo->getThicknessData("av_calib_data.xls", "MoMo", kV, 27, 29);
        t70_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
    }
    if(dcalib.filTar == "MoRh"){
        phantomCalibration* t20_MoMo;
        t20_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 2, 6);
        t20_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t30_MoMo;
        t30_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 7, 13);
        t30_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t40_MoMo;
        t40_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 14, 20);
        t40_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t50_MoMo;
        t50_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 21, 25);
        t50_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t60_MoMo;
        t60_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 26, 30);
        t60_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t70_MoMo;
        t70_MoMo->getThicknessData("av_calib_data.xls", "MoRh", kV, 31, 31);
        t70_MoMo->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
    }
    if(dcalib.filTar == "MoRh"){
        phantomCalibration* t20_MoRh;
        t20_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 2, 6);
        t20_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t30_MoRh;
        t30_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 7, 13);
        phantomCalibration* t40_MoRh;
        t30_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        t40_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 14, 20);
        phantomCalibration* t50_MoRh;
        t40_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        t50_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 21, 25);
        phantomCalibration* t60_MoRh;
        t50_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        t60_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 26, 30);
        phantomCalibration* t70_MoRh;
        t60_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        t70_MoRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 31, 31);
        t70_MoRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
    }
    if(dcalib.filTar == "RhRh" && kV == 29){
        phantomCalibration* t05_RhRh;
        t05_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 2, 3);
        t05_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t10_RhRh;
        t10_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 4, 6);
        t10_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t20_RhRh;
        t20_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 7, 11);
        t20_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t30_RhRh;
        t30_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 12, 18);
        t30_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t40_RhRh;
        t40_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 19, 27);
        t40_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t50_RhRh;
        t50_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 28, 38);
        t50_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t60_RhRh;
        t60_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 39, 51);
        t60_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t65_RhRh;
        t65_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 52, 65);
        t65_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t70_RhRh;
        t70_RhRh->getThicknessData("av_calib_data.xls", "RhRh", kV, 66, 80);
        t70_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
    } else if(dcalib.filTar == "RhRh"){
        phantomCalibration* t20_RhRh;
        t20_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 2, 4);
        t20_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t30_RhRh;
        t30_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 5, 7);
        t30_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t40_RhRh;
        t40_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 8, 14);
        t40_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t50_RhRh;
        t50_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 15, 19);
        t50_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t60_RhRh;
        t60_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 20, 22);
        t60_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
        phantomCalibration* t70_RhRh;
        t70_RhRh->getThicknessData("av_calib_data.xls", "MoRh", kV, 23, 25);
        t70_RhRh->dataCorrection(dcalib.tg, dcalib.qc_ln_MPV_mAs);
    }
}
