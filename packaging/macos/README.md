# ZenRunner macOS Packaging

This directory contains all files needed to create a macOS DMG installer for ZenRunner.

## 📁 Files

### Core Files

- **`build-dmg.sh`** - Main script to create the DMG installer
  - Creates the .app bundle
  - Bundles Qt frameworks with macdeployqt
  - Fixes rpaths for proper framework loading
  - Signs the bundle (ad-hoc or with Developer ID)
  - Creates the final compressed DMG

- **`Info.plist`** - macOS application metadata
  - Bundle identifier, version, name
  - Required Qt and system configurations
  - High DPI and dark mode support

- **`entitlements.plist`** - Code signing entitlements
  - Required for Developer ID signing
  - Includes JIT and library validation entitlements for Qt

### Diagnostic Tools

- **`verify-bundle.sh`** - Bundle verification script
  - Checks bundle structure and completeness
  - Verifies Qt frameworks are present
  - Validates rpaths configuration
  - Checks code signature
  - Identifies common issues

- **`DIAGNOSTIC_DMG.md`** - Comprehensive troubleshooting guide
  - Common crash causes and fixes
  - Step-by-step diagnostics
  - Console log interpretation
  - Advanced debugging techniques

## 🚀 Quick Start

### Build a DMG

```bash
# 1. Compile the application first
cd /path/to/ZenRunner
./install.sh --no-install

# 2. Create the DMG
cd packaging/macos
./build-dmg.sh

# Output: ../../build/ZenRunner-1.0.0.dmg
```

### Verify a Bundle

```bash
# Verify the app bundle before packaging
./verify-bundle.sh ../../build/ZenRunner.app

# Or verify after installation
./verify-bundle.sh /Applications/ZenRunner.app
```

## 🔧 Common Tasks

### Test Installation Locally

```bash
# 1. Create DMG
./build-dmg.sh

# 2. Mount DMG
open ../../build/ZenRunner-1.0.0.dmg

# 3. Install by dragging to Applications
# (Or copy for testing: cp -R "/Volumes/ZenRunner 1.0.0/ZenRunner.app" ~/Desktop/)

# 4. Run from terminal to see debug output
~/Desktop/ZenRunner.app/Contents/MacOS/ZenRunner
```

### Fix Rpath Issues

If the app crashes with "Library not loaded" errors:

```bash
# Run verification to identify issues
./verify-bundle.sh ../../build/ZenRunner.app

# If rpaths are missing, add them
install_name_tool -add_rpath "@executable_path/../Frameworks" \
  ../../build/ZenRunner.app/Contents/MacOS/ZenRunner

# Re-sign after modifications
codesign --force --deep --sign - ../../build/ZenRunner.app
```

### Sign with Developer ID

```bash
# 1. Ensure you have a Developer ID certificate installed
security find-identity -v -p codesigning

# 2. Run build-dmg.sh and answer 'y' when prompted
./build-dmg.sh
# > Do you want to sign the application? (y/N): y

# 3. For notarization (required for public distribution)
xcrun notarytool submit ../../build/ZenRunner-1.0.0.dmg \
  --apple-id your@email.com \
  --team-id YOUR_TEAM_ID \
  --password app-specific-password \
  --wait

# 4. Staple the notarization ticket
xcrun stapler staple ../../build/ZenRunner-1.0.0.dmg
```

## 🐛 Troubleshooting

### App Crashes After Installation

1. **Run verification script:**
   ```bash
   ./verify-bundle.sh /Applications/ZenRunner.app
   ```

2. **Check Console logs:**
   ```bash
   # In one terminal:
   log stream --predicate 'processImagePath contains "ZenRunner"' --level debug
   
   # In another:
   open /Applications/ZenRunner.app
   ```

3. **Read the diagnostic guide:**
   ```bash
   cat DIAGNOSTIC_DMG.md
   # Or open in browser: open DIAGNOSTIC_DMG.md
   ```

### Missing Qt Frameworks

```bash
# List what macdeployqt bundled
ls -la ../../build/ZenRunner.app/Contents/Frameworks/

# Re-run macdeployqt manually
/opt/homebrew/opt/qt@6/bin/macdeployqt ../../build/ZenRunner.app \
  -qmldir=../../src/ui \
  -always-overwrite \
  -verbose=1
```

### Gatekeeper Blocks App

For local testing:
```bash
# Remove quarantine attribute
xattr -cr /Applications/ZenRunner.app

# Or use right-click > Open in Finder
```

For distribution:
- Sign with Developer ID certificate
- Notarize with Apple (see above)

## 📚 Documentation

- **[CREER_DMG.md](../../CREER_DMG.md)** - Complete DMG creation guide (French)
- **[DIAGNOSTIC_DMG.md](DIAGNOSTIC_DMG.md)** - Troubleshooting guide (French)
- **[PACKAGING.md](../../docs/PACKAGING.md)** - General packaging documentation
- **[PACKAGING_IMPLEMENTATION.md](../../PACKAGING_IMPLEMENTATION.md)** - Implementation details

## 🔗 Resources

- [Qt macOS Deployment](https://doc.qt.io/qt-6/macos-deployment.html)
- [Apple Code Signing Guide](https://developer.apple.com/documentation/xcode/code-signing-your-app)
- [Apple Notarization Guide](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [macdeployqt Documentation](https://doc.qt.io/qt-6/macos-deployment.html#using-the-macdeploy-tool)

## ⚙️ Requirements

- macOS 10.15 or later
- Xcode Command Line Tools: `xcode-select --install`
- Qt 6.2+ with macdeployqt: `brew install qt@6`
- CMake 3.21+: `brew install cmake`

## 📋 Build System Integration

This packaging is integrated with:
- **CMakeLists.txt**: MACOSX_BUNDLE configuration
- **CPack**: DMG generation via `cpack -G DragNDrop`
- **install.sh**: Build system for the application

## 💡 Tips

1. **Always test on a different Mac** - Ensures portability
2. **Check bundle size** - Should be ~20-30 MB uncompressed
3. **Verify on multiple macOS versions** - 10.15, 11.0, 12.0+
4. **Use verify-bundle.sh before distribution** - Catches issues early
5. **Keep build logs** - Useful for debugging

## 🔄 Workflow

```
1. Build Application → 2. Create Bundle → 3. Verify Bundle
                              ↓
                       4. Sign Bundle → 5. Create DMG
                              ↓
                       6. Test DMG → 7. Notarize (optional)
                              ↓
                       8. Distribute
```

## 📝 Notes

- **Ad-hoc signing** is sufficient for local use
- **Developer ID signing** required for distribution outside App Store
- **Notarization** required for macOS 10.15+ to avoid Gatekeeper warnings
- The build-dmg.sh script handles most common issues automatically
- Use verify-bundle.sh to catch problems before creating the DMG

---

**Need Help?** See [DIAGNOSTIC_DMG.md](DIAGNOSTIC_DMG.md) or open a [GitHub issue](https://github.com/marcucus/ZenRunner/issues).
