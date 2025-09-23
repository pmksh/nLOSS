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
/*
std::vector<Complex> dct2(const std::vector<Complex> a) {
    const int N = static_cast<int>(a.size());
    const int M = 2 * N;

    // Step 1: Create symmetric sequence
    std::vector<Complex> extended(M);
    for (int i = 0; i < N; i++) {
        extended[i] = a[i].real();
        extended[M - 1 - i] = a[i].real();
    }

    // Step 2: Compute DFT of extended sequence
    std::vector<Complex> A = dft(extended);

    // Step 3: Extract DCT-II values
    std::vector<Complex> result(N);
    const double factor = M_PI / (2.0 * N);

    for (int k = 0; k < N; k++) {
        Complex phase = std::exp(Complex(0.0, -factor * k));
        result[k] = (A[k] * phase).real();
    }

    return result;
}
*/


std::vector<Complex> dct2(const std::vector<Complex> a) {
    const int N = static_cast<int>(a.size());
    std::vector<Complex> result(N);

    for (int k = 0; k < N; k++) {
        double sum = 0.0;
        for (int n = 0; n < N; n++) {
            sum += a[n].real() * std::cos(PI * (n + 0.5) * k / N);
        }
        result[k] = Complex(sum, 0.0);  // result is real, but keep as Complex
    }

    return result;
}

// Inverse DCT-II (DCT-III) returning Complex values
std::vector<Complex> idct2(const std::vector<Complex> a) {
    const int N = static_cast<int>(a.size());
    std::vector<Complex> result(N);

    for (int n = 0; n < N; n++) {
        double sum = a[0].real() / 2.0;  // k=0 term is halved
        for (int k = 1; k < N; k++) {
            sum += a[k].real() * std::cos(PI * (n + 0.5) * k / N);
        }
        result[n] = Complex(2 * sum / N, 0.0);  // wrap result as Complex
    }

    return result;
}