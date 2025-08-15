# DeskBreeze
A bare-bones C++ & React all-in-one back-and-frontend for desktop applications

# Build instructions
## Prerequisites
1. Install a C++ compiler (e.g., GCC or Clang).
2. Install Qt framework (version 5.15 or later recommended).
3. Ensure `CMake` (version 3.15 or later) is installed.
4. Install `git` for cloning the repository.


## Platform Support

### Linux (GTK WebKit)
```bash
# Install dependencies on Ubuntu/Debian:
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.0-dev libsqlite3-dev

# Install dependencies on Fedora/RHEL:
sudo dnf install gtk3-devel webkit2gtk3-devel sqlite-devel
```

### macOS (WKWebView)
- Uses native WKWebView framework
- Install Homebrew if not already installed: [Homebrew](https://brew.sh/)
- Use Homebrew to install other dependencies:
    ```bash
    brew install sqlite
    ```

- No additional dependencies required

### Windows (WebView2)
- Uses Microsoft WebView2 runtime
- WebView2 is included with Windows 11 and newer Windows 10 versions
- For older systems, WebView2 runtime will be installed automatically
- Ensure `sqlite3` is included in your environment or install it via a package manager like [vcpkg](https://github.com/microsoft/vcpkg):
    ```bash
    vcpkg install sqlite3

    ```

### On Windows:
1. Download and install the Qt framework from [Qt Downloads](https://www.qt.io/download).

## Build Steps
1. Clone the repository:
    ```bash
    git clone https://github.com/adamantic-io/deskbreeze.git
    cd deskbreeze
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
