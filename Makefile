# Define the C++ compiler and standard flags
CXX = g++
# -Wall: Enable all standard warnings
# -Wextra: Enable extra warnings
# -std=c++17: Use the C++17 standard
# -O2: Optimization level 2
# NEW: -MMD and -MP automatically generate dependency files (.d) for accurate header tracking.
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -MMD -MP

# Define the build directory where all artifacts will be placed
BUILD_DIR = build
# Define the name of the final executable
TARGET_NAME = nLOSS
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

# Define all source files (.cpp)
SRCS = nLOSS.cpp ImageData.cpp FFTTools.cpp
# Create a list of object files (.o) with the build directory path prefix
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.cpp=.o))
# Define the dependency files (.d) which mirror the .o files
DEPS = $(OBJS:.o=.d)

# -------------------------------------------------------------------
# Include automatically generated dependency files
# -include tells make to ignore errors if the files don't exist yet (first run).
# This is how the object files gain their dependency on header files.
# -------------------------------------------------------------------
-include $(DEPS)

# -------------------------------------------------------------------
# Primary Build Targets
# -------------------------------------------------------------------

# The default target. Builds the final executable.
all: $(TARGET)

# Rule to ensure the build directory exists before attempting compilation or linking.
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Rule to link the object files into the final executable.
# It depends on all object files and the existence of the build directory.
$(TARGET): $(OBJS) | $(BUILD_DIR)
	@echo "==> Linking $(TARGET_NAME)..."
	$(CXX) $(OBJS) -o $(TARGET)

# Pattern rule for compiling any source file (%.cpp) into an object file (%.o).
# The compiler flags (-MMD -MP) in CXXFLAGS automatically generate the dependency files (.d)
# which are then included above.
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo "  -> Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------------------------------------------------------------------
# Utility Targets
# -------------------------------------------------------------------

# Clean target: removes the entire build directory, including generated .o, .d, and the executable.
clean:
	@echo "==> Cleaning up build directory..."
	$(RM) -r $(BUILD_DIR)

# Phony targets prevent 'make' from confusing targets with similarly named files.
.PHONY: all clean
