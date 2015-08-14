#pragma once

#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <map>

typedef unsigned int NxU32;
typedef int NxI32;
typedef float REAL;

const REAL BF_PI = 3.1415926535897932384626433832795028841971693993751f;
const REAL BF_DEG_TO_RAD = ((2.0f * BF_PI) / 360.0f);
const REAL BF_RAD_TO_DEG = (360.0f / (2.0f * BF_PI));

class various{
    public:
        template <typename T> static std::string ToString(T input);
        template <typename T> static double ToDouble(T input);
        static void iterateVectorToFile(std::vector<std::pair<double, double>> vectorInput, std::string fileName);
        static std::string fileNameErase(std::string fileName);
        static std::map<int, std::vector<std::pair<double,double>>> getThicknessDataFound(std::map<int, std::vector<std::pair<double,double>>> rawData, const int i, const std::string filTar, const int kV);

        static void bf_getTranslation(const REAL *matrix,REAL *t);
        static void bf_matrixToQuat(const REAL *matrix,REAL *quat);
        static void bf_matrixMultiply(const REAL *pA,const REAL *pB,REAL *pM);
        static void bf_eulerToQuat(REAL roll,REAL pitch,REAL yaw,REAL *quat);
        static void bf_eulerToQuat(const REAL *euler,REAL *quat);
        static void bf_setTranslation(const REAL *translation,REAL *matrix);
        static void bf_transform(const REAL matrix[16],const REAL v[3],REAL t[3]);
        static REAL bf_dot(const REAL *p1,const REAL *p2);
        static void bf_cross(REAL *cross,const REAL *a,const REAL *b);
        static void bf_quatToMatrix(const REAL *quat,REAL *matrix);
        static void bf_rotationArc(const REAL *v0,const REAL *v1,REAL *quat);
        static void bf_planeToMatrix(const REAL *plane,REAL *matrix);
        static void bf_inverseRT(const REAL matrix[16],const REAL pos[3],REAL t[3]);
        static void bf_rotate(const REAL matrix[16],const REAL v[3],REAL t[3]);
        static void bf_computeOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *matrix);
        static bool bf_computeBestFitPlane(NxU32 vcount, const REAL *points, NxU32 vstride, const REAL *weights, NxU32 wstride, REAL *plane);
        static void bf_computeBestFitOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *matrix,bool bruteForce);
        static void bf_computeBestFitOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *pos,REAL *quat,bool bruteForce);
};

