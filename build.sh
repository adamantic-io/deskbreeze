#!/bin/bash

# Build script for WebView-based DeskBreeze app

set -e

echo "Building DeskBreeze WebView App..."

# Create build directory
mkdir -p build
cd build

# Configure cmake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: $(pwd)/app/DeskBreezeWebView"