#include "Commons.h"

#include <optional>
#include <vector>
#include <map>
#include <charconv>


class ImageData; 



// Try to parse string as int, return value if valid, otherwise std::nullopt
std::optional<int> toInt(const std::string& s);


//------------------------------------------------------------------------------------------------------------------


unsigned char int_to_uchar(int value);

unsigned char double_to_uchar(double value);

double uchar_to_double(unsigned char value);

unsigned char complex_to_uchar(Complex value);

Complex uchar_to_complex(unsigned char value);

//------------------------------------------------------------------------------------------------------------------


// Convert int vector to complex vector
std::vector<std::complex<double>> int_to_complex(const std::vector<int>& input);

// Convert complex vector to int vector by taking norm and rounding down
std::vector<int> complex_to_int(const std::vector<std::complex<double>>& input);

std::vector<std::complex<double>> double_to_complex(const std::vector<double>& input);

// Convert complex vector to double vector
std::vector<double> complex_to_double(const std::vector<std::complex<double>>& input);


// -----------------------------------------------------------------------------------------------

void levelHelper(ImageData& img, int x_s, int y_s, int x_l, int y_l);


// parse pair
bool parsePair(const std::string& str, double& a, double& b);