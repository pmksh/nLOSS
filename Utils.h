

// Try to parse string as int, return value if valid, otherwise std::nullopt
std::optional<int> toInt(const std::string& s) {
    int value{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

    if (ec == std::errc() && ptr == s.data() + s.size()) {
        return value;  // success
    }
    return std::nullopt; // failure
}


// Parse global flags
std::map<std::string, int> parseVector(const std::vector<std::string>& args, int start){

    std::map<std::string, int> catches = {{"failed", 0}, {"-n", 0}, {"-fr", 0}};

    for (int i = start; i < args.size(); ++i) {
        if (args[i] == "-n") {
            i++;
            if (auto val = toInt(args[i])) {
                if(*val <= 15) catches["-n"] = *val;
            } else {
                std::cout << "Error: -n must be followed by a number";
                catches["failed"] = 1;
                return catches;
            }
        }
        else if (args[i] == "-fr") {
            catches["-fr"] = 1;
        }
    }

    return catches;
}


unsigned char int_to_uchar(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<unsigned char>(value);
}

unsigned char double_to_uchar(double value) {
    if (value < 0.0) return 0;
    if (value > 255.0) return 255;
    return static_cast<unsigned char>(floor(value));
}