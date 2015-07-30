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
        std::string filTar;
        void insertFilTar(mammography mammData);
        void InserQcTTg(mammography mammData, const std::string fileName);
};


