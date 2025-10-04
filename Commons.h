#pragma once

#include <complex>
#include <vector>
#include <functional>

using Complex = std::complex<double>;
using TransformFunc = std::function<std::vector<Complex>(std::vector<Complex>)>;
using SortFunc = std::function<bool(const std::complex<double>&, const std::complex<double>&)>;
using Triple = std::array<std::complex<double>, 3>;
using PixelFunc = std::function<Triple(Triple&)>;
const double PI = acos(-1.0);