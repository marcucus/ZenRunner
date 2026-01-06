# ZenRunner Packaging Scripts

This directory contains scripts and configuration files for creating distributable packages of ZenRunner.

## Directory Structure

```
packaging/
├── windows/
│   ├── installer.nsi           # NSIS installer script
│   └── build-installer.ps1     # PowerShell script to build Windows installer
└── macos/
    ├── Info.plist              # macOS app bundle configuration
    └── build-dmg.sh            # Shell script to build macOS DMG
```

## Quick Start

### Windows

```powershell
# Build the application first
.\install.ps1 -BuildType Release

# Create installer
cd packaging\windows
.\build-installer.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"
```

Output: `build\ZenRunner-Setup-1.0.0.exe`

### macOS

```bash
# Build the application first
./install.sh --no-install

# Create DMG
cd packaging/macos
./build-dmg.sh
```

Output: `build/ZenRunner-1.0.0.dmg`

## Requirements

### Windows

- NSIS (Nullsoft Scriptable Install System)
- PowerShell 5.1 or later
- Qt 6.2+ installed

### macOS

- Xcode Command Line Tools
- macdeployqt (included with Qt)
- Qt 6.2+ installed

## Documentation

For detailed packaging instructions, see [docs/PACKAGING.md](../docs/PACKAGING.md)

## Security

All packaging scripts follow security best practices:

- No elevated privileges required for installation
- Sandboxed process execution
- Limited file system access
- No system modifications

See [docs/SECURITY.md](../docs/SECURITY.md) for complete security guidelines.

## Support

For issues or questions:

- Documentation: [docs/PACKAGING.md](../docs/PACKAGING.md)
- Issues: [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)
