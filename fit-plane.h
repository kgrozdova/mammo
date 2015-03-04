#ifndef INCLUDED_FIT_PLANE_H
#define INCLUDED_FIT_PLANE_H

#include <tuple>

template<class PointType ,class DataType>
std::tuple< PointType , PointType , PointType > fitPlane(DataType data)
{
    PointType A = 0,B = 0,C = 0,D = 0,E = 0,F = 0,G = 0,H = 0;
    int n = data.size();
        
    for(auto datum:data)
    {
        PointType x = std::get<0>(datum);
        PointType y = std::get<1>(datum);
        PointType z = std::get<2>(datum);
        
        A += x * x;
        B += y * y;
        C += x * y;
        D += x * z;
        E += y * z;
        F += x;
        G += y;
        H += z;
            
    }

    PointType G2mnB = G * G - n * B;
    PointType GHmnE = G * H - n * E;
    PointType EGmBH = E * G - B * H;
    PointType nC = n * C;
    PointType CG = C * G;
    PointType CH = C * H;
    
    PointType denominator = A * G2mnB - 2 * CG * F + B * F * F + nC * C;
    
    PointType a = -(C * GHmnE + F * EGmBH - D * G2mnB) / denominator;
    PointType b = ( A * GHmnE - F * ( CH + D * G) + E * F * F + nC * D ) / denominator;
    PointType c = ( A * EGmBH + CH * C - CG * D + ( B * D - C * E) * F ) / denominator;
    
    return std::make_tuple( a,b,c );

}

#endif
