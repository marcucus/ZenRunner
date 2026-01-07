# Quick Packaging Guide

This is a quick reference for creating distributable packages. For complete details, see [PACKAGING.md](PACKAGING.md).

## Prerequisites

Make sure you have built the application first:

```bash
# Linux/macOS
./install.sh --no-install

# Windows
.\install.ps1 -BuildType Release
```

## Windows Installer (.exe)

### Requirements
- NSIS installed from [nsis.sourceforge.io](https://nsis.sourceforge.io/)
- Qt 6 installed

### Build
```powershell
cd packaging\windows
.\build-installer.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"
```

### Output
`build\ZenRunner-Setup-1.0.0.exe`

### Testing
1. Double-click installer
2. Follow installation wizard
3. Launch from Start Menu
4. Verify functionality

## macOS Disk Image (.dmg)

### Requirements
- macOS 10.15+
- Xcode Command Line Tools
- Qt 6 installed

### Build
```bash
cd packaging/macos
./build-dmg.sh
```

### Output
`build/ZenRunner-1.0.0.dmg`

### Testing
1. Mount DMG
2. Drag to Applications
3. Launch from Applications
4. Verify functionality

## Linux Packages

### DEB (Ubuntu/Debian)
```bash
cd build
cpack -G DEB
sudo dpkg -i ZenRunner-1.0.0-Linux.deb
```

### RPM (Fedora/RHEL)
```bash
cd build
cpack -G RPM
sudo rpm -i ZenRunner-1.0.0-Linux.rpm
```

### Portable Archive
```bash
cd build
cpack -G TGZ
tar xzf ZenRunner-1.0.0-Linux.tar.gz
```

## Security Checklist

Before distributing:

- [ ] Built in Release mode (no debug symbols)
- [ ] Tested on clean system
- [ ] No hardcoded credentials
- [ ] No elevated privileges required
- [ ] Code signed (for production)

## Troubleshooting

### Windows: "Qt DLLs not found"
Run `windeployqt` on the executable before packaging.

### macOS: "Developer cannot be verified"
Sign and notarize the app, or users can right-click → Open.

### Linux: "Missing dependencies"
Install Qt 6 packages for your distribution.

## Next Steps

- Review [PACKAGING.md](PACKAGING.md) for detailed instructions
- Check [SECURITY.md](SECURITY.md) for security guidelines
- See [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) before releasing

## Support

- Documentation: [docs/](.)
- Issues: [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)
