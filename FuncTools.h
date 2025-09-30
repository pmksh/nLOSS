// Function tools
// work in progress
Complex Quantize(Complex x, int s){
    return Complex(x.real()-(std::fmod(x.real() , s)), 0);
}

Complex Cutoff(Complex x, int s){
    if (abs(x) > s) return x;
    return Complex(0,0);
}

