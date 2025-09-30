#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <memory>
#include <cstring>
#include <charconv>  // from_chars
#include <optional>  // for std::optional

#include <complex>
#include <cmath>
#include <algorithm>

// some useful naming shortcuts
using Complex = std::complex<double>;
using TransformFunc = std::vector<Complex>(*)(std::vector<Complex>);
const double PI = acos(-1.0);

// headers
#include "ImageData.h"
#include "Utils.h"
#include "BmpTools.h"
#include "FFTTools.h"
#include "FuncTools.h"

class CLI {
private:
    struct Command {
        std::function<void(const std::vector<std::string>&)> handler;
        std::string description;
        std::string usage;
        std::string flags;
    };
    
    std::map<std::string, Command> commands;    // Array of commands
    bool running;                               // Is running
    ImageData currentImage[16];                 // Store the current loaded image
    
    // Command handlers

    // Handles loading .bmp files
    void handleLoad(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cerr << "Error: Please specify a filename to load" << std::endl;
            std::cout << "Usage: load <filename.bmp>" << std::endl;
            return;
        }
        
        std::string filename = args[0];
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        
        // Clear any existing image
        img.clear();
        
        // Try to load the image
        if (!loadBMP(filename, img)) {
            std::cerr << "Failed to load BMP image: " << filename << std::endl;
        }
    }
    
    // handles saving .bmp files
    void handleSave(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cerr << "Error: Please specify a filename to save" << std::endl;
            std::cout << "Usage: save <filename.bmp>" << std::endl;
            return;
        }
        
        std::string filename = args[0];
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        
        if (!saveBMP(filename, img)) {
            std::cerr << "Failed to save BMP image: " << filename << std::endl;
        }
    }
    
    // Exit the Program
    void handleExit(const std::vector<std::string>& args) {
        std::cout << "Goodbye!" << std::endl;
        running = false;
    }
    
    // Usage guide 
    void handleHelp(const std::vector<std::string>& args) {
        std::cout << "Available commands:" << std::endl;
        for (const auto& [name, cmd] : commands) {
            std::cout << "  " << name << " - " << cmd.description << std::endl;
            if (!cmd.usage.empty()) {
                std::cout << "    Usage: " << cmd.usage << std::endl;
                std::cout << "    Flags: " << cmd.flags << std::endl;
                std::cout << std::endl;
            }
        }
        std::cout << "\nFlag guide:" << std::endl;
        std::cout << "-n :: choose one of 16 (0 - 15) image slots for command (default = 0)" << std::endl;
        std::cout << "-s :: input mandatory size parameter for command (no default)" << std::endl;
        std::cout << "-sx :: input x-size parameter for command (default = img.width)" << std::endl;
        std::cout << "-sy :: input y-size parameter for command (default = img.height)" << std::endl;

        std::cout << "\nSupported format: 24-bit uncompressed BMP files" << std::endl;
        std::cout << "Image is lept as complex matrix, automatically cast into 8 bit integers when saving" << std::endl;
    }
    
    void handleInfo(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        img.printInfo();
        if (img.isLoaded) {
            // Calculate some basic statistics
            long long totalR = 0, totalG = 0, totalB = 0;
            int totalPixels = img.width * img.height;
            
            for (int y = 0; y < img.height; y++) {
                for (int x = 0; x < img.width; x++) {
                    totalR += img.pixels[y][x][0].real();
                    totalG += img.pixels[y][x][1].real();
                    totalB += img.pixels[y][x][2].real();
                }
            }
            
            std::cout << "Average RGB values: (" 
                     << totalR / totalPixels << ", "
                     << totalG / totalPixels << ", "
                     << totalB / totalPixels << ")" << std::endl;
                     
            // Show BMP-specific information
            int padding = calculateRowPadding(img.width);
            int rowSize = img.width * 3 + padding;
            int imageDataSize = rowSize * img.height;
            int totalFileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageDataSize;
            
            std::cout << "BMP format details:" << std::endl;
            std::cout << "  Row padding: " << padding << " bytes" << std::endl;
            std::cout << "  Row size: " << rowSize << " bytes" << std::endl;
            std::cout << "  Image data size: " << imageDataSize << " bytes" << std::endl;
            std::cout << "  Total file size: " << totalFileSize << " bytes" << std::endl;
        }
    }

    // invert image
    void handleInvert(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        
        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }
        
        Complex c_255 = Complex(255,0);
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img.pixels[y][x][0] = c_255 - img.pixels[y][x][0]; // Invert Red
                img.pixels[y][x][1] = c_255 - img.pixels[y][x][1]; // Invert Green
                img.pixels[y][x][2] = c_255 - img.pixels[y][x][2]; // Invert Blue
            }
        }
        
        std::cout << "Image colors inverted" << std::endl;
    }
    
    // Grayscale image
    void handleGrayscale(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                // Calculate luminance
                Complex gray =
                    Complex(0.299,0) * img.pixels[y][x][0] + 
                    Complex(0.587,0) * img.pixels[y][x][1] + 
                    Complex(0.114,0) * img.pixels[y][x][2];
                img.pixels[y][x][0] = gray;
                img.pixels[y][x][1] = gray;
                img.pixels[y][x][2] = gray;
            }
        }
        
        std::cout << "Image converted to grayscale" << std::endl;
    }
    
    // Flip
    void handleFlip(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }
        
        std::string direction = "horizontal";
        if (!args.empty()) {
            direction = args[0];
        }
        
        if (direction == "horizontal" || direction == "h") {
            // Flip horizontally
            for (int y = 0; y < img.height; y++) {
                for (int x = 0; x < img.width / 2; x++) {
                    // Swap pixels
                    std::swap(img.pixels[y][x], img.pixels[y][img.width - 1 - x]);
                }
            }
            std::cout << "Image flipped horizontally" << std::endl;
        } else if (direction == "vertical" || direction == "v") {
            // Flip vertically
            for (int y = 0; y < img.height / 2; y++) {
                std::swap(img.pixels[y], img.pixels[img.height - 1 - y]);
            }
            std::cout << "Image flipped vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'horizontal' or 'vertical'" << std::endl;
        }
    }

    // Take abslute value
    void handleAbs(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img.pixels[y][x][0] = Complex(std::abs(img.pixels[y][x][0]),0);
                img.pixels[y][x][1] = Complex(std::abs(img.pixels[y][x][1]),0);
                img.pixels[y][x][2] = Complex(std::abs(img.pixels[y][x][2]),0);
            }
        }
        
        std::cout << "Taken absolute value" << std::endl;
    }

    // Quantize
    void handleQuantize(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", true}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        int s = catches["-s"];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        if(s == 0) {
            std::cerr << "Error: Size not given" << std::endl;
            return;
        }
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img.pixels[y][x][0] = Quantize(img.pixels[y][x][0], s);
                img.pixels[y][x][1] = Quantize(img.pixels[y][x][1], s);
                img.pixels[y][x][2] = Quantize(img.pixels[y][x][2], s);
            }
        }
        
        std::cout << "Quantized" << std::endl;
    }

    // Apply Cutoff
    void handleCutoff(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", true}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        int s = catches["-s"];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        if(s == 0) {
            std::cerr << "Error: Size not given" << std::endl;
            return;
        }
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img.pixels[y][x][0] = Cutoff(img.pixels[y][x][0], s);
                img.pixels[y][x][1] = Cutoff(img.pixels[y][x][1], s);
                img.pixels[y][x][2] = Cutoff(img.pixels[y][x][2], s);
            }
        }
        
        std::cout << "Cutoff Applied" << std::endl;
    }

    void handleFilter(const std::vector<std::string>& args, Complex (* filter)(double, double)) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", true}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        int s = catches["-s"];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        if(s == 0) {
            std::cerr << "Error: Size not given" << std::endl;
            return;
        }
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                img.pixels[y][x][0] = img.pixels[y][x][0] * filter(x/img.height, y/(img.height));
                img.pixels[y][x][1] = img.pixels[y][x][1] * filter(x/img.height, y/(img.height));
                img.pixels[y][x][2] = img.pixels[y][x][2] * filter(x/img.height, y/(img.height));
            }
        }
        
        std::cout << "Cutoff Applied" << std::endl;
    }

    void handleLevel(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int tx = img.width / sx;
        int ty = img.height / sy;
        int rx = img.width % sx;
        int ry = img.height % sy;

        for(int x1 = 0 ; x1 < tx ; x1++){
            for(int y1 = 0 ; y1 < ty ; y1++){
                levelHelper(img, x1 * sx, y1 * sy, sx, sy);
            }
        }
        if(rx > 0){
            for(int y1 = 0 ; y1 < ty ; y1++){
                levelHelper(img, tx * sx, y1 * sy, rx, sy);
            }
        }
        if(ry > 0){
            for(int x1 = 0 ; x1 < tx ; x1++){
                levelHelper(img, x1 * sx, ty * sy, sx, ry);
            }
        }
        if(ry > 0 && rx > 0){
            levelHelper(img, tx * sx, ty * sy, rx, ry);
        }
        std::cout << "Image Levelled" << std::endl;
    }

    void handleTransform(const std::vector<std::string>& args, TransformFunc func) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        std::string direction = "horizontal";
        if (!args.empty()) {
            direction = args[0];
        }
        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int tx = img.width / sx;
        int ty = img.height / sy;
        int rx = img.width % sx;
        int ry = img.height % sy;

        bool flag = false;

        
        if (direction == "h" || direction == "d") {
            flag = true;

            std::vector<Complex> strip_0(sy);
            std::vector<Complex> strip_1(sy);
            std::vector<Complex> strip_2(sy);
            for(int x2 = 0 ; x2 < img.width ; x2++){
                for(int y1 = 0 ; y1 < ty ; y1++){
                    
                    for (int y = 0 ; y < sy; y++) {
                        int y2 = y1 * sy + y;
                        strip_0[y] = img.pixels[y2][x2][0];
                        strip_1[y] = img.pixels[y2][x2][1];
                        strip_2[y] = img.pixels[y2][x2][2];
                    }

                    strip_0 = func(strip_0);
                    strip_1 = func(strip_1);
                    strip_2 = func(strip_2);

                    for (int y = 0 ; y < sy ; y++) {
                        int y2 = y1 * sy + y;
                        img.pixels[y2][x2][0] = strip_0[y];
                        img.pixels[y2][x2][1] = strip_1[y];
                        img.pixels[y2][x2][2] = strip_2[y];
                    }
                }

                if(ry > 0){

                    std::vector<Complex> rem_strip_0(ry);
                    std::vector<Complex> rem_strip_1(ry);
                    std::vector<Complex> rem_strip_2(ry);

                    for (int y = 0 ; y < ry; y++) {
                        int y2 = img.width - ry + y;
                        rem_strip_0[y] = img.pixels[y2][x2][0];
                        rem_strip_1[y] = img.pixels[y2][x2][1];
                        rem_strip_2[y] = img.pixels[y2][x2][2];
                    }

                    rem_strip_0 = func(rem_strip_0);
                    rem_strip_1 = func(rem_strip_1);
                    rem_strip_2 = func(rem_strip_2);

                    for (int y = 0 ; y < ry ; y++) {
                        int y2 = img.width - ry + y;
                        img.pixels[y2][x2][0] = rem_strip_0[y];
                        img.pixels[y2][x2][1] = rem_strip_1[y];
                        img.pixels[y2][x2][2] = rem_strip_2[y];
                    }


                }
            }
        } if (direction == "v" || direction == "d") {
            flag = true;

            std::vector<Complex> strip_0(sx);
            std::vector<Complex> strip_1(sx);
            std::vector<Complex> strip_2(sx);
            for(int y2 = 0 ; y2 < img.width ; y2++){
                for(int x1 = 0 ; x1 < tx ; x1++){
                    
                    for (int x = 0 ; x < sx; x++) {
                        int x2 = x1 * sx + x;
                        strip_0[x] = img.pixels[y2][x2][0];
                        strip_1[x] = img.pixels[y2][x2][1];
                        strip_2[x] = img.pixels[y2][x2][2];
                    }

                    strip_0 = func(strip_0);
                    strip_1 = func(strip_1);
                    strip_2 = func(strip_2);

                    for (int x = 0 ; x < sx ; x++) {
                        int x2 = x1 * sx + x;
                        img.pixels[y2][x2][0] = strip_0[x];
                        img.pixels[y2][x2][1] = strip_1[x];
                        img.pixels[y2][x2][2] = strip_2[x];
                    }
                }

                if(rx > 0){

                    std::vector<Complex> rem_strip_0(rx);
                    std::vector<Complex> rem_strip_1(rx);
                    std::vector<Complex> rem_strip_2(rx);

                    for (int x = 0 ; x < rx; x++) {
                        int x2 = img.width - rx + x;
                        rem_strip_0[x] = img.pixels[y2][x2][0];
                        rem_strip_1[x] = img.pixels[y2][x2][1];
                        rem_strip_2[x] = img.pixels[y2][x2][2];
                    }

                    rem_strip_0 = func(rem_strip_0);
                    rem_strip_1 = func(rem_strip_1);
                    rem_strip_2 = func(rem_strip_2);

                    for (int x = 0 ; x < rx ; x++) {
                        int x2 = img.width - rx + x;
                        img.pixels[y2][x2][0] = rem_strip_0[x];
                        img.pixels[y2][x2][1] = rem_strip_1[x];
                        img.pixels[y2][x2][2] = rem_strip_2[x];
                    }


                }
            }
        }
        if (!flag) {
            std::cerr << "Error: Invalid direction. Use 'd', 'h' or 'v'" << std::endl;
        }
        else {
            if ( direction == "v") std::cout << "Image transformed along vertical axis" << std::endl;
            if ( direction == "h") std::cout << "Image transformed along horizontal axis" << std::endl;
            if ( direction == "d") std::cout << "Image transformed along both axes" << std::endl;
        }
    }
    
    // Parse command line into command and arguments
    std::pair<std::string, std::vector<std::string>> parseInput(const std::string& input) {
        std::istringstream iss(input);
        std::string command;
        std::vector<std::string> args;
        
        if (iss >> command) {
            std::string arg;
            while (iss >> arg) {
                args.push_back(arg);
            }
        }
        
        return {command, args};
    }
    
public:
    CLI() : running(true) {
        // Register commands
        registerCommand("load", 
            [this](const std::vector<std::string>& args) { handleLoad(args); },
            "Load image from BMP file into 3D RGB array",
            "load <filename.bmp>",
            "-n"
        );
        
        registerCommand("save", 
            [this](const std::vector<std::string>& args) { handleSave(args); },
            "Save current image to BMP file",
            "save <filename.bmp>",
            "-n"
        );
        
        registerCommand("info", 
            [this](const std::vector<std::string>& args) { handleInfo(args); },
            "Show information about currently loaded image",
            "info",
            "-n"
        );
        
        registerCommand("exit", 
            [this](const std::vector<std::string>& args) { handleExit(args); },
            "Exit the program",
            "exit",
            "NONE"
        );
        
        registerCommand("quit", 
            [this](const std::vector<std::string>& args) { handleExit(args); },
            "Exit the program",
            "quit",
            "NONE"
        );
        
        registerCommand("help", 
            [this](const std::vector<std::string>& args) { handleHelp(args); },
            "Show available commands",
            "help",
            "NONE"
        );

        registerCommand("invert", 
            [this](const std::vector<std::string>& args) { handleInvert(args); },
            "Invert colors of the current image",
            "invert",
            "-n"
        );
        
        registerCommand("grayscale", 
            [this](const std::vector<std::string>& args) { handleGrayscale(args); },
            "Convert current image to grayscale",
            "grayscale",
            "-n"
        );
        
        registerCommand("flip", 
            [this](const std::vector<std::string>& args) { handleFlip(args); },
            "Flip image horizontally or vertically",
            "flip [horizontal | vertical]",
            "-n"
        );

        registerCommand("abs", 
            [this](const std::vector<std::string>& args) { handleAbs(args); },
            "Replaces each pixel with absolute value",
            "abs",
            "-n"
        );

        registerCommand("quant", 
            [this](const std::vector<std::string>& args) { handleQuantize(args); },
            "Replaces each pixel with absolute value",
            "quant -s [int]",
            "-n -s"
        );

        registerCommand("level", 
            [this](const std::vector<std::string>& args) { handleLevel(args); },
            "Averages each square",
            "level -sx [int] -sy [int]",
            "-n -sx -sy"
        );

        registerCommand("cutoff", 
            [this](const std::vector<std::string>& args) { handleCutoff(args); },
            "Replaces value with 0 if absolute value is less than s",
            "cutoff -s [int]",
            "-n -s"
        );

        registerCommand("fft", 
            [this](const std::vector<std::string>& args) { handleTransform(args, fft); },
            "Fourier Transforms image horizontally or vertically",
            "fft [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("ifft", 
            [this](const std::vector<std::string>& args) { handleTransform(args, ifft); },
            "Inverse Fourier Transforms image horizontally or vertically",
            "ifft [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("dft", 
            [this](const std::vector<std::string>& args) { handleTransform(args, dft); },
            "Fourier Transforms image horizontally or vertically",
            "dft [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("idft", 
            [this](const std::vector<std::string>& args) { handleTransform(args, idft); },
            "Inverse Fourier Transforms image horizontally or vertically",
            "idft [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("dct", 
            [this](const std::vector<std::string>& args) { handleTransform(args, dct2); },
            "Cosine Transforms real part of image horizontally or vertically",
            "dct [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("idct", 
            [this](const std::vector<std::string>& args) { handleTransform(args, idct2); },
            "Inverse Cosine Transforms real part of image horizontally or vertically",
            "idct [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("dst", 
            [this](const std::vector<std::string>& args) { handleTransform(args, dst2); },
            "Sine Transforms real part of image horizontally or vertically",
            "dst [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("idst", 
            [this](const std::vector<std::string>& args) { handleTransform(args, idst2); },
            "Inverse Sine Transforms real part of image horizontally or vertically",
            "idst [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("wht", 
            [this](const std::vector<std::string>& args) { handleTransform(args, wht); },
            "Walsh-Hadamard Transforms image horizontally or vertically",
            "wht [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("iwht", 
            [this](const std::vector<std::string>& args) { handleTransform(args, iwht); },
            "Inverse Walsh-Hadamard Transforms image horizontally or vertically",
            "iwht [h | v | d]",
            "-n -sx -sy"
        );
    }
    
    // Method to register new commands (for scalability)
    void registerCommand(const std::string& name, 
                        std::function<void(const std::vector<std::string>&)> handler,
                        const std::string& description = "",
                        const std::string& usage = "",
                        const std::string& flags = "" ) {
        commands[name] = {handler, description, usage, flags};
    }
    
    // Main CLI loop
    void run() {
        std::cout << "nLoss++ Started. Type 'help' for available commands." << std::endl;
        std::cout << "Supported format: 24-bit uncompressed BMP files" << std::endl;
        
        while (running) {
            std::cout << "> ";
            std::string input;
            std::getline(std::cin, input);
            
            // Skip empty input
            if (input.empty()) {
                continue;
            }
            
            auto [command, args] = parseInput(input);
            executeCommand(command, args);
        }
    }
    
    // Execute a command
    void executeCommand(const std::string& command, const std::vector<std::string>& args) {
        auto it = commands.find(command);
        if (it != commands.end()) {
            try {
                it->second.handler(args);
            } catch (const std::exception& e) {
                std::cerr << "Error executing command '" << command << "': " << e.what() << std::endl;
            }
        } else {
            std::cout << "Unknown command: " << command << ". Type 'help' for available commands." << std::endl;
        }
    }
};

int main() {
    CLI cli;
    cli.run();
    return 0;
}