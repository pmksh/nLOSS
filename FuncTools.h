#pragma once

#include "Commons.h"
#include "Utils.h"




// Function tools
// work in progress


Complex Quantize(Complex x, int s){
    return Complex(x.real()-(std::fmod(x.real() , s)), 0);
}

Complex Cutoff(Complex x, int s){
    if (std::abs(x) > s) return x;
    return Complex(0,0);
}


// Pixel functions

// fit to [0,255]
Triple PF_fit(Triple& a){
    a[0] = uchar_to_complex(complex_to_uchar(a[0]));
    a[1] = uchar_to_complex(complex_to_uchar(a[1]));
    a[2] = uchar_to_complex(complex_to_uchar(a[2]));
    return a;
}

// use luminance to create grayscale
Triple PF_grayscale(Triple& a){
    Complex gray =
        Complex(0.299,0) * a[0] + 
        Complex(0.587,0) * a[1] + 
        Complex(0.114,0) * a[2];
    a[0] = gray;
    a[1] = gray;
    a[2] = gray;
    return a;
}

// invert around 255
Triple PF_invert(Triple& a){
    Complex c_255 = Complex(255,0);
    a[0] = c_255 - a[0]; // Invert Red
    a[1] = c_255 - a[1]; // Invert Green
    a[2] = c_255 - a[2]; // Invert Blue
    return a;
}

// take absolute value
Triple PF_absolute(Triple& a){
    a[0] = std::abs(a[0]);
    a[1] = std::abs(a[2]);
    a[2] = std::abs(a[1]);
    return a;
}

// keep only real
Triple PF_real(Triple& a){
    a[0] = std::real(a[0]);
    a[1] = std::real(a[2]);
    a[2] = std::real(a[1]);
    return a;
}

// keep only imaginary
Triple PF_im(Triple& a){
    a[0] = a[0] - std::real(a[0]);
    a[1] = a[1] - std::real(a[2]);
    a[2] = a[2] - std::real(a[1]);
    return a;
}


// Sort tools

//sort by real
bool sort_v1(const Complex& a, const Complex& b) {
    
    if (std::real(a) == std::real(b))
        return std::arg(a) < std::arg(b);
    return std::real(a) < std::real(b);
}

