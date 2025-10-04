#include "Commons.h"

#include <vector>

// Transform tools



// FFT with zero padding O(nlog(n))
std::vector<Complex> fft(std::vector<Complex> a);

// Inverse FFT with zero padding O(nlog(n))
std::vector<Complex> ifft(std::vector<Complex> a);


// DFT implementation O(n^2)
std::vector<Complex> dft(std::vector<Complex> a);

// Inverse DFT implementation O(n^2)
std::vector<Complex> idft(std::vector<Complex> a);

// DCT-II implementation O(n^2)
std::vector<Complex> dct2(const std::vector<Complex> a);

// Inverse DCT-II (DCT-III) O(n^2)
std::vector<Complex> idct2(const std::vector<Complex> a);


// DST-II implementation O(n^2)
std::vector<Complex> dst2(const std::vector<Complex> a);


// Inverse DST-II (DST-III) O(n^2)
std::vector<Complex> idst2(const std::vector<Complex> a);

// WHT Implementation O(nlog(n))
std::vector<Complex> wht(std::vector<Complex> a);

// Inverse WHT Implementation O(nlog(n))
std::vector<Complex> iwht(std::vector<Complex> a);
