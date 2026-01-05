# ZenRunner

A high-performance native process manager for developers, built with C++20 and Qt 6.

## Vision

ZenRunner is a lightweight alternative to Electron-based development tools, designed to manage development processes with minimal resource consumption (< 30MB RAM) while providing a modern glassmorphism UI and deep OS integration.

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
- Windows: Mica effect integration
- macOS: Vibrancy effects and Notch detection
- Cross-platform: System tray and notifications

### `/include/`
Public APIs and shared definitions:
- Module interfaces exposed to other components
- Common types and enumerations
- Forward declarations for reduced dependencies

## Key Features (Planned)

- ⚡ **Ultra-lightweight**: < 30MB RAM usage
- 🎨 **Native UI**: Glassmorphism with OS-specific materials (Mica/Vibrancy)
- 📦 **Smart Detection**: Automatic package.json script discovery
- 🔄 **Workspace Management**: Group and control multiple projects
- 📊 **Live Logs**: ANSI color support with circular buffering
- 🌐 **System Integration**: Tray icon, notifications, native effects

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
| UI Framerate | 60 FPS | GPU-accelerated Qt Quick Scene Graph |
| Response Time | < 10 ms | Asynchronous signal/slot architecture |

## Design Principles

1. **Modularity**: Each component is isolated for maintainability and future growth
2. **Performance**: Native code with manual memory management
3. **Native Integration**: Deep OS-level features, not web wrappers
4. **Developer Focus**: Optimized for development workflow efficiency

## Building

```bash
# Prerequisites: Qt 6.x, CMake 3.21+, C++20 compiler

mkdir build && cd build
cmake ..
cmake --build .
```

## Development Status

🚧 **In Development** - Initial directory structure established

See [specifications.md](specifications.md) for detailed technical documentation.

## License

TBD
