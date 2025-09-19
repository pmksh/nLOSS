
using Complex = std::complex<double>;
const double PI = acos(-1.0);



std::vector<Complex> fft(std::vector<Complex> a) {
    int original_n = a.size();
    if (original_n <= 1) return a;
    
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

    return a;
}

// Inverse FFT with zero padding
std::vector<Complex> ifft(std::vector<Complex> a) {
    int original_n = a.size();
    
    // Conjugate input
    for (auto& x : a) {
        x = std::conj(x);
    }
    
    // Forward FFT
    a = fft(a);
    
    // Conjugate and scale result
    for (auto& x : a) {
        x = std::conj(x) / double(original_n);  // Scale by original size, not padded size
    }

    return a;
}

std::vector<Complex> dft(std::vector<Complex> a) {
    int N = a.size();
    std::vector<std::complex<double>> X(N);
    
    // For each output frequency bin k
    for (int k = 0; k < N; k++) {
        X[k] = std::complex<double>(0.0, 0.0);
        
        // Sum over all input samples n
        for (int n = 0; n < N; n++) {
            // Calculate the complex exponential: e^(-2πi*k*n/N)
            double angle = -2.0 * M_PI * k * n / N;
            std::complex<double> w = std::complex<double>(std::cos(angle), std::sin(angle));
            
            // Accumulate the DFT sum
            X[k] += a[n] * w;
        }
    }
    
    return X;
}

std::vector<Complex> idft(std::vector<Complex> a) {
    int N = a.size();
    
    // Conjugate input
    for (auto& x : a) {
        x = std::conj(x);
    }

    a = dft(a);

    for (auto& x : a) {
        x = std::conj(x) / double(N);
    }

    return a;
}

/*
std::vector<unsigned char> dct(std::vector<std::complex<double>> a) {
    int N = a.size();
    
    // Create extended sequence for DCT-II using DFT
    std::vector<std::complex<double>> x(2 * N);
    
    // Fill the extended sequence: x[n] = a[n] for n=0..N-1, x[n] = a[2N-1-n] for n=N..2N-1
    for (int n = 0; n < N; n++) {
        x[n] = std::complex<double>(a[n], 0.0);
        x[2 * N - 1 - n] = std::complex<double>(a[n], 0.0);
    }
    
    // Compute DFT of the extended sequence
    std::vector<std::complex<double>> X = dft(x);
    
    // Extract DCT coefficients and convert to unsigned char
    std::vector<unsigned char> y(N);
    for (int k = 0; k < N; k++) {
        // DCT-II coefficient: Y[k] = 2 * Re(X[k] * e^(-iπk/2N))
        double angle = -M_PI * k / (2.0 * N);
        std::complex<double> w = std::complex<double>(std::cos(angle), std::sin(angle));
        std::complex<double> temp = X[k] * w;
        double dct_val = 2.0 * temp.real();
        
        // Apply scaling factor for k=0
        if (k == 0) {
            dct_val *= 0.5;
        }
        
        // Convert to unsigned char with clamping
        if (dct_val < 0.0) dct_val = 0.0;
        if (dct_val > 255.0) dct_val = 255.0;
        y[k] = static_cast<unsigned char>(dct_val + 0.5); // Round to nearest integer
    }
    
    return y;
}
*/

// Convert unsigned char vector to complex vector
std::vector<std::complex<double>> uchar_to_complex(const std::vector<unsigned char>& input) {
    std::vector<std::complex<double>> result;
    result.reserve(input.size());
    for (unsigned char val : input) {
        result.emplace_back(val, 0);
    }
    return result;
}

// Convert complex vector to unsigned char vector by taking norm and rounding down
std::vector<unsigned char> complex_to_uchar(const std::vector<std::complex<double>>& input) {
    std::vector<unsigned char> result;
    result.reserve(input.size());
    for (const std::complex<double>& val : input) {
        result.push_back(static_cast<unsigned char>(val.real()));
    }
    return result;
}

