# ZenRunner Packaging Guide

This document provides comprehensive instructions for creating distributable packages of ZenRunner for Windows (.exe), macOS (.dmg), and Linux (AppImage, DEB, RPM).

## Table of Contents

- [Overview](#overview)
- [Security Considerations](#security-considerations)
- [Prerequisites](#prerequisites)
- [Windows Packaging (.exe)](#windows-packaging-exe)
- [macOS Packaging (.dmg)](#macos-packaging-dmg)
- [Linux Packaging](#linux-packaging)
- [Code Signing](#code-signing)
- [Distribution Checklist](#distribution-checklist)

## Overview

ZenRunner supports multiple packaging formats for easy distribution:

- **Windows**: NSIS installer (.exe), portable ZIP
- **macOS**: DMG disk image with .app bundle
- **Linux**: DEB packages (Ubuntu/Debian), RPM packages (Fedora/RHEL), TGZ archives

All packages are configured to ensure the application cannot cause damage to the user's system by following platform-specific security best practices.

## Security Considerations

Before packaging, review the [Security Guidelines](docs/SECURITY.md) to ensure:

1. **No Elevated Privileges**: Application runs with standard user permissions
2. **Sandboxed Processes**: Child processes are isolated through OS APIs
3. **Limited File Access**: Only reads user-selected directories and writes to user data directories
4. **No System Modifications**: Does not modify system files or configurations
5. **Input Validation**: All user inputs are validated and sanitized

### Security Checklist Before Release

- [ ] Review code for potential security vulnerabilities
- [ ] Run security scanners (CodeQL, static analysis)
- [ ] Test with restricted user accounts
- [ ] Verify no elevated permission requests
- [ ] Check file system access is limited to user directories
- [ ] Ensure no network connections (except child processes)
- [ ] Test process isolation and termination

## Prerequisites

### All Platforms

- Git
- CMake 3.21+
- Qt 6.2+
- C++20 compatible compiler

### Windows Additional Requirements

- **NSIS (Nullsoft Scriptable Install System)**: Download from [nsis.sourceforge.io](https://nsis.sourceforge.io/)
- **Visual Studio 2019+** or **MinGW**
- **Optional**: Code signing certificate for production releases

### macOS Additional Requirements

- **Xcode Command Line Tools**: `xcode-select --install`
- **macdeployqt**: Included with Qt installation
- **Optional**: Apple Developer account for code signing and notarization

### Linux Additional Requirements

- Standard build tools (gcc/clang, make)
- Platform-specific package tools:
  - **DEB**: `dpkg-deb`, `fakeroot`
  - **RPM**: `rpmbuild`
  - **AppImage**: `appimagetool`, `linuxdeploy`

## Windows Packaging (.exe)

### Method 1: Using NSIS (Recommended)

The custom NSIS script provides a professional installer with:
- Installation wizard with multiple languages (English, French)
- Optional components (Start Menu, Desktop shortcut, PATH)
- Uninstaller
- Registry integration for Add/Remove Programs

#### Steps:

1. **Build the Application**:
   ```powershell
   .\install.ps1 -BuildType Release
   ```

2. **Prepare Qt Dependencies**:
   ```powershell
   # If not already copied, use windeployqt
   cd build\bin\Release
   windeployqt ZenRunner.exe
   cd ..\..\..
   ```

3. **Build the Installer**:
   ```powershell
   cd packaging\windows
   .\build-installer.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"
   ```

4. **Output**: `build\ZenRunner-Setup-1.0.0.exe`

#### Optional: Custom Icons

To use custom icons for the installer:

1. Create an icon file: `packaging/windows/icon.ico` (256x256 recommended)
2. The build scripts will automatically use it if present
3. If not present, default NSIS icons will be used

### Method 2: Using CPack

CPack provides automatic packaging integrated with CMake:

```powershell
cd build
cpack -G NSIS
```

### Testing the Windows Installer

1. **Install**: Double-click the installer and follow the wizard
2. **Verify**: Launch from Start Menu or Desktop shortcut
3. **Test Functionality**: Run sample projects, check logs
4. **Uninstall**: Use Add/Remove Programs or the uninstaller

### Windows Installer Features

- **User-level Installation**: No administrator rights required
- **Silent Installation**: `ZenRunner-Setup-1.0.0.exe /S`
- **Custom Installation**: Select components during install
- **Automatic Updates**: Can detect and upgrade previous versions

## macOS Packaging (.dmg)

### Using the Build Script (Recommended)

The provided script creates a professional DMG with:
- .app bundle with proper structure
- Applications folder shortcut
- Attractive window layout
- Optional code signing

#### Steps:

1. **Build the Application**:
   ```bash
   ./install.sh --no-install
   ```

2. **Create DMG**:
   ```bash
   cd packaging/macos
   ./build-dmg.sh
   ```

3. **Output**: `build/ZenRunner-1.0.0.dmg`

#### Optional: Custom Icons and Background

To customize the macOS package appearance:

1. **App Icon**: Create `packaging/macos/ZenRunner.icns` (512x512@2x recommended)
2. **DMG Background**: Create `packaging/macos/background.png` (600x400 recommended)
3. The build scripts will automatically use them if present
4. If not present, default appearance will be used

### Manual .app Bundle Creation

If you need to create the bundle manually:

```bash
# Create bundle structure
mkdir -p ZenRunner.app/Contents/{MacOS,Resources,Frameworks}

# Copy executable
cp build/bin/ZenRunner ZenRunner.app/Contents/MacOS/

# Copy Info.plist
cp packaging/macos/Info.plist ZenRunner.app/Contents/

# Copy icon (if available)
cp packaging/macos/ZenRunner.icns ZenRunner.app/Contents/Resources/

# Deploy Qt frameworks
macdeployqt ZenRunner.app

# Create DMG
hdiutil create -volname "ZenRunner" -srcfolder ZenRunner.app -ov -format UDZO ZenRunner.dmg
```

### Testing the macOS DMG

1. **Mount**: Double-click the DMG file
2. **Install**: Drag ZenRunner to Applications folder
3. **First Launch**: May need to right-click → Open (Gatekeeper)
4. **Verify**: Test functionality with sample projects

### macOS Bundle Structure

```
ZenRunner.app/
├── Contents/
│   ├── Info.plist          # App metadata
│   ├── MacOS/
│   │   └── ZenRunner       # Executable
│   ├── Resources/
│   │   └── ZenRunner.icns  # App icon
│   └── Frameworks/         # Qt frameworks
│       ├── QtCore.framework
│       ├── QtGui.framework
│       ├── QtQuick.framework
│       └── ...
```

## Linux Packaging

### DEB Package (Ubuntu/Debian)

```bash
# Build
./install.sh --no-install

# Create DEB
cd build
cpack -G DEB

# Output: ZenRunner-1.0.0-Linux.deb
```

Install:
```bash
sudo dpkg -i ZenRunner-1.0.0-Linux.deb
sudo apt-get install -f  # Fix dependencies if needed
```

### RPM Package (Fedora/RHEL)

```bash
# Build
./install.sh --no-install

# Create RPM
cd build
cpack -G RPM

# Output: ZenRunner-1.0.0-Linux.rpm
```

Install:
```bash
sudo rpm -i ZenRunner-1.0.0-Linux.rpm
# or
sudo dnf install ZenRunner-1.0.0-Linux.rpm
```

### Portable Archive

```bash
cd build
cpack -G TGZ

# Output: ZenRunner-1.0.0-Linux.tar.gz
```

Extract and run:
```bash
tar xzf ZenRunner-1.0.0-Linux.tar.gz
cd ZenRunner-1.0.0-Linux/bin
./ZenRunner
```

## Code Signing

Code signing is essential for distribution and user trust.

### Windows Code Signing

Requires a code signing certificate from a trusted CA.

```powershell
# Sign the installer
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com ZenRunner-Setup-1.0.0.exe

# Verify signature
signtool verify /pa ZenRunner-Setup-1.0.0.exe
```

### macOS Code Signing

Requires an Apple Developer account and Developer ID certificate.

```bash
# Sign the application
codesign --force --deep --sign "Developer ID Application: Your Name (TEAMID)" ZenRunner.app

# Verify signature
codesign --verify --verbose ZenRunner.app
spctl --assess --verbose ZenRunner.app

# Notarize (required for macOS 10.15+)
xcrun notarytool submit ZenRunner.dmg --apple-id your@email.com --team-id TEAMID --password app-specific-password

# Staple the notarization ticket
xcrun stapler staple ZenRunner.dmg
```

For detailed notarization steps, see [Apple's Notarization Guide](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution).

### Linux

Code signing is optional on Linux but recommended:

```bash
# Sign with GPG
gpg --detach-sign --armor ZenRunner-1.0.0-Linux.deb

# Users can verify with:
gpg --verify ZenRunner-1.0.0-Linux.deb.asc
```

## Distribution Checklist

Before releasing a new package:

### Pre-Build Checks

- [ ] Update version numbers in:
  - [ ] `CMakeLists.txt` (PROJECT_VERSION)
  - [ ] `packaging/windows/installer.nsi` (VERSION*)
  - [ ] `packaging/macos/Info.plist` (CFBundleVersion)
- [ ] Update README.md and CHANGELOG
- [ ] Run all tests and ensure they pass
- [ ] Check for security vulnerabilities (CodeQL scan)
- [ ] Review dependencies for known vulnerabilities
- [ ] Update LICENSE file if needed

### Build Checks

- [ ] Clean build from scratch
- [ ] Build in Release mode with optimizations
- [ ] Verify no debug symbols in release build
- [ ] Check binary size is reasonable
- [ ] Test on clean VM or system

### Package Checks

- [ ] Create packages for all target platforms
- [ ] Verify package metadata (name, version, description)
- [ ] Check file permissions are correct
- [ ] Ensure all dependencies are included
- [ ] Test installation on clean system
- [ ] Test uninstallation leaves no artifacts

### Security Checks

- [ ] No hardcoded credentials or secrets
- [ ] No unnecessary file system access
- [ ] No elevated privilege requirements
- [ ] Process isolation is working
- [ ] Input validation is in place
- [ ] ANSI escape sequences handled safely

### Distribution Checks

- [ ] Code signing completed
- [ ] macOS notarization completed (if applicable)
- [ ] Create SHA256 checksums
- [ ] Test download and installation
- [ ] Update website/GitHub release page
- [ ] Notify users of new release

### Post-Release

- [ ] Monitor for crash reports
- [ ] Monitor for security issues
- [ ] Respond to user feedback
- [ ] Plan next release cycle

## Troubleshooting

### Windows

**Issue**: Installer shows "Unknown Publisher"
- **Solution**: Sign the installer with a valid code signing certificate

**Issue**: Qt DLLs not found
- **Solution**: Run `windeployqt` on the executable or ensure Qt bin directory is in PATH

**Issue**: NSIS compilation fails
- **Solution**: Check NSIS is installed and makensis.exe is in PATH

### macOS

**Issue**: "App is damaged and can't be opened"
- **Solution**: Remove quarantine attribute: `xattr -cr ZenRunner.app`

**Issue**: "Developer cannot be verified"
- **Solution**: Sign and notarize the application, or users can right-click → Open

**Issue**: Qt frameworks not found
- **Solution**: Ensure macdeployqt completed successfully

### Linux

**Issue**: Missing dependencies
- **Solution**: Install Qt 6 packages for your distribution

**Issue**: Library not found
- **Solution**: Check LD_LIBRARY_PATH or use `ldd` to identify missing libraries

## Support

For packaging issues or questions:

- **Documentation**: See [INSTALL.md](../INSTALL.md) for build instructions
- **Security**: See [SECURITY.md](docs/SECURITY.md) for security guidelines
- **Issues**: Report on [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)

## Resources

- [CMake CPack Documentation](https://cmake.org/cmake/help/latest/module/CPack.html)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [macOS App Bundle Structure](https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html)
- [Qt Deployment Guide](https://doc.qt.io/qt-6/deployment.html)
- [Apple Code Signing Guide](https://developer.apple.com/support/code-signing/)
