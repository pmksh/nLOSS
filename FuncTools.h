
Complex Quantize(Complex x, int s){
    return Complex(x.real()-(std::fmod(x.real() , s)), 0);
}