#!/bin/bash
set -e

# Bootstrap vcpkg if missing
if [ ! -d ".vcpkg" ]; then
  echo "Cloning vcpkg..."
  git clone https://github.com/microsoft/vcpkg.git .vcpkg
  ./.vcpkg/bootstrap-vcpkg.sh
fi

echo "Running CMake with vcpkg toolchain..."
cmake -Bbuild -S. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$PWD/.vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j$(nproc)