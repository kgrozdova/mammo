#pragma once

#include <iostream>
#include <cmath>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_poly.h>
#include <utility>

#include "phantomCalibration.h"
#include "dailyCalibration.h"

class scanner{
    public:
        double shift;
    public:
        scanner(const int shiftIn): shift(shiftIn) { };
        ~scanner() {}
        // fit linear function into data
        static std::pair<double,double> linearfit(const double* x1, const double* y1, const int sizeArr);
        // calculate shift from the QC data point
        static double calcShift(const std::pair<double,double> coeff, const double x0, const double y);
};


