#include "ImageData.h"

void ImageData::allocate(int w, int h) {
    width = w;
    height = h;
    pixels = std::vector<std::vector<std::vector<Complex>>>(
        height, 
        std::vector<std::vector<Complex>>(
            width, 
            std::vector<Complex>(3, 0) // RGB values
        )
    );
    isLoaded = true;
}

void ImageData::clear() {
    pixels.clear();
    width = height = 0;
    isLoaded = false;
}

void ImageData::printInfo() const {
    if (isLoaded) {
        std::cout << "Image loaded: " << width << "x" << height << " pixels" << std::endl;
    } else {
        std::cout << "No image loaded" << std::endl;
    }
}
