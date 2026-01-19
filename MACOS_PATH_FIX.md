# macOS Process Launch Fix - PATH Resolution Enhancement

## Issue Description

When ZenRunner is launched from the macOS GUI (Finder, Launchpad, or dock), scripts fail to start with the error:
```
Process failed to start: The process failed to start. Either the invoked program is missing, or you may have insufficient permissions.
```

This issue occurs when trying to run npm, yarn, or pnpm scripts from within the application.

## Root Cause

macOS GUI applications launched outside of a terminal do not inherit the user's shell PATH environment variable. When an application is launched from Finder:

1. `QProcessEnvironment::systemEnvironment()` only provides a minimal PATH (typically `/usr/bin:/bin`)
2. Common installation locations like `/usr/local/bin` (Homebrew Intel), `/opt/homebrew/bin` (Homebrew Apple Silicon), and user-specific paths are not included
3. Package managers (npm, yarn, pnpm) installed via Homebrew or other methods cannot be found
4. QProcess fails to start with `FailedToStart` error

This is a well-known macOS limitation documented in Apple's developer documentation.

## Solution

The fix enhances the `ProcessManager::runScript()` method to build a comprehensive PATH that includes all common macOS binary locations before attempting to execute commands.

### Implementation Details

1. **Enhanced PATH Construction** (lines 523-609 in `src/core/ProcessManager.cpp`):
   - Starts with local `node_modules/.bin` (highest priority)
   - Adds common macOS binary locations (if they exist):
     - `/usr/local/bin` - Homebrew on Intel Macs
     - `/opt/homebrew/bin` - Homebrew on Apple Silicon Macs
     - `/opt/local/bin` - MacPorts installations
     - `/usr/bin` - System binaries
     - `/bin` - Basic system binaries
   - Checks for NVM (Node Version Manager):
     - First looks for `~/.nvm/current/bin` symlink (active version)
     - Falls back to scanning `~/.nvm/versions/node/` for latest version
   - Checks for Volta at `~/.volta/bin`
   - Adds user local binaries at `~/.local/bin`
   - Appends existing system PATH components

2. **Executable Path Resolution** (lines 597-609):
   - Uses `QStandardPaths::findExecutable()` to search all PATH components
   - Resolves the full path to npm/yarn/pnpm before execution
   - Provides clear debug logging showing where executables were found

3. **Cross-Platform Compatibility**:
   - macOS-specific enhancements are wrapped in `#ifdef Q_OS_MACOS`
   - Linux and Windows continue to work as before
   - All platforms benefit from the local `node_modules/.bin` handling

### Code Changes

**File:** `src/core/ProcessManager.cpp`

**Key additions:**
- Include `QStandardPaths` for executable resolution
- Enhanced PATH building with platform-specific logic
- Improved debug logging for troubleshooting
- Optimized path handling (avoid unnecessary string splits/joins)

## Testing

To verify this fix:

1. **Build and package the application:**
   ```bash
   cd packaging/macos
   ./build-dmg.sh
   ```

2. **Install on macOS:**
   - Mount the .dmg file
   - Drag ZenRunner.app to Applications folder
   - Launch from Applications (not from terminal)

3. **Test script execution:**
   - Open a JavaScript/TypeScript project with package.json
   - Add the project to ZenRunner
   - Click "Run" on any npm script
   - Verify the script starts successfully

4. **Check debug logs:**
   - View Console.app logs filtered by "ZenRunner"
   - Look for messages like:
     ```
     Enhanced PATH with 8 additional locations
     Found executable at: /opt/homebrew/bin/npm
     Executing: /opt/homebrew/bin/npm run dev in /path/to/project
     ```

## Additional Benefits

This fix also improves reliability for:
- Node.js projects using different version managers (NVM, Volta, n, etc.)
- Projects with local dependencies in `node_modules/.bin`
- Mixed environments where npm might be installed in non-standard locations
- Users who install tools via different package managers (Homebrew, MacPorts)

## Related Documentation

- [Apple Technical Note TN2449: Launching UNIX processes from GUIs](https://developer.apple.com/library/archive/technotes/tn2449/_index.html)
- [Qt Documentation: QProcessEnvironment](https://doc.qt.io/qt-6/qprocessenvironment.html)
- [Node.js PATH resolution](https://nodejs.org/api/modules.html#modules_all_together)

## Debug Logging

When troubleshooting PATH issues, look for these log messages:

- `"Running script: [name] in [path] with [package-manager]"` - Script execution initiated
- `"Enhanced PATH with N additional locations"` - Number of directories added (macOS only)
- `"Found NVM installation at: [path]"` - NVM detected and added
- `"Found Volta installation at: [path]"` - Volta detected and added
- `"Found executable at: [path]"` - Executable successfully resolved
- `"Warning: Could not find [command] in PATH"` - Executable not found, will attempt direct execution

## Future Improvements

Potential enhancements for future versions:

1. Cache executable paths to avoid repeated directory scans
2. Add support for additional version managers (asdf, fnm, etc.)
3. Allow users to configure custom PATH additions via settings
4. Provide UI feedback when executables cannot be found
5. Auto-detect shell configuration files (.zshrc, .bashrc) and parse PATH from them
