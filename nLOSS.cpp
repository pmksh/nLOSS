#include "Commons.h"
#include "ImageData.h"
#include "Utils.h"
#include "BmpTools.h"
#include "FFTTools.h"
#include "FuncTools.h"
#include "FragTools.h"


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <memory>
#include <cstring>
#include <complex>
#include <cmath>
#include <algorithm>





// Parse global flags
std::map<std::string, int> parseVector(const std::vector<std::string>& args, int start, std::map<std::string, bool> allowed){

    std::map<std::string, int> catches = {{"failed", 0}, {"-n", 0}, {"-s", 0}, {"-sx", 0}, {"-sy", 0}, {"-fr", 0}};

    for (int i = start; i < args.size(); ++i) {
        if (args[i] == "-n" && allowed["-n"]) {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val <= 15) catches["-n"] = *val;
            } else {
                std::cout << "Error: -n must be followed by a number"<<std::endl;;
                catches["failed"] = 1;
                return catches;
            }
        }
        else if (args[i] == "-s" && allowed["-s"]) {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val > 0) catches["-s"] = *val;
            } else {
                std::cout << "Error: -s must be followed by a number"<<std::endl;;
                catches["failed"] = 1;
                return catches;
            }
        }
        else if (args[i] == "-sx" && allowed["-sx"]) {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val > 0) catches["-sx"] = *val;
            } else {
                std::cout << "Error: -sx must be followed by a number"<<std::endl;;
                catches["failed"] = 1;
                return catches;
            }
        }
        else if (args[i] == "-sy" && allowed["-sy"]) {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val > 0) catches["-sy"] = *val;
            } else {
                std::cout << "Error: -sy must be followed by a number"<<std::endl;;
                catches["failed"] = 1;
                return catches;
            }
        }
        else if (args[i] == "-fr" && allowed["-fr"]) {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val > 0) catches["-fr"] = *val;
            } else {
                std::cout << "Error: -fr must be followed by a number"<<std::endl;;
                catches["failed"] = 1;
                return catches;
            }
        }
        else {
            std::cout << "Error: argument " << args[i] <<" not allowed in this function"<<std::endl;;
            catches["failed"] = 1;
            return catches;
        }
    }

    return catches;
}


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
    
    // Image info
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

    // Flip
    void handleFlip(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", true}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }
        
        std::string direction = "h";
        if (!args.empty()) {
            direction = args[0];
        }

        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int fr = catches["-fr"];

        std::vector<struct frame> frames;
        if (fr == 0) {
            frames = GridFrag(img.height, img.width, sx, sy);
        }
        else {
            frames = nuFrag(img.height, img.width, fr, 0);
        }


        
        if (direction == "h") {

            for (struct frame f : frames){
                for (int y = 0; y < f.y_size; y++) {
                    for (int x = 0; x < f.x_size / 2; x++) {
                        
                        std::swap(img.pixels[f.y + y][f.x+ x], img.pixels[f.y + y][f.x + f.x_size - 1 - x]);
                    }
                }
            }
            
            std::cout << "Image flipped horizontally" << std::endl;
        } else if (direction == "v") {
            
            for (struct frame f : frames){
                for (int x = 0; x < f.x_size; x++) {
                    for (int y = 0; y < f.y_size / 2; y++) {
                        
                        std::swap(img.pixels[f.y + y][f.x + x], img.pixels[f.y + f.y_size - 1 - y][f.x + x]);
                    }
                }
            }

            std::cout << "Image flipped vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'h' for 'horizontal' or 'v' for 'vertical'" << std::endl;
        }
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

    // Apply Multiplicative filter
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

    // handle pixel functions
    void handleFunc(const std::vector<std::string>& args, PixelFunc func) {
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
                Triple a;
                a[0] = img.pixels[y][x][0];
                a[1] = img.pixels[y][x][1];
                a[2] = img.pixels[y][x][2];
                a = func(a);
                img.pixels[y][x][0] = a[0];
                img.pixels[y][x][1] = a[1];
                img.pixels[y][x][2] = a[2];
                
            }
        }
        
        std::cout << "Applied pixel function" << std::endl;
    }

    // average each block
    void handleLevel(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", true}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int fr = catches["-fr"];

        std::vector<struct frame> frames;
        if (fr == 0) {
            frames = GridFrag(img.height, img.width, sx, sy);
        }
        else {
            frames = nuFrag(img.height, img.width, fr, 0);
        }

        for(struct frame f : frames){
            levelHelper(img, f.x, f.y, f.x_size, f.y_size);
        }

        std::cout << "Image Levelled" << std::endl;
    }

    // Apply transform
    void handleTransform(const std::vector<std::string>& args, TransformFunc func) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", true}};
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
        int fr = catches["-fr"];

        std::vector<struct frame> frames;
        if (fr == 0) {
            frames = GridFrag(img.height, img.width, sx, sy);
        }
        else {
            frames = nuFrag(img.height, img.width, fr, 0);
        }

        bool flag = false;

        for(struct frame f : frames){

            if (direction == "h" || direction == "d") {
                flag = true;

                for (int x0 = f.x ; x0 < f.x + f.x_size; x0++){
                    
                    for (int color = 0; color < 3; color++){
                        std::vector<Complex> strip(f.y_size);
                        for (int i = 0; i < f.y_size; i++) strip[i] = img.pixels[f.y + i][x0][color];
                        strip = func(strip);
                        for (int i = 0; i < f.y_size; i++) img.pixels[f.y + i][x0][color] = strip[i];
                    }
                }
            }

            if (direction == "v" || direction == "d") {
                flag = true;

                for (int y0 = f.y ; y0 < f.y + f.y_size; y0++){

                    for (int color = 0; color < 3; color++){
                        std::vector<Complex> strip(f.x_size);
                        for (int i = 0; i < f.x_size; i++) strip[i] = img.pixels[y0][f.x + i][color];
                        strip = func(strip);
                        for (int i = 0; i < f.x_size; i++) img.pixels[y0][f.x + i][color] = strip[i];
                    }
                }
            }
        }

        if (!flag) {
            std::cerr << "Error: Invalid direction. Use 'd', 'h' or 'v'" << std::endl;
        } else {
            if (direction == "v")
                std::cout << "Image transformed along vertical axis" << std::endl;
            if (direction == "h")
                std::cout << "Image transformed along horizontal axis" << std::endl;
            if (direction == "d")
                std::cout << "Image transformed along both axes" << std::endl;
        }
    }

    // Apply clamp
    void handleClamp(const std::vector<std::string>& args) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", true}, {"-sy", true}, {"-fr", true}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int fr = catches["-fr"];

        std::vector<struct frame> frames;
        if (fr == 0) {
            frames = GridFrag(img.height, img.width, sx, sy);
        }
        else {
            frames = nuFrag(img.height, img.width, fr, 0);
        }
        
        for(struct frame f : frames){

            double maxAbs[3] = {255.0, 255.0, 255.0};

                for (int y = f.y; y < f.y + f.y_size; y++) {
                    for (int x = f.x; x < f.x + f.x_size; x++) {
                        for (int c = 0; c < 3; c++) {
                            maxAbs[c] = std::max(maxAbs[c], std::abs(img.pixels[y][x][c]));
                        }
                    }
                }

                for (int y = f.y; y < f.y + f.y_size; y++) {
                    for (int x = f.x; x < f.x + f.x_size; x++) {
                        for (int c = 0; c < 3; c++) {
                            if (maxAbs[c] != 0.0) {
                                img.pixels[y][x][c] =
                                    img.pixels[y][x][c] / maxAbs[c] * 255.0;
                            }
                        }
                    }
                }
        }

        std::cout << "Image clamped" << std::endl;
    }

    // Apply sort (breaks up pixels)
    void handleSortDisjoint(const std::vector<std::string>& args, SortFunc func) {
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
                    
                    std::sort(strip_0.begin(), strip_0.end(), func);
                    std::sort(strip_1.begin(), strip_1.end(), func);
                    std::sort(strip_2.begin(), strip_2.end(), func);

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

                    std::sort(rem_strip_0.begin(), rem_strip_0.end(), func);
                    std::sort(rem_strip_1.begin(), rem_strip_1.end(), func);
                    std::sort(rem_strip_2.begin(), rem_strip_2.end(), func);

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

                    std::sort(strip_0.begin(), strip_0.end(), func);
                    std::sort(strip_1.begin(), strip_1.end(), func);
                    std::sort(strip_2.begin(), strip_2.end(), func);

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

                    std::sort(rem_strip_0.begin(), rem_strip_0.end(), func);
                    std::sort(rem_strip_1.begin(), rem_strip_1.end(), func);
                    std::sort(rem_strip_2.begin(), rem_strip_2.end(), func);

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
            if ( direction == "v") std::cout << "Image sorted along vertical axis" << std::endl;
            if ( direction == "h") std::cout << "Image sorted along horizontal axis" << std::endl;
            if ( direction == "d") std::cout << "Image sorted along both axes" << std::endl;
        }
    }
    
    // Apply warp
    void handleWarp(const std::vector<std::string>& args, WarpFunc invFunc) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", true}, {"-sx", true}, {"-sy", true}, {"-fr", true}};
        std::map<std::string, int> catches = parseVector(args, 0, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        int sx = catches["-sx"] ? catches["-sx"] : img.width;
        int sy = catches["-sy"] ? catches["-sy"] : img.height;
        int s = catches["-s"] ? catches["-s"] : 0;
        int fr = catches["-fr"];

        std::vector<struct frame> frames;
        if (fr == 0) {
            frames = GridFrag(img.height, img.width, sx, sy);
        }
        else {
            frames = nuFrag(img.height, img.width, fr, 0);
        }

        int fx, fy, fx1, fy1;
        double nx, ny, rx, ry;
        std::vector<std::vector<std::array<Complex,3>>> newPixels(
            img.height,
            std::vector<std::array<Complex,3>>(img.width)
        );
        
        for(struct frame f : frames){


            for (int y = 0; y < f.y_size; y++){
                for (int x = 0; x < f.x_size; x++){
                    Triple a;

                    double normX = static_cast<double>(x) / (f.x_size);
                    double normY = static_cast<double>(y) / (f.y_size);
                    std::pair<double, double> tmp = invFunc(normX , normY);
                    nx = tmp.first;
                    ny = tmp.second;
                    nx *= (f.x_size- 1);
                    ny *= (f.y_size - 1);

                    fx = std::clamp((int)std::floor(nx), 0 , f.x_size - 1);
                    fy = std::clamp((int)std::floor(ny), 0, f.y_size- 1);

                    fx1 = std::clamp(fx + 1, 0, f.x_size - 1);
                    fy1 = std::clamp(fy + 1, 0, f.y_size - 1);

                    rx = nx - (double) std::floor(nx);
                    ry = ny - (double) std::floor(ny);

                    if (s == 1) {
                        newPixels[f.y + y][f.x + x][0] = (1-ry) * (1-rx) * img.pixels[f.y + fy][f.x + fx][0] + 
                                            (ry) * (1-rx) * img.pixels[f.y + fy1][f.x + fx][0] + 
                                            (1-ry) * (rx) * img.pixels[f.y + fy][f.x + fx1][0] + 
                                            (ry) * (rx) * img.pixels[f.y + fy1][f.x + fx1][0];

                        newPixels[f.y + y][f.x + x][1] = (1-ry) * (1-rx) * img.pixels[f.y + fy][f.x + fx][1] + 
                                            (ry) * (1-rx) * img.pixels[f.y + fy1][f.x + fx][1] + 
                                            (1-ry) * (rx) * img.pixels[f.y + fy][f.x + fx1][1] + 
                                            (ry) * (rx) * img.pixels[f.y + fy1][f.x + fx1][1];

                        newPixels[f.y + y][f.x + x][2] = (1-ry) * (1-rx) * img.pixels[f.y + fy][f.x + fx][2] + 
                                            (ry) * (1-rx) * img.pixels[f.y + fy1][f.x + fx][2] + 
                                            (1-ry) * (rx) * img.pixels[f.y + fy][f.x + fx1][2] + 
                                            (ry) * (rx) * img.pixels[f.y + fy1][f.x + fx1][2];
                    }
                    else {
                        newPixels[f.y + y][f.x + x][0] = img.pixels[f.y + fy][f.x + fx][0];
                        newPixels[f.y + y][f.x + x][1] = img.pixels[f.y + fy][f.x + fx][1];
                        newPixels[f.y + y][f.x + x][2] = img.pixels[f.y + fy][f.x + fx][2];
                    }
                }
            }
        }

        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                
                img.pixels[y][x][0] = newPixels[y][x][0];
                img.pixels[y][x][1] = newPixels[y][x][1];
                img.pixels[y][x][2] = newPixels[y][x][2];
            }
        }
        
        std::cout << "Applied warp function" << std::endl;
    }
    
    // Apply func with complex input
    void handleFuncComplex(const std::vector<std::string>& args, PixelFuncComplex func) {
        std::map<std::string, bool> allowed = {{"-n", true}, {"-s", false}, {"-sx", false}, {"-sy", false}, {"-fr", false}};
        std::map<std::string, int> catches = parseVector(args, 1, allowed);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];


        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        double a1, a2;

        if (args.size() < 1 || !parsePair(args[0], a1, a2)){
            std::cerr << "Error: please input two doubles (a,b)" << std::endl;
            return;
        }

        Complex c = Complex(a1,a2);
        
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                Triple a;
                a[0] = img.pixels[y][x][0];
                a[1] = img.pixels[y][x][1];
                a[2] = img.pixels[y][x][2];
                a = func(a,c);
                img.pixels[y][x][0] = a[0];
                img.pixels[y][x][1] = a[1];
                img.pixels[y][x][2] = a[2];
                
            }
        }
        
        std::cout << "Applied pixel function" << std::endl;
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
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_invert); },
            "Invert colors of the current image",
            "invert",
            "-n"
        );
        
        registerCommand("grayscale", 
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_grayscale); },
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
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_absolute); },
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

        registerCommand("sort", 
            [this](const std::vector<std::string>& args) { handleSortDisjoint(args, sort_v1); },
            "Sort colors image horizontally or vertically",
            "sort [h | v | d]",
            "-n -sx -sy"
        );

        registerCommand("fit", 
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_fit); },
            "fit each pixel to [0,255]",
            "fit",
            "-n"
        );

        registerCommand("real", 
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_real); },
            "keep only real part of image",
            "real",
            "-n"
        );

        registerCommand("im", 
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_im); },
            "keep only imaginary part of image",
            "im",
            "-n"
        );

        registerCommand("warp-sqrt", 
            [this](const std::vector<std::string>& args) { handleWarp(args, warp_square); },
            "takes (x,y) -> (sqrt(x),sqrt(y)), -s 1 determines blending",
            "warp-sqrt",
            "-n -s"
        );

        registerCommand("warp-square", 
            [this](const std::vector<std::string>& args) { handleWarp(args, warp_sqrt); },
            "takes (x,y) -> (x^2,y^2), -s 1 determines blending",
            "warp-square",
            "-n -s"
        );

        registerCommand("pixel-square", 
            [this](const std::vector<std::string>& args) { handleFunc(args, PF_square); },
            "takes [r,g,b] -> [r^2,g^2,b^2]",
            "pixel-square",
            "-n"
        );

        registerCommand("pixel-mult", 
            [this](const std::vector<std::string>& args) { handleFuncComplex(args, PFC_mult); },
            "multiplies by a complex constant a+bi",
            "pixel-mult(a,b)",
            "-n"
        );

        registerCommand("pixel-div", 
            [this](const std::vector<std::string>& args) { handleFuncComplex(args, PFC_div); },
            "divides by a complex constant a+bi",
            "pixel-div (a,b)",
            "-n"
        );

        registerCommand("pixel-add", 
            [this](const std::vector<std::string>& args) { handleFuncComplex(args, PFC_add); },
            "adds a complex constant a+bi",
            "pixel-add (a,b)",
            "-n"
        );

        registerCommand("clamp", 
            [this](const std::vector<std::string>& args) { handleClamp(args); },
            "clamps each rectangle within max values",
            "clamp",
            "-n -sx, -sy"
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
            if (! std::getline(std::cin, input)) running = false;
            
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