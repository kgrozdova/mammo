#include "scanner.h"

using namespace std;

pair<double,double> scanner::linearfit(const double* x, const double* y, const int sizeArr){
    size_t sizeArray = (size_t)sizeArr;
    double c0, c1, cov00, cov01, cov11, sumsq;
    gsl_fit_linear(x, 1, y, 1, sizeArray, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);
    pair<double,double> ret;
    ret.first = c1;
    ret.second = c0;
    return ret;
}

double scanner::calcShift(const pair<double,double> coeff, const double x0, const double y){
    double x = (y - coeff.second)/coeff.first;
    double shift = x - x0;
    return shift;
}

