# Installation Testing Guide

This document provides test cases and verification steps for the ZenRunner installation system.

## Test Matrix

### Platforms to Test

| Platform | Version | Architecture | Status |
|----------|---------|--------------|--------|
| Ubuntu | 20.04 LTS | x86_64 | ⏳ Pending |
| Ubuntu | 22.04 LTS | x86_64 | ⏳ Pending |
| Debian | 11 | x86_64 | ⏳ Pending |
| Fedora | 38 | x86_64 | ⏳ Pending |
| Arch Linux | Rolling | x86_64 | ⏳ Pending |
| macOS | 11 (Big Sur) | x86_64 | ⏳ Pending |
| macOS | 12 (Monterey) | ARM64 | ⏳ Pending |
| macOS | 13 (Ventura) | ARM64 | ⏳ Pending |
| Windows | 10 (21H2) | x86_64 | ⏳ Pending |
| Windows | 11 (22H2) | x86_64 | ⏳ Pending |

## Test Cases

### TC-001: Clean Installation (User Mode)

**Objective**: Verify fresh installation to user directory

**Prerequisites**: 
- All dependencies installed
- No previous ZenRunner installation

**Steps**:
```bash
# 1. Clone repository
git clone https://github.com/marcucus/ZenRunner.git
cd ZenRunner

# 2. Run installation script
./install.sh  # Linux/macOS
# or
.\install.ps1  # Windows

# 3. Verify installation
ls ~/.local/bin/ZenRunner  # Linux/macOS
ls $env:LOCALAPPDATA\Programs\ZenRunner\ZenRunner.exe  # Windows

# 4. Test launcher
./zenrunner.sh  # Linux/macOS
.\zenrunner.bat  # Windows
```

**Expected Result**:
- ✅ Installation completes without errors
- ✅ Executable is created in user directory
- ✅ Launcher script works
- ✅ Application starts successfully
- ✅ Configuration directory is created

**Pass Criteria**:
- Application launches and shows main window
- No error messages during installation
- Memory usage < 30 MB on idle

---

### TC-002: System-Wide Installation

**Objective**: Verify system-wide installation (requires sudo/admin)

**Prerequisites**: 
- All dependencies installed
- Administrator/sudo access

**Steps**:
```bash
# Linux/macOS
./install.sh -m system

# Verify
ls /usr/local/bin/ZenRunner
which ZenRunner
ZenRunner
```

**Expected Result**:
- ✅ Installation to /usr/local/bin succeeds
- ✅ ZenRunner is in PATH
- ✅ Can launch from any directory
- ✅ All users can access (Linux/macOS)

---

### TC-003: Installation with Missing Dependencies

**Objective**: Verify dependency checking works correctly

**Prerequisites**: 
- Remove one dependency (e.g., Qt6)

**Steps**:
```bash
./install.sh  # Should detect missing Qt6
```

**Expected Result**:
- ✅ Script detects missing dependency
- ✅ Clear error message shown
- ✅ Instructions provided for installation
- ✅ Installation stops gracefully

---

### TC-004: Clean Build

**Objective**: Verify clean build removes old artifacts

**Prerequisites**: 
- Previous build exists

**Steps**:
```bash
./install.sh -c
```

**Expected Result**:
- ✅ Old build directory removed
- ✅ Fresh build succeeds
- ✅ No leftover artifacts
- ✅ Executable is updated

---

### TC-005: Debug Build

**Objective**: Verify debug build mode works

**Steps**:
```bash
./install.sh -d  # Linux/macOS
.\install.ps1 -BuildType Debug  # Windows
```

**Expected Result**:
- ✅ Debug build completes
- ✅ Debug symbols included
- ✅ Application runs
- ✅ Can attach debugger

---

### TC-006: Build-Only Mode

**Objective**: Verify build without installation

**Steps**:
```bash
./install.sh --no-install  # Linux/macOS
.\install.ps1 -NoInstall  # Windows
```

**Expected Result**:
- ✅ Application builds successfully
- ✅ No installation performed
- ✅ Executable in build/ directory
- ✅ Launcher script still works (finds local build)

---

### TC-007: Launcher Script Detection

**Objective**: Verify launcher finds executable in all locations

**Test Scenarios**:
1. Executable in build/bin/
2. Executable in ~/.local/bin/
3. Executable in /usr/local/bin/
4. Executable in PATH

**Steps**:
```bash
# Test each scenario
./zenrunner.sh
```

**Expected Result**:
- ✅ Launcher finds executable in priority order
- ✅ Correct executable is launched
- ✅ Clear error if not found

---

### TC-008: Uninstallation (Keep Config)

**Objective**: Verify uninstall preserves configuration

**Steps**:
```bash
# Run application and create some config
./zenrunner.sh
# (Create workspace, add project)
# Exit application

# Uninstall
./uninstall.sh
# Choose to keep config files

# Verify
ls ~/.config/ZenRunner  # Should exist
ls ~/.local/bin/ZenRunner  # Should NOT exist
```

**Expected Result**:
- ✅ Executable removed
- ✅ Configuration preserved
- ✅ Can reinstall and keep settings

---

### TC-009: Complete Uninstallation

**Objective**: Verify complete removal

**Steps**:
```bash
./uninstall.sh
# Choose to remove config files
```

**Expected Result**:
- ✅ All executables removed
- ✅ All configuration removed
- ✅ PATH cleaned up
- ✅ No ZenRunner files remain

---

### TC-010: Reinstallation

**Objective**: Verify reinstallation over existing install

**Steps**:
```bash
# First installation
./install.sh

# Modify something
touch build/test.txt

# Reinstall
./install.sh

# Verify
ls build/test.txt  # Should still exist (not cleaned)
```

**Expected Result**:
- ✅ Reinstallation succeeds
- ✅ Executable updated
- ✅ No conflicts or errors
- ✅ Configuration preserved

---

### TC-011: Qt Path Detection (macOS)

**Objective**: Verify Qt detection on macOS

**Steps**:
```bash
# With Homebrew Qt
brew install qt@6
./install.sh
```

**Expected Result**:
- ✅ Script finds Qt in /opt/homebrew/opt/qt@6
- ✅ CMAKE_PREFIX_PATH set correctly
- ✅ Build succeeds
- ✅ Application runs

---

### TC-012: Qt Path Specification (Windows)

**Objective**: Verify custom Qt path on Windows

**Steps**:
```powershell
.\install.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"
```

**Expected Result**:
- ✅ Uses specified Qt path
- ✅ Build succeeds
- ✅ Qt DLLs copied to install location
- ✅ Application runs without errors

---

### TC-013: Parallel Build

**Objective**: Verify multi-core compilation works

**Steps**:
```bash
./install.sh
# Check output for parallel build indication
```

**Expected Result**:
- ✅ Uses multiple cores (nproc on Linux, hw.ncpu on macOS)
- ✅ Build is faster than single-threaded
- ✅ No build errors from parallelization

---

### TC-014: PATH Update

**Objective**: Verify PATH is updated correctly

**Steps**:
```bash
# Before installation
echo $PATH  # Note current PATH

# Install
./install.sh

# After installation
echo $PATH  # Should include ~/.local/bin
```

**Expected Result**:
- ✅ PATH includes installation directory
- ✅ Can run ZenRunner from any directory
- ✅ Shell config file updated (.bashrc/.zshrc)

---

### TC-015: Launch with Arguments

**Objective**: Verify launcher passes arguments to executable

**Steps**:
```bash
./zenrunner.sh --help
./zenrunner.sh /path/to/project
```

**Expected Result**:
- ✅ Arguments passed correctly
- ✅ Application responds to arguments
- ✅ No argument loss or corruption

---

## Performance Tests

### PT-001: Build Time

**Objective**: Measure build time

**Steps**:
```bash
time ./install.sh --no-install
```

**Expected Results**:
- First build: < 10 minutes
- Incremental build: < 3 minutes
- Multi-core speedup observed

---

### PT-002: Installation Size

**Objective**: Verify disk usage

**Steps**:
```bash
du -sh ~/.local/bin/ZenRunner
du -sh build/
```

**Expected Results**:
- Executable: < 10 MB
- Build directory: < 100 MB
- Total installation: < 50 MB

---

### PT-003: Runtime Performance

**Objective**: Verify runtime meets performance targets

**Steps**:
```bash
./zenrunner.sh
# Monitor with system tools
```

**Expected Results**:
- Startup time: < 2 seconds
- Idle RAM: < 15 MB
- Active RAM: < 30 MB
- CPU idle: < 2%
- UI: 60 FPS

---

## Automation Scripts

### Quick Test Script (Linux/macOS)

```bash
#!/bin/bash
# test_installation.sh

echo "=== ZenRunner Installation Test ==="

# Clean environment
rm -rf build
./uninstall.sh -y 2>/dev/null || true

# Test installation
echo "Test 1: User installation"
./install.sh
if [ $? -eq 0 ]; then
    echo "✅ Installation successful"
else
    echo "❌ Installation failed"
    exit 1
fi

# Test launcher
echo "Test 2: Launcher script"
./zenrunner.sh --help >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✅ Launcher works"
else
    echo "❌ Launcher failed"
    exit 1
fi

# Test uninstall
echo "Test 3: Uninstallation"
echo "y" | ./uninstall.sh
if [ ! -f ~/.local/bin/ZenRunner ]; then
    echo "✅ Uninstallation successful"
else
    echo "❌ Uninstallation failed"
    exit 1
fi

echo "=== All tests passed ==="
```

### Quick Test Script (Windows PowerShell)

```powershell
# test_installation.ps1

Write-Host "=== ZenRunner Installation Test ===" -ForegroundColor Cyan

# Clean environment
if (Test-Path build) { Remove-Item build -Recurse -Force }

# Test installation
Write-Host "Test 1: User installation" -ForegroundColor Yellow
.\install.ps1 -SkipDepsCheck
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Installation successful" -ForegroundColor Green
} else {
    Write-Host "❌ Installation failed" -ForegroundColor Red
    exit 1
}

# Test launcher
Write-Host "Test 2: Launcher script" -ForegroundColor Yellow
.\zenrunner.bat
if (Test-Path "$env:LOCALAPPDATA\Programs\ZenRunner\ZenRunner.exe") {
    Write-Host "✅ Installation verified" -ForegroundColor Green
} else {
    Write-Host "❌ Executable not found" -ForegroundColor Red
    exit 1
}

Write-Host "=== All tests passed ===" -ForegroundColor Green
```

---

## Reporting Test Results

When reporting test results, include:

1. **Environment**:
   - OS and version
   - Architecture (x86_64/ARM64)
   - Qt version
   - CMake version
   - Compiler version

2. **Test Results**:
   - Test case ID
   - Pass/Fail status
   - Actual behavior if failed
   - Error messages
   - Screenshots if applicable

3. **Performance Metrics**:
   - Build time
   - Installation size
   - Runtime memory usage
   - Application startup time

4. **Issues Found**:
   - Description
   - Steps to reproduce
   - Expected vs actual behavior
   - Workaround if any

---

## Success Criteria

The installation system is considered successful if:

- ✅ All test cases pass on at least 2 platforms per OS family
- ✅ Installation completes in < 10 minutes on standard hardware
- ✅ Clear error messages for all failure scenarios
- ✅ Documentation is accurate and complete
- ✅ Launcher scripts work reliably
- ✅ Uninstallation is clean and complete
- ✅ No manual intervention required for standard installations

---

## Known Issues

Document any known issues here:

1. **Issue**: Description
   - **Platform**: Which OS/version
   - **Workaround**: How to fix
   - **Status**: Open/In Progress/Fixed

---

## Test Log Template

```
Test Date: YYYY-MM-DD
Tester: Name
Platform: OS Version (Architecture)

Test Case: TC-XXX
Status: ✅ Pass / ❌ Fail / ⚠️ Skip
Duration: X minutes
Notes: Additional observations

Environment:
- OS: 
- Qt Version: 
- CMake Version: 
- Compiler: 

Results:
[Detailed test results]
```
