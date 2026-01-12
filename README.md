# ZenRunner

A high-performance native process manager for developers, built with C++20 and Qt 6.

## Vision

ZenRunner is a lightweight alternative to Electron-based development tools, designed to manage development processes with minimal resource consumption (< 30MB RAM) while providing a modern glassmorphism UI and deep OS integration.

## Quick Start

### Installation

#### Linux / macOS
```bash
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner
chmod +x install.sh
./install.sh
```

#### Windows (PowerShell)
```powershell
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner
.\install.ps1
```

### Launching ZenRunner

After installation, launch ZenRunner using one of these methods:

#### Using Launcher Script (Recommended)
```bash
# Linux / macOS
./zenrunner.sh

# Windows
.\zenrunner.bat
```

#### Direct Execution
```bash
# If installed system-wide
ZenRunner

# If installed to user directory
~/.local/bin/ZenRunner  # Linux / macOS
%LOCALAPPDATA%\Programs\ZenRunner\ZenRunner.exe  # Windows
```

For detailed installation instructions, troubleshooting, and dependency information, see:
- **[QUICKSTART.md](QUICKSTART.md)** - Fast-track guide to get running quickly
- **[INSTALL.md](INSTALL.md)** - Complete installation and troubleshooting guide

## Project Structure

The project follows a modular architecture with clear separation of concerns:

```
ZenRunner/
├── src/
│   ├── core/           # Process management, data models, JSON parsing
│   ├── ui/             # QML files, GLSL shaders, UI JavaScript
│   └── platform/       # OS-specific wrappers and utilities
├── include/            # Public headers and shared type definitions
│   ├── core/           # Core module public headers
│   ├── platform/       # Platform abstraction interfaces
│   └── types/          # Common type definitions
├── specifications.md   # Detailed technical specifications
└── CMakeLists.txt      # Build configuration
```

## Module Organization

### `/src/core/`
Core business logic and data management:
- Process lifecycle management using QProcess
- Project and Workspace data models
- JSON parsing for package.json detection
- Circular buffer for efficient log storage

### `/src/ui/`
User interface components:
- QML declarative UI components
- GLSL shaders for visual effects
- JavaScript for animations and UI logic
- Glassmorphism design system

### `/src/platform/`
OS-specific implementations:
- Windows: Mica effect integration via DWM API
- macOS: Vibrancy effects via NSVisualEffectView and Notch detection
- Cross-platform: System tray with real-time status icons and context menu
- Platform helpers for QML integration

### `/include/`
Public APIs and shared definitions:
- Module interfaces exposed to other components
- Common types and enumerations
- Forward declarations for reduced dependencies

## Key Features

- ⚡ **Ultra-lightweight**: < 30MB RAM usage
- 🎨 **Native UI**: Glassmorphism with OS-specific materials (Mica/Vibrancy)
- 📦 **Smart Detection**: Automatic package.json script discovery
- 🗂️ **Workspace Management**: Group projects and launch with batch actions (sequential/parallel)
- 🔄 **Process Control**: Start, stop, and monitor multiple development processes
- 📊 **Live Logs**: ANSI color support with circular buffering (5000 lines per project)
- 🛡️ **Log Throttling**: Intelligent UI protection against log floods (99%+ update reduction)
- 🌐 **System Integration**: 
  - Real-time system tray with color-coded status (Gray/Green/Red/Orange)
  - Native notifications with click handling
  - Context menu with quick actions
  - Minimize to tray functionality
- 🍎 **macOS Notch Support**: Dynamic Island-like quick control positioning
- 🪟 **Windows 11 Mica**: Native translucent background effect

## Workspace System

ZenRunner includes a powerful workspace system for managing multiple projects as a unit:

- **Group related projects** (Frontend, Backend, Database, Tools)
- **One-click batch actions**: Start all projects with a single button
- **Execution modes**: 
  - **Parallel**: Start all projects simultaneously for maximum speed
  - **Sequential**: Start projects in order when there are dependencies
- **Visual organization**: Color-coded workspaces with status indicators
- **Persistent storage**: Workspaces are saved automatically

See [Workspace Guide](docs/WORKSPACE_GUIDE.md) for detailed documentation.

## Technology Stack

- **C++20**: Modern C++ with zero-cost abstractions
- **Qt 6**: Cross-platform framework with hardware-accelerated rendering
- **QML**: Declarative UI with 60 FPS performance
- **CMake**: Build system with modular configuration

## Performance Targets

| Metric | Target | Strategy |
|--------|--------|----------|
| RAM (Idle) | < 15 MB | Static allocation, lazy loading |
| RAM (Active) | < 30 MB | Circular buffers, efficient data structures |
| UI Framerate | 60 FPS | GPU-accelerated Qt Quick Scene Graph with RHI |
| Response Time | < 10 ms | Asynchronous signal/slot architecture |
| Terminal Output | No freezing | ListView virtualization, chunk-based reading |

**🎯 60 FPS Optimization**: ZenRunner implements comprehensive GPU acceleration using Qt's Rendering Hardware Interface (RHI), ensuring consistent 60 FPS performance with:
- Vulkan on Linux, Metal on macOS, Direct3D on Windows
- Threaded render loop for parallel CPU/GPU processing
- Optimized QML layer compositing and caching
- See [docs/PERFORMANCE.md](docs/PERFORMANCE.md) for details

**⚡ Terminal Optimization**: The terminal view uses ListView virtualization instead of TextEdit to handle massive log output without freezing:
- 64KB chunk-based reading prevents UI blocking
- Only visible log lines are rendered (virtualization)
- Aggressive 2000-line UI buffer with efficient pruning
- See [PERFORMANCE_OPTIMIZATION_TERMINAL.md](PERFORMANCE_OPTIMIZATION_TERMINAL.md) for details

## Design Principles

1. **Modularity**: Each component is isolated for maintainability and future growth
2. **Performance**: Native code with manual memory management
3. **Native Integration**: Deep OS-level features, not web wrappers
4. **Developer Focus**: Optimized for development workflow efficiency

## Native OS Integration

ZenRunner integrates deeply with native operating system APIs for optimal performance and user experience:

### Windows 11
- **Mica Material**: Uses DWM (Desktop Window Manager) API to apply Mica effect
- Samples desktop wallpaper for dynamic, translucent background
- Automatically adapts to light/dark theme changes
- Available in standard and alt (darker) variants

### macOS
- **Vibrancy Effect**: Uses NSVisualEffectView for native blur and translucency
- **Notch Detection**: Detects MacBook Pro notch via safeAreaInsets API
- **Dynamic Island-like Controls**: QuickControlBar positions itself around the notch
- Automatic dark/light mode adaptation

### System Tray / Menu Bar
- **Cross-platform**: Works on Windows, macOS, and Linux
- **Real-time Status**: Color-coded icons (Gray/Green/Red/Orange)
  - Gray: Idle (no processes running)
  - Green: Active (processes running normally)
  - Red: Error (process crashed)
  - Orange: Warning state
- **Context Menu**: Quick actions for show/hide, workspace control, and quit
- **Native Notifications**: OS-native notification system with click handling
- **Minimize to Tray**: Run in background without closing

See [docs/NATIVE_INTEGRATION.md](docs/NATIVE_INTEGRATION.md) for detailed integration guide and API reference.

## Building from Source

### Quick Build (Manual)

```bash
# Prerequisites: Qt 6.2+, CMake 3.21+, C++20 compiler

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./bin/ZenRunner
```

### Automated Build & Install (Recommended)

Use the provided installation scripts for an automated build and installation process:

```bash
# Linux / macOS
./install.sh

# Windows (PowerShell)
.\install.ps1
```

The installation scripts will:
- Check and guide you through installing dependencies
- Build the application with optimizations
- Install to your system or user directory
- Create launcher scripts for easy access

For detailed build instructions, dependency installation, and troubleshooting, see:
- [INSTALL.md](INSTALL.md) - Complete installation guide
- [docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md) - Build and performance testing
- [docs/PACKAGING.md](docs/PACKAGING.md) - Creating distributable packages (.exe, .dmg)

## Distribution Packages

ZenRunner can be packaged for easy distribution on all platforms:

### Windows (.exe)
Professional NSIS installer with:
- Installation wizard (English/French)
- Start Menu and Desktop shortcuts
- Add/Remove Programs integration
- Optional PATH addition

```powershell
cd packaging\windows
.\build-installer.ps1
```

### macOS (.dmg)
Disk image with .app bundle:
- Drag-and-drop installation
- Native macOS integration
- Code signing support
- Notarization ready

```bash
cd packaging/macos
./build-dmg.sh
```

📖 **Guide complet en français** : [CREER_DMG.md](CREER_DMG.md)

### Linux
Multiple package formats:
- DEB (Ubuntu/Debian)
- RPM (Fedora/RHEL)
- TGZ (portable)

```bash
cd build
cpack -G DEB  # or RPM, TGZ
```

For complete packaging instructions and security considerations, see [docs/PACKAGING.md](docs/PACKAGING.md).

## Documentation

### Core Features
- [Workspace Guide](docs/WORKSPACE_GUIDE.md) - Multi-project workspace management
- [Circular Buffer Implementation](CIRCULAR_BUFFER_IMPLEMENTATION.md) - Memory-efficient log storage
- [Log Throttling System](LOG_THROTTLING_IMPLEMENTATION.md) - UI protection against log floods
- [Terminal Performance Optimization](PERFORMANCE_OPTIMIZATION_TERMINAL.md) - ListView virtualization and chunk-based reading
- [Native Integration](docs/NATIVE_INTEGRATION.md) - OS-specific features and APIs
- [Performance Guide](docs/PERFORMANCE.md) - 60 FPS optimization details

### Packaging and Distribution
- [Packaging Guide](docs/PACKAGING.md) - Creating distributable packages
- [Security Guidelines](docs/SECURITY.md) - Application security and safety measures

### Technical Specifications
- [specifications.md](specifications.md) - Complete technical documentation
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture overview

## Development Status

🚧 **In Development** - Initial directory structure established

See [specifications.md](specifications.md) for detailed technical documentation.

## License

MIT License - See [LICENSE.txt](LICENSE.txt) for details.
