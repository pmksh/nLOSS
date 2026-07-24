#pragma once

#include <complex>
#include <vector>
#include <functional>

using Complex           =       std::complex<double>;
using Triple            =       std::array<std::complex<double>, 3>;

using TransformFunc     =       std::function<std::vector<Complex>(std::vector<Complex>)>;
using SortFunc          =       std::function<bool(const std::complex<double>&, const std::complex<double>&)>;
using PixelFunc         =       std::function<Triple(Triple&)>;
using PixelFuncComplex  =       std::function<Triple(Triple&, Complex&)>;
using WarpFunc          =       std::function<std::pair<double, double>(double, double)>;

const double PI = acos(-1.0);


struct frame{
    int x;
    int y;
    int x_size;
    int y_size;
};

struct pixel{
    Complex r;
    Complex g;
    Complex b;
};