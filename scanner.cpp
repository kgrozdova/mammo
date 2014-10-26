#include <iostream>
#include <math.h>
#include "main_header.h"
#include <gsl/gsl_fit.h>
#include <gsl/gsl_poly.h>

using namespace std;

double* scanner::linearfit(const double* x, const double* y){
    size_t sizeArr = (sizeof(x)/sizeof(*x));
    double* c0;
    double* c1;
    double* cov00;
    double* cov01;
    double* cov11;
    double* sumsq;
    gsl_fit_linear(x, 1, y, 1, sizeArr, c0, c1, cov00, cov01, cov11, sumsq);
    double ret[2] = { *c1, *c0 };
    return ret;
}

double scanner::calcShift(double* coeff, double x0, double y){
    double x = (y - coeff[1])/coeff[0];
    double shift = x - x0;
    return shift;
}
