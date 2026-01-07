# ZenRunner Security Guidelines

## Overview

ZenRunner is designed with security and system safety as core principles. This document outlines the security measures implemented to ensure the application cannot cause damage to the user's system.

## Security Principles

### 1. Sandboxing and Process Isolation

ZenRunner operates within the security boundaries of the operating system:

- **Process Isolation**: All child processes are managed through Qt's `QProcess` API, which enforces OS-level process isolation
- **User Permissions**: The application runs with standard user permissions (not elevated/administrator rights)
- **Working Directory Control**: Each process executes within its own project directory, preventing accidental system-wide operations

### 2. File System Access

The application has limited and controlled file system access:

- **Read-Only Operations**: ZenRunner only reads configuration files (package.json) from user-selected directories
- **Write Operations**: Limited to user data directories for settings and workspace persistence:
  - Linux/macOS: `~/.config/ZenRunner/` and `~/.local/share/ZenRunner/`
  - Windows: `%APPDATA%\ZenRunner\` and `%LOCALAPPDATA%\ZenRunner\`
- **No System Modifications**: The application does not modify system files, registry entries (beyond user PATH), or system configurations
- **No Automatic Execution**: Scripts are never executed automatically; user action is always required

### 3. Process Management Safety

ZenRunner provides safe process management:

- **User-Initiated Actions**: All process starts, stops, and restarts require explicit user action
- **Graceful Termination**: Processes are terminated gracefully using standard signals (SIGTERM/SIGINT) before forced termination
- **Resource Monitoring**: Built-in resource monitoring helps prevent runaway processes
- **Process Scope**: Only manages user development processes, never system processes

### 4. Network Security

- **No External Connections**: ZenRunner itself does not make any network connections
- **Child Process Control**: Network access by child processes is controlled by the executed scripts themselves
- **No Data Collection**: No telemetry, analytics, or user data is collected or transmitted

### 5. Input Validation and Sanitization

- **Path Validation**: All file paths are validated and normalized to prevent directory traversal attacks
- **JSON Parsing**: Uses Qt's secure `QJsonDocument` parser with error handling
- **Command Injection Prevention**: User scripts are executed as-is without shell interpretation by ZenRunner
- **Log Sanitization**: ANSI escape sequences are parsed safely without executing arbitrary commands

### 6. Memory Safety

- **No Buffer Overflows**: C++20 with Qt's container classes provides memory safety
- **Circular Buffers**: Log buffers have fixed size limits (5000 lines) to prevent memory exhaustion
- **Smart Pointers**: Automatic memory management using C++ smart pointers and Qt's parent-child ownership
- **No Use-After-Free**: Qt's object lifecycle management prevents dangling pointers

### 7. Dependency Security

- **Minimal Dependencies**: Only relies on Qt 6 framework and system libraries
- **Known Vulnerabilities**: Regular updates to Qt framework address security issues
- **Static Analysis**: CodeQL scanning for potential vulnerabilities in custom code

## Security Best Practices for Users

### Running Development Scripts

While ZenRunner is secure, the scripts it executes have the same permissions as the user:

1. **Review Scripts**: Always review `package.json` scripts before running them
2. **Trust Sources**: Only add projects from trusted sources
3. **Limit Permissions**: Run ZenRunner with your regular user account, not as administrator
4. **Monitor Activity**: Use the built-in resource monitoring to watch for unexpected behavior

### Installation Security

1. **Verify Downloads**: Always download ZenRunner from official sources
2. **Code Signing**: Verify digital signatures on Windows (.exe) and macOS (.dmg) packages
3. **Build from Source**: For maximum security, build from source after reviewing the code

### Workspace Safety

1. **Isolated Workspaces**: Each workspace only affects projects within it
2. **No Cross-Contamination**: Projects in different workspaces cannot affect each other
3. **Clean Separation**: Logs and process states are maintained separately per project

## Reported Vulnerabilities

We take security seriously. If you discover a security vulnerability, please report it responsibly:

- **GitHub Security Advisories**: Use the [private security reporting feature](https://github.com/marcucus/ZenRunner/security/advisories) on GitHub (recommended)
- **GitHub Issues**: For non-security bugs, use regular [GitHub Issues](https://github.com/marcucus/ZenRunner/issues)

**Please do not** report security vulnerabilities through public GitHub issues.

## Security Auditing

### Automated Validation

Use the provided security validation script to check that a built executable follows security best practices:

```bash
./scripts/validate-security.sh
```

This script checks for:
- No setuid/setgid bits (Unix/Linux/macOS)
- Reasonable binary size
- No debug symbols in release builds
- Proper Qt dependencies
- Correct file permissions
- No suspicious hardcoded system paths

### Static Analysis

The project uses:
- CodeQL for security vulnerability scanning
- C++ compiler warnings at high levels (-Wall -Wextra -Wpedantic)
- Qt's built-in security features

### Code Review

All code changes undergo review for:
- Memory safety concerns
- Input validation
- Privilege escalation risks
- Resource exhaustion vulnerabilities

## Compliance

ZenRunner adheres to:
- **Principle of Least Privilege**: Runs with minimal necessary permissions
- **Defense in Depth**: Multiple layers of security controls
- **Secure by Default**: Secure configuration out of the box
- **Fail Secure**: Errors result in safe states (e.g., process not started)

## Security Updates

Security updates are released as soon as possible after discovery. Users are encouraged to:
- Keep ZenRunner updated to the latest version
- Monitor release notes for security-related updates
- Subscribe to security notifications (if available)

## Technical Security Details

### Process Execution Model

```cpp
// Processes are started with QProcess, which:
// 1. Uses OS-level process creation APIs (CreateProcess on Windows, fork/exec on Unix)
// 2. Inherits user's security context
// 3. Cannot escalate privileges
// 4. Are isolated from ZenRunner's address space
```

### File Access Patterns

```
READ access:
  - User-selected project directories (package.json)
  - User configuration directory (~/.config/ZenRunner/)
  
WRITE access:
  - User data directory (~/.local/share/ZenRunner/)
  - User configuration directory (~/.config/ZenRunner/)
  - User log directory (if separate)

NO access:
  - System directories
  - Other users' files
  - Protected operating system files
```

### Platform-Specific Security

#### Windows
- Runs in user mode (no UAC elevation required)
- Uses standard Windows security model
- Respects NTFS permissions
- Follows Windows App Security best practices

#### macOS
- Runs in user space (no root/sudo required)
- Respects macOS Gatekeeper and code signing
- Follows App Sandbox guidelines (for future App Store distribution)
- Uses standard Unix permissions

#### Linux
- Follows XDG Base Directory Specification
- Respects Unix file permissions
- No setuid/setgid binaries
- Standard user-space application

## Conclusion

ZenRunner is designed to be a safe, secure development tool that respects system boundaries and user privacy. By following these security principles and best practices, users can confidently use ZenRunner without risk of system damage or security compromise.

For questions about security, please refer to the project documentation or contact the maintainers.
