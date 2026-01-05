# QML UI Architecture and Design

## Overview

ZenRunner's frontend is built entirely in QML following a declarative approach with glassmorphism design principles. The UI achieves optimal performance through GPU acceleration and efficient memory management.

## Architecture Principles

### 1. Separation of Concerns
- **Presentation Layer**: Pure QML components in `src/ui/`
- **Business Logic**: C++ backend (to be integrated)
- **Platform Integration**: Native effects in `src/platform/`

### 2. Component Hierarchy

```
Main.qml (Application Window)
├── Dashboard.qml (Primary View)
│   ├── Header (GlassCard)
│   ├── Projects Panel (GlassCard)
│   └── Sidebar
│       ├── Workspaces (GlassCard)
│       └── System Status (GlassCard)
├── ProjectView.qml (Project Details)
├── LogConsole.qml (Terminal Output)
└── WorkspaceManager.qml (Workspace Control)
```

### 3. Reusable Components

#### GlassCard.qml
A versatile container with glassmorphism styling:
- Translucent background (opacity: 0.15)
- Subtle border with gradient
- Inner highlight for depth
- Hover effects for interactivity
- Configurable blur and corner radius

#### GlassButton.qml
Action buttons with glass aesthetic:
- Smooth hover transitions
- Accent color indicators
- Pressed state feedback
- Disabled state handling

## Glassmorphism Design System

### Visual Properties
- **Background**: Semi-transparent with blur effect
- **Borders**: 1px with 30% opacity
- **Corner Radius**: 8-16px depending on component
- **Color Palette**: Dark theme optimized for developers
  - Primary: `#4a90e2` (Blue)
  - Secondary: `#7c4dff` (Purple)
  - Success: `#4ade80` (Green)
  - Warning: `#fbbf24` (Yellow)
  - Error: `#ef4444` (Red)

### Typography
- **Title**: 28px Bold, White
- **Subtitle**: 18px DemiBold, White
- **Body**: 14px Regular, #888888
- **Monospace**: 12px (for logs and code)

## Performance Optimizations

### 1. Memory Efficiency
- Minimal object hierarchy depth
- ListView with `cacheBuffer` for log scrolling
- Conditional rendering of expensive effects
- Lazy loading of non-visible content

### 2. GPU Acceleration
- Qt Quick Scene Graph for 60 FPS rendering
- Hardware-accelerated blending and compositing
- Native platform effects (Mica/Vibrancy) offload work to OS

### 3. Animation Performance
- Use of `Behavior` for smooth property transitions
- Sequential animations for particle effects
- Easing curves for natural motion (Easing.OutQuad, Easing.InOutSine)

## Native Platform Integration

### Windows 11 - Mica Effect
Implemented in `WindowsNativeEffects.cpp`:
- Uses DWM (Desktop Window Manager) API
- `DWMWA_SYSTEMBACKDROP_TYPE` attribute
- Samples desktop wallpaper for background
- Automatic theme following (light/dark)

### macOS - Vibrancy Effect
Implemented in `MacOSNativeEffects.mm`:
- Uses `NSVisualEffectView`
- Material: `NSVisualEffectMaterialHUDWindow`
- Follows window active state
- Behind-window blending mode
- Notch detection via `safeAreaInsets`

### Generic Fallback
For Linux and older OS versions:
- Pure QML glassmorphism effects
- Software-based blur (when GPU available)
- Graceful degradation

## Component Details

### Dashboard.qml
Main overview interface featuring:
- **Header**: Application title and quick action buttons
- **Projects Panel**: List of imported projects with status
- **Workspaces Panel**: Grouped projects management
- **System Status**: Real-time performance metrics
  - Memory usage indicator
  - Active process count
  - UI framerate monitor

### ProjectView.qml
Detailed project view with:
- Project metadata (name, path, status)
- Available scripts grid with color-coded categories
- Quick action buttons (logs, terminal, settings)
- Running state indicator

### LogConsole.qml
Terminal-style log viewer:
- Circular buffer display (5000 lines max)
- ANSI color code support (planned)
- Monospace font for code readability
- Auto-scroll functionality
- Search and clear controls
- Performance metrics in status bar

### WorkspaceManager.qml
Workspace organization interface:
- List of configured workspaces
- Project count indicators
- Color-coded workspace identification
- Bulk start/stop controls
- Active state monitoring

## Responsive Design

The UI adapts to different window sizes:
- Minimum size: 800x600
- Optimal size: 1200x800
- Flexible grid layouts
- Panels collapse gracefully on smaller screens

## Accessibility

- High contrast text on dark backgrounds
- Color indicators paired with text labels
- Hover feedback for interactive elements
- Keyboard navigation support (planned)

## Future Enhancements

1. **GLSL Shaders**: Custom blur and glow effects
2. **Animations**: Page transitions and micro-interactions
3. **Theming**: User-customizable color schemes
4. **Data Binding**: Integration with C++ backend models
5. **Drag & Drop**: Project reordering and workspace management
6. **Context Menus**: Right-click actions for power users

## Build Requirements

To build and run ZenRunner UI:

### Prerequisites
- **Qt 6.5+**: Required for QML and QtQuick
- **CMake 3.21+**: Build system
- **C++20 Compiler**: 
  - GCC 10+ / Clang 12+ (Linux/macOS)
  - MSVC 2019+ (Windows)

### Platform-Specific
- **Windows**: Windows SDK 10.0.22000+ (for Mica)
- **macOS**: macOS 10.15+ SDK
- **Linux**: X11 or Wayland development libraries

### Build Steps
```bash
mkdir build && cd build
cmake ..
cmake --build .
./ZenRunner
```

## Testing Strategy

1. **Visual Testing**: Manual verification of glassmorphism effects
2. **Performance Testing**: FPS monitoring during animations
3. **Memory Testing**: Verify < 30MB target with multiple projects
4. **Platform Testing**: Test native effects on Windows 11 and macOS

## Conclusion

The QML UI implementation achieves the project goals:
- ✅ Declarative approach with clear separation of concerns
- ✅ Glassmorphism aesthetic with native material effects
- ✅ Optimized hierarchy for memory efficiency
- ✅ GPU acceleration for 60 FPS performance
- ✅ Platform-specific integration (Mica/Vibrancy)
