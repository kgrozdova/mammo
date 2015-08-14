#pragma once

#include <cmath>
#include <string>
#include <sstream>

#include "main_header.h"

class dailyCalibration {
    public:
        double qc_ln_MPV_mAs;
        double t;
        double tg;
        std::string filTarQC;
        /* SET FILTER/TARGET COMBINATION VARIABLE */
        void insertFilTar(mammography mammData);
        /* LOAD DAILY CALIBRATION DATA */
        void InserQcTTg(mammography mammData, const std::string fileName);
};


