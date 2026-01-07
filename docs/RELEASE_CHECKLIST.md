# ZenRunner Release Checklist

This checklist should be followed when preparing a new release of ZenRunner.

## Pre-Release Preparation

### Version Management
- [ ] Update version number in `CMakeLists.txt` (PROJECT VERSION)
- [ ] Update version in `packaging/windows/installer.nsi` (VERSIONMAJOR, VERSIONMINOR, VERSIONBUILD)
- [ ] Update version in `packaging/macos/Info.plist` (CFBundleVersion, CFBundleShortVersionString)
- [ ] Update version in packaging scripts if hardcoded
- [ ] Create a `CHANGELOG.md` entry for this release

### Code Quality
- [ ] Run all unit tests: `cd build && ctest`
- [ ] Run manual integration tests
- [ ] Check for compiler warnings
- [ ] Run static analysis tools
- [ ] Run CodeQL security scanning
- [ ] Review code for TODOs and FIXMEs

### Security Review
- [ ] Review [SECURITY.md](SECURITY.md) guidelines
- [ ] Run security validation script: `./scripts/validate-security.sh`
- [ ] Verify no hardcoded credentials or secrets
- [ ] Verify no elevated privilege requirements
- [ ] Test process isolation works correctly
- [ ] Verify file system access is limited to user directories
- [ ] Check input validation on all user inputs
- [ ] Review ANSI escape sequence handling for safety
- [ ] Test on restricted user account

### Documentation
- [ ] Update README.md with new features
- [ ] Update INSTALL.md if build process changed
- [ ] Update docs/PACKAGING.md if packaging changed
- [ ] Review and update all documentation for accuracy
- [ ] Check all documentation links work

### Dependencies
- [ ] Check for Qt framework updates
- [ ] Review dependency licenses
- [ ] Check for known vulnerabilities in dependencies
- [ ] Update third-party acknowledgments if needed

## Build Phase

### Clean Build
- [ ] Delete build directory
- [ ] Perform clean build: `mkdir build && cd build`
- [ ] Build in Release mode: `cmake .. -DCMAKE_BUILD_TYPE=Release`
- [ ] Verify no debug symbols: `file bin/ZenRunner`
- [ ] Check binary size is reasonable

### Platform-Specific Builds

#### Windows
- [ ] Build on Windows with Visual Studio or MinGW
- [ ] Run `windeployqt` to include Qt dependencies
- [ ] Create NSIS installer: `cd packaging/windows && .\build-installer.ps1`
- [ ] Test installer on clean Windows VM
- [ ] Verify application launches from Start Menu
- [ ] Test uninstaller
- [ ] Check Add/Remove Programs entry

#### macOS
- [ ] Build on macOS with Xcode tools
- [ ] Run `macdeployqt` to bundle Qt frameworks
- [ ] Create DMG: `cd packaging/macos && ./build-dmg.sh`
- [ ] Test DMG on clean macOS system
- [ ] Verify drag-and-drop installation works
- [ ] Test first launch (Gatekeeper)
- [ ] Check menu bar integration

#### Linux
- [ ] Build on Linux with GCC/Clang
- [ ] Create DEB package: `cd build && cpack -G DEB`
- [ ] Create RPM package: `cd build && cpack -G RPM`
- [ ] Create TGZ archive: `cd build && cpack -G TGZ`
- [ ] Test DEB installation on Ubuntu/Debian
- [ ] Test RPM installation on Fedora/RHEL
- [ ] Test TGZ extraction and execution

## Code Signing (Production Releases)

### Windows
- [ ] Obtain code signing certificate
- [ ] Sign installer: `signtool sign /f cert.pfx ZenRunner-Setup-*.exe`
- [ ] Verify signature: `signtool verify /pa ZenRunner-Setup-*.exe`
- [ ] Test signed installer

### macOS
- [ ] Sign app bundle: `codesign --sign "Developer ID" ZenRunner.app`
- [ ] Verify signature: `codesign --verify --verbose ZenRunner.app`
- [ ] Notarize DMG: `xcrun notarytool submit ZenRunner.dmg`
- [ ] Staple ticket: `xcrun stapler staple ZenRunner.dmg`
- [ ] Verify stapled ticket: `xcrun stapler validate ZenRunner.dmg`

### Linux
- [ ] Sign packages with GPG (optional)
- [ ] Create detached signatures

## Testing Phase

### Functional Testing
- [ ] Test project import (package.json detection)
- [ ] Test process start/stop/restart
- [ ] Test log viewing and ANSI colors
- [ ] Test workspace creation and management
- [ ] Test parallel vs sequential execution
- [ ] Test system tray integration
- [ ] Test native effects (Mica on Windows, Vibrancy on macOS)
- [ ] Test notch detection on macOS (if available)

### Platform-Specific Testing

#### Windows
- [ ] Test on Windows 10
- [ ] Test on Windows 11
- [ ] Test with Windows Defender active
- [ ] Test Mica effect
- [ ] Test system tray icons and menu

#### macOS
- [ ] Test on macOS 10.15+
- [ ] Test on Intel Mac
- [ ] Test on Apple Silicon Mac
- [ ] Test Vibrancy effect
- [ ] Test notch integration (MacBook Pro)
- [ ] Test menu bar integration

#### Linux
- [ ] Test on Ubuntu/Debian
- [ ] Test on Fedora/RHEL
- [ ] Test on Arch Linux
- [ ] Test X11 and Wayland
- [ ] Test different desktop environments (GNOME, KDE, XFCE)

### Performance Testing
- [ ] Memory usage < 30MB under load
- [ ] Memory usage < 15MB at idle
- [ ] UI maintains 60 FPS
- [ ] Log buffer stays within 5000 lines
- [ ] No memory leaks (run for extended period)

### Security Testing
- [ ] Cannot write outside user directories
- [ ] Cannot modify system files
- [ ] Cannot escalate privileges
- [ ] Process isolation verified
- [ ] Input sanitization works
- [ ] No code injection vulnerabilities

## Distribution Phase

### Package Preparation
- [ ] Generate SHA256 checksums for all packages
- [ ] Create checksums file: `sha256sum ZenRunner-* > SHA256SUMS`
- [ ] Sign checksums file (GPG)
- [ ] Prepare release notes
- [ ] Screenshot new features for announcement

### GitHub Release
- [ ] Create Git tag: `git tag -a v1.0.0 -m "Release 1.0.0"`
- [ ] Push tag: `git push origin v1.0.0`
- [ ] Create GitHub Release from tag
- [ ] Upload Windows installer (.exe)
- [ ] Upload macOS DMG (.dmg)
- [ ] Upload Linux packages (.deb, .rpm, .tar.gz)
- [ ] Upload SHA256SUMS file
- [ ] Include release notes
- [ ] Mark as pre-release if beta

### Website/Documentation
- [ ] Update website (if applicable)
- [ ] Update download links
- [ ] Publish blog post/announcement
- [ ] Update documentation on GitHub Pages (if used)

### Communication
- [ ] Announce on project website
- [ ] Post on social media (if applicable)
- [ ] Notify mailing list/Discord/forum users
- [ ] Update version in package managers (if applicable)

## Post-Release

### Monitoring
- [ ] Monitor GitHub issues for bug reports
- [ ] Monitor crash reports (if telemetry enabled)
- [ ] Check download statistics
- [ ] Gather user feedback

### Follow-Up
- [ ] Address critical bugs immediately
- [ ] Plan patch release if needed
- [ ] Document known issues
- [ ] Plan next release cycle

## Rollback Procedure

If critical issues are discovered:

1. **Immediate Actions**
   - [ ] Mark release as "broken" on GitHub
   - [ ] Remove download links if severe security issue
   - [ ] Post warning on website/social media

2. **Investigation**
   - [ ] Identify root cause
   - [ ] Assess impact and affected users
   - [ ] Develop fix

3. **Patch Release**
   - [ ] Create hotfix branch
   - [ ] Implement and test fix
   - [ ] Follow abbreviated release process
   - [ ] Release as patch version (e.g., 1.0.1)

4. **Communication**
   - [ ] Notify all users of issue and fix
   - [ ] Document in changelog
   - [ ] Update security advisory if needed

## Version Numbering

ZenRunner follows semantic versioning (MAJOR.MINOR.PATCH):

- **MAJOR**: Breaking changes or major feature overhaul
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, security patches

Examples:
- 1.0.0 → 1.0.1: Bug fix
- 1.0.1 → 1.1.0: New feature
- 1.1.0 → 2.0.0: Breaking change

## Notes

- Keep this checklist updated with lessons learned from each release
- Automate as much as possible (CI/CD pipelines)
- Document any deviations from this checklist
- Review and improve process after each release

## References

- [Semantic Versioning](https://semver.org/)
- [Keep a Changelog](https://keepachangelog.com/)
- [Apple Notarization Guide](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [Microsoft Code Signing](https://docs.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools)
