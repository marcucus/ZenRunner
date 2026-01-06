# ZenRunner Quick Start Guide

Get up and running with ZenRunner in minutes!

## 🚀 Quick Installation

### One-Line Install

#### Linux / macOS
```bash
git clone https://github.com/marcucus/ZenRunner.git && cd ZenRunner && chmod +x install.sh && ./install.sh
```

#### Windows (PowerShell - Run as Administrator)
```powershell
git clone https://github.com/marcucus/ZenRunner.git; cd ZenRunner; .\install.ps1
```

### What It Does

The installation script will:
1. ✅ Check for required dependencies (CMake, Qt6, C++ compiler)
2. ✅ Build ZenRunner with Release optimizations
3. ✅ Install to your system or user directory
4. ✅ Create a convenient launcher script
5. ✅ Set up PATH for easy access

### Installation Time
- **First time**: 5-10 minutes (including dependency downloads if needed)
- **Subsequent builds**: 2-3 minutes

## 🎯 Launch ZenRunner

After installation, launch ZenRunner using:

```bash
# Option 1: Using launcher (recommended)
./zenrunner.sh          # Linux / macOS
.\zenrunner.bat         # Windows

# Option 2: Direct execution
ZenRunner               # If installed to PATH
~/.local/bin/ZenRunner  # Linux / macOS user install
```

## 📦 System Requirements

### Minimum
- **RAM**: 2 GB (ZenRunner uses < 30 MB)
- **Disk**: 100 MB
- **OS**: Linux (Ubuntu 20.04+), macOS (11+), Windows (10/11)

### Dependencies
- **CMake** 3.21+
- **Qt 6** 6.2+
- **C++20** compiler (GCC 10+, Clang 12+, MSVC 2019+)

## 🔧 Dependency Installation

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-declarative-dev qt6-tools-dev git
```

### Fedora
```bash
sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel qt6-qtdeclarative-devel git
```

### Arch Linux
```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative qt6-tools git
```

### macOS (Homebrew)
```bash
brew install cmake qt@6 git
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"
```

### Windows
1. Install [Visual Studio 2019+](https://visualstudio.microsoft.com/) with C++ workload
2. Install [CMake](https://cmake.org/download/)
3. Install [Qt 6](https://www.qt.io/download-open-source-software)
4. Install [Git](https://git-scm.com/download/win)

## 🎨 First Run

1. **Launch ZenRunner**
   ```bash
   ./zenrunner.sh  # or .\zenrunner.bat on Windows
   ```

2. **Import a Project**
   - Click "Add Project" or drag & drop a folder
   - ZenRunner automatically detects `package.json` scripts

3. **Create a Workspace** (Optional)
   - Group related projects together
   - Launch multiple projects with one click
   - Choose parallel or sequential execution

4. **Run Your Development Servers**
   - Click any script button to start a process
   - View live logs with ANSI color support
   - Monitor process status in system tray

## 🛠️ Common Commands

### Installation
```bash
./install.sh              # Install to user directory
./install.sh -m system    # Install system-wide (needs sudo)
./install.sh -c           # Clean build
./install.sh --no-install # Build only, don't install
```

### Launching
```bash
./zenrunner.sh            # Launch ZenRunner
./zenrunner.sh --version  # Show version (if supported)
```

### Uninstallation
```bash
./uninstall.sh            # Remove ZenRunner
```

## 📚 Next Steps

After getting ZenRunner running:

1. **Learn Workspace Management**
   - Read: [docs/WORKSPACE_GUIDE.md](docs/WORKSPACE_GUIDE.md)
   - Create workspaces for your projects
   - Use batch actions for productivity

2. **Explore Native Features**
   - Read: [docs/NATIVE_INTEGRATION.md](docs/NATIVE_INTEGRATION.md)
   - System tray integration
   - Native notifications
   - OS-specific effects (Mica/Vibrancy)

3. **Understand Performance**
   - Read: [docs/PERFORMANCE.md](docs/PERFORMANCE.md)
   - 60 FPS UI optimization
   - Memory management
   - GPU acceleration

4. **Build & Test**
   - Read: [docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md)
   - Performance testing
   - Debug builds
   - Profiling tools

## ❓ Troubleshooting

### "Qt6 not found"
```bash
# Linux
sudo apt install qt6-base-dev qt6-declarative-dev

# macOS
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"

# Windows
# Set Qt path in CMake: cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
```

### "Executable not found"
```bash
# Rebuild the application
./install.sh -c
```

### "Permission denied"
```bash
# Make scripts executable
chmod +x install.sh zenrunner.sh uninstall.sh
```

### Build errors
```bash
# Check dependencies
cmake --version  # Should be 3.21+
g++ --version    # Should support C++20

# Clean and rebuild
rm -rf build
./install.sh
```

## 🔗 Useful Links

- **Full Installation Guide**: [INSTALL.md](INSTALL.md)
- **Main Documentation**: [README.md](README.md)
- **Technical Specs**: [specifications.md](specifications.md)
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)

## 💡 Tips

1. **Use Workspaces**: Group related projects for easy management
2. **Pin Common Scripts**: Mark frequently used scripts as favorites
3. **System Tray**: Minimize to tray to keep ZenRunner running in background
4. **Keyboard Shortcuts**: Learn shortcuts for faster workflow (see UI)
5. **Log Search**: Use Ctrl+F in log console to find specific entries

## 🎓 Example Workflow

```bash
# 1. Clone and install ZenRunner
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner
./install.sh

# 2. Launch ZenRunner
./zenrunner.sh

# 3. Import your projects
# - Click "Add Project"
# - Select your project folder
# - ZenRunner detects package.json automatically

# 4. Create a workspace (optional)
# - Click "New Workspace"
# - Name it (e.g., "My Web App")
# - Add your frontend and backend projects
# - Click "Start All" to launch both

# 5. Enjoy development!
# - Minimal resource usage (< 30 MB RAM)
# - 60 FPS smooth UI
# - Native OS integration
# - Live log streaming
```

## 🤝 Get Help

- **Documentation**: Check [INSTALL.md](INSTALL.md) for detailed guides
- **Issues**: [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)
- **Discussions**: [GitHub Discussions](https://github.com/marcucus/ZenRunner/discussions)

---

**Ready to boost your development workflow? Start using ZenRunner today! 🚀**
