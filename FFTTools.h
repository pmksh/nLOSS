// Transform tools


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

    for (auto& x : a) {
        x = std::conj(x);
    }
    
    a = fft(a);

    for (auto& x : a) {
        x = std::conj(x) / double(original_n);
    }

    return a;
}

std::vector<Complex> dft(std::vector<Complex> a) {
    int N = a.size();
    std::vector<std::complex<double>> X(N);
    
    
    for (int k = 0; k < N; k++) {
        X[k] = std::complex<double>(0.0, 0.0);
        
        for (int n = 0; n < N; n++) {
            // Calculate the complex exponential: e^(-2πi*k*n/N)
            double angle = -2.0 * M_PI * k * n / N;
            std::complex<double> w = std::complex<double>(std::cos(angle), std::sin(angle));
            
            X[k] += a[n] * w;
        }
    }
    
    return X;
}

std::vector<Complex> idft(std::vector<Complex> a) {
    int N = a.size();
    
    for (auto& x : a) {
        x = std::conj(x);
    }

    a = dft(a);

    for (auto& x : a) {
        x = std::conj(x) / double(N);
    }

    return a;
}

