#pragma once

#include "Commons.h"

#include <optional>
#include <vector>
#include <map>
#include <charconv>



// Try to parse string as int, return value if valid, otherwise std::nullopt
std::optional<int> toInt(const std::string& s) {
    int value{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

    if (ec == std::errc() && ptr == s.data() + s.size()) {
        return value;  // success
    }
    return std::nullopt; // failure
}


//------------------------------------------------------------------------------------------------------------------


unsigned char int_to_uchar(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<unsigned char>(value);
}

unsigned char double_to_uchar(double value) {
    if (value < 0.0) return 0;
    if (value > 255.0) return 255;
    return static_cast<unsigned char>(floor(value));
}

double uchar_to_double(unsigned char value) {
    return static_cast<double>(value);
}

unsigned char complex_to_uchar(Complex value) {

    if (value.real() < 0.0) return 0;
    if (value.real() > 255.0) return 255;
    return static_cast<unsigned char>(floor(value.real()));
}

Complex uchar_to_complex(unsigned char value) {
    return Complex(static_cast<double>(value),0);
}

//------------------------------------------------------------------------------------------------------------------


// Convert int vector to complex vector
std::vector<std::complex<double>> int_to_complex(const std::vector<int>& input) {
    std::vector<std::complex<double>> result;
    result.reserve(input.size());
    for (int val : input) {
        result.emplace_back(val, 0);
    }
    return result;
}

// Convert complex vector to int vector by taking norm and rounding down
std::vector<int> complex_to_int(const std::vector<std::complex<double>>& input) {
    std::vector<int> result;
    result.reserve(input.size());
    for (const std::complex<double>& val : input) {
        result.push_back(val.real());
    }
    return result;
}


std::vector<std::complex<double>> double_to_complex(const std::vector<double>& input) {
    std::vector<std::complex<double>> result;
    result.reserve(input.size());
    for (double val : input) {
        result.emplace_back(val, 0);
    }
    return result;
}

// Convert complex vector to double vector
std::vector<double> complex_to_double(const std::vector<std::complex<double>>& input) {
    std::vector<double> result;
    result.reserve(input.size());
    for (const std::complex<double>& val : input) {
        result.push_back(std::abs(val));
    }
    return result;
}

// -----------------------------------------------------------------------------------------------

void levelHelper(ImageData& img, int x_s, int y_s, int x_l, int y_l){

    Complex sum_0 = Complex(0,0);
    Complex sum_1 = Complex(0,0);
    Complex sum_2 = Complex(0,0);

    for (int y = 0 ; y < y_l; y++) {
        for (int x = 0 ; x < x_l; x++) {
            int y2 = y_s + y;
            int x2 = x_s + x;
            sum_0 += img.pixels[y2][x2][0];
            sum_1 += img.pixels[y2][x2][1];
            sum_2 += img.pixels[y2][x2][2];
        }
    }

    sum_0 /= Complex(x_l * y_l, 0);
    sum_1 /= Complex(x_l * y_l, 0);
    sum_2 /= Complex(x_l * y_l, 0);

    for (int y = 0 ; y < y_l; y++) {
        for (int x = 0 ; x < x_l; x++) {
            int y2 = y_s + y;
            int x2 = x_s + x;
            img.pixels[y2][x2][0] = sum_0;
            img.pixels[y2][x2][1] = sum_1;
            img.pixels[y2][x2][2] = sum_2;
        }
    }
}