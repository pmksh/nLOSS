
// BMP file header structures
#pragma pack(push, 1)  // Pack structures to 1 byte alignment
struct BMPFileHeader {
    char signature[2];     // "BM"
    uint32_t fileSize;     // Total file size
    uint16_t reserved1;    // Reserved, must be 0
    uint16_t reserved2;    // Reserved, must be 0
    uint32_t dataOffset;   // Offset to pixel data
};

struct BMPInfoHeader {
    uint32_t size;            // Size of this header (40 bytes)
    int32_t width;            // Image width
    int32_t height;           // Image height (positive = bottom-up)
    uint16_t planes;          // Must be 1
    uint16_t bitsPerPixel;    // Bits per pixel (24 for RGB)
    uint32_t compression;     // Compression type (0 = none)
    uint32_t imageSize;       // Image size (can be 0 for uncompressed)
    int32_t xPixelsPerMeter;  // Horizontal resolution
    int32_t yPixelsPerMeter;  // Vertical resolution
    uint32_t colorsUsed;      // Colors in palette (0 = all)
    uint32_t colorsImportant; // Important colors (0 = all)
};
#pragma pack(pop)

int calculateRowPadding(int width) {
        int bytesPerRow = width * 3; // 3 bytes per pixel (RGB)
        int padding = (4 - (bytesPerRow % 4)) % 4; // BMP rows must be multiple of 4 bytes
        return padding;
    }

bool loadBMP(const std::string& filename, ImageData& currentImage) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // Read BMP file header
    BMPFileHeader fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
    
    if (file.gcount() != sizeof(BMPFileHeader)) {
        std::cerr << "Error: Failed to read BMP file header" << std::endl;
        return false;
    }
    
    // Check BMP signature
    if (fileHeader.signature[0] != 'B' || fileHeader.signature[1] != 'M') {
        std::cerr << "Error: Invalid BMP file signature" << std::endl;
        return false;
    }
    
    // Read BMP info header
    BMPInfoHeader infoHeader;
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));
    
    if (file.gcount() != sizeof(BMPInfoHeader)) {
        std::cerr << "Error: Failed to read BMP info header" << std::endl;
        return false;
    }
    
    // Validate BMP format
    if (infoHeader.bitsPerPixel != 24) {
        std::cerr << "Error: Only 24-bit BMP files are supported" << std::endl;
        return false;
    }
    
    if (infoHeader.compression != 0) {
        std::cerr << "Error: Compressed BMP files are not supported" << std::endl;
        return false;
    }
    
    // Handle negative height (top-down bitmap)
    int imageHeight = abs(infoHeader.height);
    bool topDown = infoHeader.height < 0;
    
    currentImage.allocate(infoHeader.width, imageHeight);
    
    // Move to pixel data
    file.seekg(fileHeader.dataOffset, std::ios::beg);
    
    int padding = calculateRowPadding(infoHeader.width);
    
    // Read pixel data
    // BMP stores pixels as BGR (Blue, Green, Red) not RGB
    // BMP stores rows bottom-to-top unless height is negative
    for (int row = 0; row < imageHeight; row++) {
        int y = topDown ? row : (imageHeight - 1 - row);
        
        for (int x = 0; x < infoHeader.width; x++) {
            unsigned char bgr[3];
            file.read(reinterpret_cast<char*>(bgr), 3);
            
            if (file.gcount() != 3) {
                std::cerr << "Error: Failed to read pixel data" << std::endl;
                currentImage.clear();
                return false;
            }
            
            // Convert BGR to RGB
            currentImage.pixels[y][x][0] = bgr[2]; // Red
            currentImage.pixels[y][x][1] = bgr[1]; // Green
            currentImage.pixels[y][x][2] = bgr[0]; // Blue
        }
        
        // Skip padding bytes
        if (padding > 0) {
            file.seekg(padding, std::ios::cur);
        }
    }
    
    file.close();
    std::cout << "Successfully loaded BMP image: " << filename << std::endl;
    currentImage.printInfo();
    return true;
}

bool saveBMP(const std::string& filename, ImageData& currentImage) {
        if (!currentImage.isLoaded) {
            std::cerr << "Error: No image loaded to save" << std::endl;
            return false;
        }
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot create file " << filename << std::endl;
            return false;
        }
        
        int padding = calculateRowPadding(currentImage.width);
        int rowSize = currentImage.width * 3 + padding;
        int imageSize = rowSize * currentImage.height;
        int fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
        
        // Create and write file header
        BMPFileHeader fileHeader;
        fileHeader.signature[0] = 'B';
        fileHeader.signature[1] = 'M';
        fileHeader.fileSize = fileSize;
        fileHeader.reserved1 = 0;
        fileHeader.reserved2 = 0;
        fileHeader.dataOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
        
        file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BMPFileHeader));
        
        // Create and write info header
        BMPInfoHeader infoHeader;
        infoHeader.size = sizeof(BMPInfoHeader);
        infoHeader.width = currentImage.width;
        infoHeader.height = currentImage.height; // Positive = bottom-up
        infoHeader.planes = 1;
        infoHeader.bitsPerPixel = 24;
        infoHeader.compression = 0;
        infoHeader.imageSize = imageSize;
        infoHeader.xPixelsPerMeter = 2835; // 72 DPI
        infoHeader.yPixelsPerMeter = 2835; // 72 DPI
        infoHeader.colorsUsed = 0;
        infoHeader.colorsImportant = 0;
        
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
        
        // Write pixel data (bottom-to-top, BGR format)
        std::vector<unsigned char> paddingBytes(padding, 0);
        
        for (int row = currentImage.height - 1; row >= 0; row--) {
            for (int x = 0; x < currentImage.width; x++) {
                // Convert RGB to BGR
                unsigned char bgr[3] = {
                    currentImage.pixels[row][x][2], // Blue
                    currentImage.pixels[row][x][1], // Green
                    currentImage.pixels[row][x][0]  // Red
                };
                file.write(reinterpret_cast<const char*>(bgr), 3);
            }
            
            // Write padding bytes
            if (padding > 0) {
                file.write(reinterpret_cast<const char*>(paddingBytes.data()), padding);
            }
        }
        
        file.close();
        std::cout << "Successfully saved BMP image: " << filename << std::endl;
        return true;
    }