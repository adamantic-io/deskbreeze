# DeskBreeze
A bare-bones C++ & React all-in-one back-and-frontend for desktop applications

# Build instructions
## Prerequisites
1. Install a C++ compiler (e.g., GCC or Clang).
2. Install Qt framework (version 5.15 or later recommended).
3. Ensure `CMake` (version 3.15 or later) is installed.
4. Install `git` for cloning the repository.

## Dependencies
To install the required dependencies, follow these steps:

### On Linux:
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-webengine-dev libsqlite3-dev
```

### On macOS:
1. Install Homebrew if not already installed: [Homebrew](https://brew.sh/)
2. Use Homebrew to install dependencies:
    ```bash
    brew install qt sqlite
    ```

### On Windows:
1. Download and install the Qt framework from [Qt Downloads](https://www.qt.io/download).
2. Ensure `sqlite3` is included in your environment or install it via a package manager like [vcpkg](https://github.com/microsoft/vcpkg):
    ```bash
    vcpkg install sqlite3

    ```

## Build Steps
1. Clone the repository:
    ```bash
    git clone https://github.com/yourusername/PetStoreQt.git
    cd PetStoreQt
    ```

2. Create a build directory:
    ```bash
    mkdir build
    cd build
    ```

3. Configure the project using CMake:
    ```bash
    cmake ..
    ```

4. Build the project:
    ```bash
    cmake --build .
    ```

5. Run the application:
    ```bash
    ./PetStoreQt
    ```
