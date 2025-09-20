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

using Complex = std::complex<double>;
const double PI = acos(-1.0);

#include "ImageData.h"
#include "Utils.h"
#include "BmpTools.h"
#include "FFTTools.h"

class CLI {
private:
    struct Command {
        std::function<void(const std::vector<std::string>&)> handler;
        std::string description;
        std::string usage;
    };
    
    std::map<std::string, Command> commands;
    bool running;
    ImageData currentImage[16]; // Store the current loaded image
    
    // Command handlers
    void handleLoad(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cerr << "Error: Please specify a filename to load" << std::endl;
            std::cout << "Usage: load <filename.bmp>" << std::endl;
            return;
        }
        
        std::string filename = args[0];
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        
        // Clear any existing image
        img.clear();
        
        // Try to load the image
        if (!loadBMP(filename, img)) {
            std::cerr << "Failed to load BMP image: " << filename << std::endl;
        }
    }
    
    void handleSave(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cerr << "Error: Please specify a filename to save" << std::endl;
            std::cout << "Usage: save <filename.bmp>" << std::endl;
            return;
        }
        
        std::string filename = args[0];
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];
        
        if (!saveBMP(filename, img)) {
            std::cerr << "Failed to save BMP image: " << filename << std::endl;
        }
    }
    
    void handleExit(const std::vector<std::string>& args) {
        std::cout << "Goodbye!" << std::endl;
        running = false;
    }
    
    void handleHelp(const std::vector<std::string>& args) {
        std::cout << "Available commands:" << std::endl;
        for (const auto& [name, cmd] : commands) {
            std::cout << "  " << name << " - " << cmd.description << std::endl;
            if (!cmd.usage.empty()) {
                std::cout << "    Usage: " << cmd.usage << std::endl;
            }
        }
        std::cout << "\nSupported format: 24-bit uncompressed BMP files" << std::endl;
    }
    
    void handleInfo(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 0);
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

    void handleInvert(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 0);
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
    
    void handleGrayscale(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 0);
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
    
    void handleFlip(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 1);
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

    void handleAbs(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 0);
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

    void handleFFT(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        std::string direction = "horizontal";
        int s = -1;
        if (!args.empty()) {
            direction = args[0];
        }
        
        if (direction == "horizontal" || direction == "h") {
            if (s == -1){
                for (int x = 0; x < img.width; x++){
                    std::vector<Complex> strip[3];
                    for (int y = 0; y < img.height; y++) {
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = fft(strip[0]);
                    strip[1] = fft(strip[1]);
                    strip[2] = fft(strip[2]);

                    for (int y = 0; y < img.height; y++) {
                        img.pixels[y][x][0] = strip[0][y];
                        img.pixels[y][x][1] = strip[1][y];
                        img.pixels[y][x][2] = strip[2][y];
                    }
                }   
            }
            std::cout << "Image fourier transformed horizontally" << std::endl;
        } else if (direction == "vertical" || direction == "v") {
            if (s == -1){
                for (int y = 0; y < img.height; y++) {
                    std::vector<Complex> strip[3];
                    for (int x = 0; x < img.width; x++){
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = fft(strip[0]);
                    strip[1] = fft(strip[1]);
                    strip[2] = fft(strip[2]);

                    for (int x = 0; x < img.width; x++){
                        img.pixels[y][x][0] = strip[0][x];
                        img.pixels[y][x][1] = strip[1][x];
                        img.pixels[y][x][2] = strip[2][x];
                    }
                }   
            }
            std::cout << "Image fourier transformed vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'horizontal' or 'vertical'" << std::endl;
        }
    }

    void handleIFFT(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        std::string direction = "horizontal";
        int s = -1;
        if (!args.empty()) {
            direction = args[0];
        }
        
        if (direction == "horizontal" || direction == "h") {
            if (s == -1){
                for (int x = 0; x < img.width; x++){
                    std::vector<Complex> strip[3];
                    for (int y = 0; y < img.height; y++) {
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = ifft(strip[0]);
                    strip[1] = ifft(strip[1]);
                    strip[2] = ifft(strip[2]);

                    for (int y = 0; y < img.height; y++) {
                        img.pixels[y][x][0] = strip[0][y];
                        img.pixels[y][x][1] = strip[1][y];
                        img.pixels[y][x][2] = strip[2][y];
                    }
                }   
            }
            std::cout << "Image inverse fourier transformed horizontally" << std::endl;
        } else if (direction == "vertical" || direction == "v") {
            if (s == -1){
                for (int y = 0; y < img.height; y++) {
                    std::vector<Complex> strip[3];
                    for (int x = 0; x < img.width; x++){
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = ifft(strip[0]);
                    strip[1] = ifft(strip[1]);
                    strip[2] = ifft(strip[2]);

                    for (int x = 0; x < img.width; x++){
                        img.pixels[y][x][0] = strip[0][x];
                        img.pixels[y][x][1] = strip[1][x];
                        img.pixels[y][x][2] = strip[2][x];
                    }
                }   
            }
            std::cout << "Image inverse fourier transformed vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'horizontal' or 'vertical'" << std::endl;
        }
    }

    void handleDFT(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        std::string direction = "horizontal";
        int s = -1;
        if (!args.empty()) {
            direction = args[0];
        }
        
        if (direction == "horizontal" || direction == "h") {
            if (s == -1){
                for (int x = 0; x < img.width; x++){
                    std::vector<Complex> strip[3];
                    for (int y = 0; y < img.height; y++) {
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = dft(strip[0]);
                    strip[1] = dft(strip[1]);
                    strip[2] = dft(strip[2]);

                    for (int y = 0; y < img.height; y++) {
                        img.pixels[y][x][0] = strip[0][y];
                        img.pixels[y][x][1] = strip[1][y];
                        img.pixels[y][x][2] = strip[2][y];
                    }
                }   
            }
            std::cout << "Image discrete fourier transformed horizontally" << std::endl;
        } else if (direction == "vertical" || direction == "v") {
            if (s == -1){
                for (int y = 0; y < img.height; y++) {
                    std::vector<Complex> strip[3];
                    for (int x = 0; x < img.width; x++){
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = dft(strip[0]);
                    strip[1] = dft(strip[1]);
                    strip[2] = dft(strip[2]);

                    for (int x = 0; x < img.width; x++){
                        img.pixels[y][x][0] = strip[0][x];
                        img.pixels[y][x][1] = strip[1][x];
                        img.pixels[y][x][2] = strip[2][x];
                    }
                }   
            }
            std::cout << "Image discrete fourier transformed vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'horizontal' or 'vertical'" << std::endl;
        }
    }

    void handleIDFT(const std::vector<std::string>& args) {
        std::map<std::string, int> catches = parseVector(args, 1);
        if (catches["failed"]) return;
        ImageData& img = currentImage[catches["-n"]];

        if (!img.isLoaded) {
            std::cerr << "Error: No image loaded" << std::endl;
            return;
        }

        std::string direction = "horizontal";
        int s = -1;
        if (!args.empty()) {
            direction = args[0];
        }
        
        if (direction == "horizontal" || direction == "h") {
            if (s == -1){
                for (int x = 0; x < img.width; x++){
                    std::vector<Complex> strip[3];
                    for (int y = 0; y < img.height; y++) {
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = idft(strip[0]);
                    strip[1] = idft(strip[1]);
                    strip[2] = idft(strip[2]);

                    for (int y = 0; y < img.height; y++) {
                        img.pixels[y][x][0] = strip[0][y];
                        img.pixels[y][x][1] = strip[1][y];
                        img.pixels[y][x][2] = strip[2][y];
                    }
                }   
            }
            std::cout << "Image inverse discrete fourier transformed horizontally" << std::endl;
        } else if (direction == "vertical" || direction == "v") {
            if (s == -1){
                for (int y = 0; y < img.height; y++) {
                    std::vector<Complex> strip[3];
                    for (int x = 0; x < img.width; x++){
                        strip[0].push_back(img.pixels[y][x][0]);
                        strip[1].push_back(img.pixels[y][x][1]);
                        strip[2].push_back(img.pixels[y][x][2]);
                    }

                    strip[0] = idft(strip[0]);
                    strip[1] = idft(strip[1]);
                    strip[2] = idft(strip[2]);

                    for (int x = 0; x < img.width; x++){
                        img.pixels[y][x][0] = strip[0][x];
                        img.pixels[y][x][1] = strip[1][x];
                        img.pixels[y][x][2] = strip[2][x];
                    }
                }   
            }
            std::cout << "Image inverse discrete fourier transformed vertically" << std::endl;
        } else {
            std::cerr << "Error: Invalid direction. Use 'horizontal' or 'vertical'" << std::endl;
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
            "load <filename.bmp>"
        );
        
        registerCommand("save", 
            [this](const std::vector<std::string>& args) { handleSave(args); },
            "Save current image to BMP file",
            "save <filename.bmp>"
        );
        
        registerCommand("info", 
            [this](const std::vector<std::string>& args) { handleInfo(args); },
            "Show information about currently loaded image",
            "info"
        );
        
        registerCommand("exit", 
            [this](const std::vector<std::string>& args) { handleExit(args); },
            "Exit the program",
            "exit"
        );
        
        registerCommand("quit", 
            [this](const std::vector<std::string>& args) { handleExit(args); },
            "Exit the program",
            "quit"
        );
        
        registerCommand("help", 
            [this](const std::vector<std::string>& args) { handleHelp(args); },
            "Show available commands",
            "help"
        );

        registerCommand("invert", 
            [this](const std::vector<std::string>& args) { handleInvert(args); },
            "Invert colors of the current image",
            "invert"
        );
        
        registerCommand("grayscale", 
            [this](const std::vector<std::string>& args) { handleGrayscale(args); },
            "Convert current image to grayscale",
            "grayscale"
        );
        
        registerCommand("flip", 
            [this](const std::vector<std::string>& args) { handleFlip(args); },
            "Flip image horizontally or vertically",
            "flip [horizontal | vertical]"
        );

        registerCommand("abs", 
            [this](const std::vector<std::string>& args) { handleAbs(args); },
            "Replaces each pixel with absolute value",
            "abs"
        );

        registerCommand("fft", 
            [this](const std::vector<std::string>& args) { handleFFT(args); },
            "Fourier Transforms image horizontally or vertically",
            "fft [horizontal | vertical]"
        );

        registerCommand("ifft", 
            [this](const std::vector<std::string>& args) { handleIFFT(args); },
            "Inverse Fourier Transforms image horizontally or vertically",
            "ifft [horizontal | vertical]"
        );

        registerCommand("dft", 
            [this](const std::vector<std::string>& args) { handleDFT(args); },
            "Fourier Transforms image horizontally or vertically",
            "dft [horizontal | vertical]"
        );

        registerCommand("idft", 
            [this](const std::vector<std::string>& args) { handleIDFT(args); },
            "Inverse Fourier Transforms image horizontally or vertically",
            "idft [horizontal | vertical]"
        );
    }
    
    // Method to register new commands (for scalability)
    void registerCommand(const std::string& name, 
                        std::function<void(const std::vector<std::string>&)> handler,
                        const std::string& description = "",
                        const std::string& usage = "") {
        commands[name] = {handler, description, usage};
    }
    
    // Main CLI loop
    void run() {
        std::cout << "Image Processing CLI Started. Type 'help' for available commands." << std::endl;
        std::cout << "Supported format: 24-bit uncompressed BMP files" << std::endl;
        std::cout << "Try 'create' to make a test image, then 'save test.bmp' to save it." << std::endl;
        
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