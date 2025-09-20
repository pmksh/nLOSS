// Simple structure to hold image data


struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<std::vector<std::vector<Complex>>> pixels; // [height][width][RGB]
    bool isLoaded = false;
    
    void allocate(int w, int h) {
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
    
    void clear() {
        pixels.clear();
        width = height = 0;
        isLoaded = false;
    }
    
    void printInfo() const {
        if (isLoaded) {
            std::cout << "Image loaded: " << width << "x" << height << " pixels" << std::endl;
        } else {
            std::cout << "No image loaded" << std::endl;
        }
    }
};