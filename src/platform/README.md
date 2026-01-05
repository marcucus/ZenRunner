# Platform Module

This module contains OS-specific implementations and native API integrations.

## Responsibilities

- **OS-Specific Wrappers**: Platform-dependent features abstracted behind common interfaces
- **Native Integration**: Deep system integration for optimal performance and appearance
- **System Utilities**: Helper functions for platform-specific operations

## Key Components (Planned)

### Windows
- `MicaEffect`: Mica material integration via DWM API
- `Win32Utils`: Windows-specific utilities

### macOS
- `VibrancyEffect`: NSVisualEffectView integration
- `NotchDetection`: Safe area detection for MacBook Pro notch
- `MacOSUtils`: macOS-specific utilities

### Cross-Platform
- `SystemTray`: Unified system tray/menu bar interface
- `NativeNotifications`: System notification integration

## Design Principles

- Abstract platform differences behind clean interfaces
- Conditional compilation for platform-specific code
- Minimal runtime overhead
- Graceful degradation on unsupported platforms
