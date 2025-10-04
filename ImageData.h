// Main structure to hold image data
// Image is stored as complex [height][width][RGB] array
// Is automatically cast to unsigned cgar when saving

#include "Commons.h"

#include <iostream>
#include <vector>

struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<std::vector<std::vector<Complex>>> pixels; // [height][width][RGB]
    bool isLoaded = false;
    
    void allocate(int w, int h);
    
    void clear();
    
    void printInfo() const;
};