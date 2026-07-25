#include "FilterTools.h"

// x, y are in [0,1]

Complex Filter_radius(double x, double y){
    return Complex (1 - (x*x + y*y)/2, 0);
}