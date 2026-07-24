#include "Commons.h"


// Function tools
// work in progress


Complex Quantize(Complex x, int s);


Complex Cutoff(Complex x, int s);


// Pixel functions

// fit to [0,255]
Triple PF_fit(Triple& a);


// use luminance to create grayscale
Triple PF_grayscale(Triple& a);


// invert around 255
Triple PF_invert(Triple& a);

// take absolute value
Triple PF_absolute(Triple& a);

// keep only real
Triple PF_real(Triple& a);


// keep only imaginary
Triple PF_im(Triple& a);


// square each
Triple PF_square(Triple& a);


// multiply by constant
Triple PFC_mult(Triple& a, Complex& c);


// divide by constant
Triple PFC_div(Triple& a, Complex& c);


// add constant
Triple PFC_add(Triple& a, Complex& c);


// Sort tools

//sort by real
bool sort_v1(const Complex& a, const Complex& b);



// warp tools

std::pair<double, double> warp_square(double x, double y);

std::pair<double, double> warp_sqrt(double x, double y);
