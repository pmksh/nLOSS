#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

using Complex = std::complex<double>;
const double PI = std::acos(-1.0);

static void fft(std::vector<Complex>& a) {
    int original_n = a.size();
    if (original_n <= 1) return;
    
    // Find next power of 2
    int n = 1;
    while (n < original_n) {
        n <<= 1;  // n *= 2
    }
    
    // Zero-pad to power of 2 if necessary
    if (n != original_n) {
        a.resize(n, Complex(0, 0));
    }
    
    // Bit-reverse permutation
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    
    // Iterative FFT
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * PI / len;
        Complex wlen(std::cos(angle), std::sin(angle));
        
        for (int i = 0; i < n; i += len) {
            Complex w(1);
            for (int j = 0; j < len / 2; j++) {
                Complex u = a[i + j];
                Complex v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    
    // Truncate back to original size
    if (n != original_n) {
        a.resize(original_n);
    }
}

// Inverse FFT with zero padding
static void ifft(std::vector<Complex>& a) {
    int original_n = a.size();
    
    // Conjugate input
    for (auto& x : a) {
        x = std::conj(x);
    }
    
    // Forward FFT
    fft(a);
    
    // Conjugate and scale result
    for (auto& x : a) {
        x = std::conj(x) / double(original_n);  // Scale by original size, not padded size
    }
}

// Convert int vector to complex vector
std::vector<Complex> int_to_complex(const std::vector<int>& input) {
    std::vector<Complex> result;
    result.reserve(input.size());
    for (int val : input) {
        result.emplace_back(val, 0);
    }
    return result;
}

// Convert complex vector to int vector by taking norm and rounding down
std::vector<int> complex_to_int(const std::vector<Complex>& input) {
    std::vector<int> result;
    result.reserve(input.size());
    for (const Complex& val : input) {
        result.push_back(static_cast<int>(std::abs(val)));
    }
    return result;
}
