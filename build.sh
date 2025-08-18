#!/bin/bash

remove_www() {
    echo "Removing app/www directory and app/www.zip ..."
    rm -rf app/www
    rm -f app/www.zip
    rm -f app/embedded_assets.cpp
}

# Build script for WebView-based DeskBreeze app

set -e

echo "Building DeskBreeze WebView App..."

remove_www

# Create build directory
mkdir -p build
cd build

# Configure cmake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: $(pwd)/app/DeskBreezeWebView"