# Building ZenRunner

This guide explains how to build and run ZenRunner from source.

## Prerequisites

### Required Software

1. **Qt 6.5 or later**
   - Download from: https://www.qt.io/download-qt-installer
   - Required components:
     - Qt Core
     - Qt Quick
     - Qt Widgets

2. **CMake 3.21 or later**
   - Download from: https://cmake.org/download/

3. **C++20 Compatible Compiler**
   - **Linux**: GCC 10+ or Clang 12+
   - **macOS**: Xcode 13+ (Clang)
   - **Windows**: Visual Studio 2019+ or MinGW-w64

### Platform-Specific Requirements

#### Windows
- Windows 10 or Windows 11
- Windows SDK 10.0.22000+ (for Mica effect on Windows 11)
- Visual Studio 2019 or later (recommended)

#### macOS
- macOS 10.15 (Catalina) or later
- Xcode Command Line Tools
- For Vibrancy effects: macOS 10.10+
- For Notch detection: macOS 12.0+

#### Linux
- X11 or Wayland development libraries
- OpenGL development headers
- Example (Ubuntu/Debian):
  ```bash
  sudo apt-get install build-essential libgl1-mesa-dev
  ```

## Building from Source

### 1. Clone the Repository

```bash
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner
```

### 2. Configure with CMake

```bash
mkdir build
cd build
cmake ..
```

**Troubleshooting Qt6 Not Found:**

If CMake cannot find Qt6, specify the Qt installation path:

```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

Common Qt installation paths:
- **Linux**: `~/Qt/6.x.x/gcc_64`
- **macOS**: `~/Qt/6.x.x/macos`
- **Windows**: `C:\Qt\6.x.x\msvc2019_64`

### 3. Build

```bash
cmake --build .
```

Or use platform-specific build tools:

**Unix/Linux/macOS:**
```bash
make -j$(nproc)
```

**Windows (Visual Studio):**
```bash
cmake --build . --config Release
```

### 4. Run

**Unix/Linux:**
```bash
./ZenRunner
```

**macOS:**
```bash
./ZenRunner.app/Contents/MacOS/ZenRunner
```

**Windows:**
```bash
.\Release\ZenRunner.exe
```

## Development Build

For development with faster iteration:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## IDE Setup

### Qt Creator (Recommended)

1. Open Qt Creator
2. File → Open File or Project
3. Select `CMakeLists.txt` from the ZenRunner root directory
4. Configure the project with your Qt kit
5. Build and run with Ctrl+R (Cmd+R on macOS)

### Visual Studio Code

1. Install extensions:
   - CMake Tools
   - C/C++
   - Qt tools

2. Open the ZenRunner folder in VS Code

3. Configure CMake kit when prompted

4. Press F7 to build

### Visual Studio (Windows)

1. Generate Visual Studio solution:
   ```bash
   mkdir build-vs
   cd build-vs
   cmake -G "Visual Studio 17 2022" ..
   ```

2. Open `ZenRunner.sln` in Visual Studio

3. Build with Ctrl+Shift+B

## Verifying the Build

After building, you should see:
- Main executable: `ZenRunner` (or `ZenRunner.exe` on Windows)
- QML resources compiled into the binary
- Platform-specific libraries linked

Test the build:
```bash
./ZenRunner --version  # Check version info
./ZenRunner            # Launch the application
```

## Common Build Issues

### Issue: Qt6 not found

**Solution:** Set CMAKE_PREFIX_PATH to your Qt installation:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

### Issue: C++20 not supported

**Solution:** Update your compiler:
- Linux: Install GCC 10+ or Clang 12+
- macOS: Update Xcode to version 13+
- Windows: Update Visual Studio to 2019+

### Issue: Missing Qt Quick

**Solution:** Install Qt Quick module through Qt Maintenance Tool:
1. Run Qt Maintenance Tool
2. Select "Add or remove components"
3. Ensure Qt Quick is checked

### Issue: GLSL shader compilation errors

**Solution:** Ensure you have OpenGL support:
- Linux: Install mesa development packages
- Update graphics drivers

## Performance Optimization

For maximum performance in release builds:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
```

**Note:** `-march=native` optimizes for your CPU but binary won't be portable.

## Testing the Build

1. **Launch the application**
2. **Verify UI rendering** - Should see glassmorphism effects
3. **Check platform effects:**
   - Windows 11: Look for Mica background effect
   - macOS: Look for Vibrancy blur effect
4. **Monitor performance:**
   - UI should maintain 60 FPS
   - Memory usage should be < 30MB idle

## Next Steps

- See [UI_ARCHITECTURE.md](./UI_ARCHITECTURE.md) for UI component details
- See [../specifications.md](../specifications.md) for technical specifications
- Check [../README.md](../README.md) for project overview

## Getting Help

If you encounter build issues:

1. Check the [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)
2. Verify you have all prerequisites installed
3. Try cleaning the build directory: `rm -rf build && mkdir build`
4. Check CMake output for specific error messages

## Contributing

When building for development:
- Use Debug builds for better error messages
- Enable all compiler warnings
- Run with memory leak detection tools
- Test on your target platform

Happy building! 🚀
