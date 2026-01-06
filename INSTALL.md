# ZenRunner Installation Guide

This guide will help you install ZenRunner on your system. ZenRunner is a lightweight, native process manager for developers built with C++20 and Qt 6.

## Table of Contents
- [System Requirements](#system-requirements)
- [Quick Installation](#quick-installation)
- [Detailed Installation](#detailed-installation)
- [Post-Installation](#post-installation)
- [Troubleshooting](#troubleshooting)
- [Uninstallation](#uninstallation)

## System Requirements

### Minimum Requirements
- **Operating System**: 
  - Linux (Ubuntu 20.04+ / Debian 10+ / Fedora 34+)
  - macOS 11 (Big Sur) or later
  - Windows 10 (version 1809+) or Windows 11
- **RAM**: 2 GB (ZenRunner uses < 30 MB)
- **Disk Space**: 100 MB for application and dependencies
- **CPU**: Any modern x86_64 or ARM64 processor

### Required Software
- **CMake**: 3.21 or higher
- **Qt 6**: 6.2 or higher
- **C++20 Compiler**:
  - Linux: GCC 10+ or Clang 12+
  - macOS: Xcode 13+ (includes Apple Clang)
  - Windows: Visual Studio 2019+ or MinGW-w64

## Quick Installation

### Using Installation Script (Recommended)

#### Linux / macOS
```bash
# Clone the repository
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner

# Run the installation script
chmod +x install.sh
./install.sh

# Launch ZenRunner
./zenrunner.sh
```

#### Windows (PowerShell)
```powershell
# Clone the repository
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner

# Run the installation script
.\install.ps1

# Launch ZenRunner
.\zenrunner.bat
```

## Detailed Installation

### Step 1: Install Dependencies

#### Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    libqt6core6 \
    libqt6gui6 \
    libqt6quick6 \
    libqt6widgets6 \
    git
```

#### Fedora / RHEL / CentOS
```bash
sudo dnf install -y \
    cmake \
    gcc-c++ \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qttools-devel \
    git
```

#### Arch Linux
```bash
sudo pacman -S --needed \
    base-devel \
    cmake \
    qt6-base \
    qt6-declarative \
    qt6-tools \
    git
```

#### macOS (Homebrew)
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake qt@6 git

# Add Qt to PATH
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"

# Add these to your ~/.zshrc or ~/.bash_profile for permanent access
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
echo 'export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"' >> ~/.zshrc
```

#### Windows

**Option 1: Using Qt Online Installer (Recommended)**
1. Download Qt Online Installer from https://www.qt.io/download-open-source-software
2. Run the installer and select:
   - Qt 6.5 or later
   - Qt Quick (QML)
   - CMake integration
   - MinGW 64-bit or MSVC 2019 64-bit compiler

**Option 2: Using Visual Studio**
1. Install Visual Studio 2019 or later with "Desktop development with C++" workload
2. Install CMake (included with Visual Studio or download from cmake.org)
3. Install Qt 6 as described in Option 1

### Step 2: Clone the Repository

```bash
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner
```

### Step 3: Build ZenRunner

#### Linux / macOS
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build (use -j for parallel compilation)
cmake --build . --config Release -j$(nproc)

# The executable will be in build/bin/ZenRunner
```

#### Windows (Command Prompt)
```cmd
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# The executable will be in build\bin\Release\ZenRunner.exe
```

#### Windows (Visual Studio)
```cmd
# Generate Visual Studio solution
cmake .. -G "Visual Studio 17 2022" -A x64

# Build using MSBuild
cmake --build . --config Release

# Or open build/ZenRunner.sln in Visual Studio and build from there
```

### Step 4: Install (Optional)

#### Linux / macOS (System-wide installation)
```bash
# From the build directory
sudo cmake --install . --prefix /usr/local

# This will install:
# - Executable: /usr/local/bin/ZenRunner
# - Desktop file: /usr/local/share/applications/zenrunner.desktop (if available)
```

#### Linux (User installation)
```bash
# Install to user directory
cmake --install . --prefix ~/.local

# Add to PATH if not already there
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Post-Installation

### Verify Installation

Run ZenRunner to verify it's working:

#### Linux / macOS
```bash
# If installed system-wide
ZenRunner

# If using launcher script
./zenrunner.sh

# If running from build directory
./build/bin/ZenRunner
```

#### Windows
```cmd
# If installed
ZenRunner.exe

# If using launcher script
zenrunner.bat

# If running from build directory
build\bin\Release\ZenRunner.exe
```

### First Run

On first launch, ZenRunner will:
1. Create configuration directories in your home folder
2. Initialize settings storage
3. Display the welcome screen

**Configuration locations:**
- Linux: `~/.config/ZenRunner/`
- macOS: `~/Library/Application Support/ZenRunner/`
- Windows: `%APPDATA%\ZenRunner\`

### Desktop Integration (Optional)

#### Linux
Create a desktop entry:

```bash
cat > ~/.local/share/applications/zenrunner.desktop << EOF
[Desktop Entry]
Type=Application
Name=ZenRunner
Comment=Lightweight process manager for developers
Exec=/usr/local/bin/ZenRunner
Icon=zenrunner
Terminal=false
Categories=Development;Utility;
Keywords=process;manager;development;
StartupWMClass=ZenRunner
EOF

# Update desktop database
update-desktop-database ~/.local/share/applications/
```

#### macOS
The app bundle should be automatically detected. You can also add it to Dock or Applications folder.

#### Windows
Create a shortcut:
1. Right-click on `ZenRunner.exe`
2. Select "Create shortcut"
3. Move shortcut to Desktop or Start Menu

## Troubleshooting

### Qt6 Not Found

**Symptoms**: CMake error "Could not find Qt6"

**Solutions**:

```bash
# Linux: Ensure Qt6 packages are installed
apt list --installed | grep qt6

# macOS: Set Qt path
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"

# Windows: Set Qt path in CMake
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
```

### Missing Dependencies

**Symptoms**: Compilation errors about missing headers

**Solutions**:

```bash
# Ubuntu/Debian: Install missing Qt modules
sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel

# macOS
brew reinstall qt@6
```

### Application Won't Start

**Symptoms**: Crashes on startup or "Library not loaded" errors

**Solutions**:

#### Linux
```bash
# Check library dependencies
ldd ./build/bin/ZenRunner

# If libraries are missing, add Qt library path
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
```

#### macOS
```bash
# Check library dependencies
otool -L ./build/bin/ZenRunner

# If Qt libraries not found
export DYLD_LIBRARY_PATH="/opt/homebrew/opt/qt@6/lib:$DYLD_LIBRARY_PATH"
```

#### Windows
Ensure Qt DLLs are in PATH or copy them next to the executable:
- Qt6Core.dll
- Qt6Gui.dll
- Qt6Quick.dll
- Qt6Widgets.dll
- Qt6Qml.dll

### Build Errors

**C++20 not supported**:
```bash
# Update your compiler
# Ubuntu
sudo apt install gcc-11 g++-11
export CXX=g++-11

# macOS
xcode-select --install
```

**Out of memory during compilation**:
```bash
# Reduce parallel jobs
cmake --build . --config Release -j2
```

### Permission Denied

**Linux/macOS**:
```bash
# Make scripts executable
chmod +x install.sh zenrunner.sh

# If installation fails
sudo cmake --install build/ --prefix /usr/local
```

## Uninstallation

### Using Uninstall Script
```bash
# Linux / macOS
./uninstall.sh

# Windows
.\uninstall.ps1
```

### Manual Uninstallation

#### Linux / macOS
```bash
# If installed system-wide
sudo rm /usr/local/bin/ZenRunner
sudo rm -rf /usr/local/share/zenrunner

# Remove user data
rm -rf ~/.config/ZenRunner/
rm -rf ~/.local/share/ZenRunner/

# Remove desktop entry
rm ~/.local/share/applications/zenrunner.desktop
```

#### Windows
```cmd
# Remove executable (if installed)
del "C:\Program Files\ZenRunner\ZenRunner.exe"

# Remove user data
rmdir /s /q "%APPDATA%\ZenRunner"
```

## Getting Help

If you encounter issues not covered in this guide:

1. **Check existing documentation**: See [README.md](README.md) and [docs/](docs/)
2. **Build and test docs**: See [docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md)
3. **Report an issue**: https://github.com/marcucus/ZenRunner/issues
4. **Community support**: Check discussions on GitHub

## Next Steps

After installation:
1. Read the [User Guide](docs/WORKSPACE_GUIDE.md) to learn about workspace management
2. Check [Performance Guide](docs/PERFORMANCE.md) for optimization tips
3. Explore [Native Integration](docs/NATIVE_INTEGRATION.md) features

Enjoy using ZenRunner! 🚀
