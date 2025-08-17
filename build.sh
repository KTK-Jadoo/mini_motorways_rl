#!/bin/bash
# build.sh - Mac build script for Mini Motorways RL

echo "Mini Motorways RL - Mac Build Script"
echo "===================================="

# Check for required tools
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Installing..."
    brew install cmake
fi

if ! command -v pkg-config &> /dev/null; then
    echo "❌ pkg-config not found. Installing..."
    brew install pkg-config
fi

# Check for required libraries
echo "Checking dependencies..."

if ! brew list glfw &> /dev/null; then
    echo "❌ GLFW not found. Installing..."
    brew install glfw
fi

if ! brew list glew &> /dev/null; then
    echo "❌ GLEW not found. Installing..."
    brew install glew
fi

if ! brew list glm &> /dev/null; then
    echo "❌ GLM not found. Installing..."
    brew install glm
fi

echo "✅ All dependencies found"

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed"
    exit 1
fi

# Build
echo "Building..."
make -j$(sysctl -n hw.ncpu)

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

echo ""
echo "✅ Build successful!"
echo ""
echo "To run:"
echo "  ./mini_motorways_rl demo          # Interactive demo"
echo "  ./mini_motorways_rl train qlearning 1000 false  # Train agent"
echo "  ./mini_motorways_rl test qlearning final_model.txt 5  # Test agent"
echo ""

# Test if the executable was created
if [ -f "./mini_motorways_rl" ]; then
    echo "🎮 Ready to run! Try: ./mini_motorways_rl demo"
else
    echo "❌ Executable not found. Check build errors above."
fi