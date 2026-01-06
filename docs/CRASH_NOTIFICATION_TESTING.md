# Crash Notification System - Testing Guide

## Overview
This document describes how to test the crash notification system implemented for ZenRunner's asynchronous process management.

## Implementation Summary

### What Was Added
1. **ProcessManager::processCrashed Signal**
   - Emitted specifically when a process exits with `QProcess::CrashExit`
   - Provides process ID and exit code for user notification
   - Location: `include/core/ProcessManager.h` and `src/core/ProcessManager.cpp`

2. **Notification Integration in main.cpp**
   - Integrated `NativePlatformManager` for system tray and notifications
   - Connected crash events to show user notifications
   - Connected error events to show warnings
   - Updates tray icon state based on process states

3. **Graceful Shutdown Enhancement**
   - Stops all running processes before application exit
   - Uses `ProcessManager::stopAll(5000)` with 5-second timeout
   - Relies on existing SIGTERM → SIGKILL fallback mechanism
   - Includes brief wait period for signal propagation

## Testing Scenarios

### Test 1: Process Crash Notification
**Objective**: Verify that a crashing process triggers a notification

**Steps**:
1. Start ZenRunner application
2. Create a test process that will crash (e.g., command that doesn't exist or will segfault)
3. Start the process through ProcessManager
4. Observe the crash

**Expected Results**:
- Console logs: `Process crashed: <id> with exit code: <code>`
- System notification appears: "Process Crashed - Process '<id>' crashed with exit code <code>"
- System tray icon changes to Error state (red)

**Code to trigger crash**:
```cpp
ProcessConfig config;
config.command = "/bin/sh";
config.arguments = QStringList{"-c", "kill -SEGV $$"};  // Self-terminate with SIGSEGV
auto result = processManager.createProcess("test_crash", config);
if (result.isOk()) {
    processManager.startProcess("test_crash");
}
```

### Test 2: Process Error Notification
**Objective**: Verify that process errors trigger notifications

**Steps**:
1. Start ZenRunner application
2. Create a process with invalid configuration (non-existent command)
3. Attempt to start the process

**Expected Results**:
- Console logs: `Process error: <id> - Failed to start: ...`
- System notification: "Process Error - Process '<id>': Failed to start..."
- System tray icon changes to Warning state (orange)

**Code to trigger error**:
```cpp
ProcessConfig config;
config.command = "/nonexistent/command";
auto result = processManager.createProcess("test_error", config);
if (result.isOk()) {
    processManager.startProcess("test_error");  // Will fail to start
}
```

### Test 3: Graceful Shutdown
**Objective**: Verify all processes are stopped gracefully on app exit

**Steps**:
1. Start ZenRunner application
2. Start multiple long-running processes
3. Quit the application (via menu or quit signal)
4. Monitor console logs

**Expected Results**:
- Console logs: `Stopping <N> running processes...`
- Each process receives SIGTERM first
- If a process doesn't terminate within 5 seconds, it receives SIGKILL
- Console logs: `Goodbye!`
- Application exits cleanly without zombie processes

**Code to test shutdown**:
```cpp
// Start several long-running processes
for (int i = 0; i < 3; i++) {
    ProcessConfig config;
    config.command = "/bin/sh";
    config.arguments = QStringList{"-c", "sleep 1000"};
    auto result = processManager.createProcess(QString("sleep_%1").arg(i), config);
    if (result.isOk()) {
        processManager.startProcess(QString("sleep_%1").arg(i));
    }
}

// Wait a moment, then quit
QTimer::singleShot(2000, &app, &QGuiApplication::quit);
```

### Test 4: State Transitions
**Objective**: Verify tray icon state updates correctly

**Steps**:
1. Start ZenRunner (tray should be Idle/gray)
2. Start a process (tray should become Active/green)
3. Stop the process (tray should return to Idle/gray)
4. Start a process that crashes (tray should become Error/red)

**Expected State Transitions**:
- No processes running → Idle (gray)
- Processes running → Active (green)
- Process crashes → Error (red)
- Process errors → Warning (orange)
- All processes stopped → Idle (gray)

## Manual Testing Checklist

- [ ] Process crash triggers notification
- [ ] Process error triggers notification
- [ ] Tray icon updates on state changes
- [ ] Multiple crashes show multiple notifications
- [ ] Graceful shutdown stops all processes
- [ ] SIGTERM → SIGKILL fallback works (test with unresponsive process)
- [ ] Application exits cleanly without zombie processes
- [ ] Notification click interactions work (if implemented)

## Automated Testing

To create automated tests, consider:

1. **Unit Tests for Signal Emission**
   ```cpp
   // Test that processCrashed signal is emitted
   QSignalSpy crashSpy(&processManager, &ProcessManager::processCrashed);
   // ... trigger crash ...
   QCOMPARE(crashSpy.count(), 1);
   ```

2. **Integration Tests for Notification Flow**
   ```cpp
   // Test that crash notification reaches platform manager
   QSignalSpy notificationSpy(&platformManager, /* notification signal */);
   // ... trigger crash ...
   QVERIFY(notificationSpy.count() > 0);
   ```

3. **Shutdown Tests**
   ```cpp
   // Test that processes are stopped on shutdown
   // Start processes, trigger quit, verify all processes stopped
   ```

## Known Limitations

1. **Notification System**: The notification system relies on `QSystemTrayIcon::showMessage()` which may have platform-specific limitations:
   - Windows: Uses native toast notifications
   - macOS: Uses Notification Center (may require permissions)
   - Linux: Depends on desktop environment (GNOME, KDE, etc.)

2. **Timeout Behavior**: The 5-second timeout for graceful shutdown is hardcoded. Consider making this configurable for different use cases.

3. **Zombie Prevention**: While the implementation handles SIGTERM/SIGKILL properly, extremely stubborn processes (stuck in kernel) may still leave zombies until parent process exits.

## Debugging

To enable verbose logging:
```cpp
// Add to main.cpp after QGuiApplication creation
qSetMessagePattern("[%{time hh:mm:ss.zzz}] [%{type}] %{message}");
```

To test SIGTERM/SIGKILL fallback:
```cpp
// Create a process that ignores SIGTERM
config.command = "/bin/sh";
config.arguments = QStringList{"-c", "trap '' TERM; sleep 1000"};
```

## Conclusion

The crash notification system is now fully integrated and should provide immediate user feedback when processes crash or encounter errors. The graceful shutdown mechanism ensures all child processes are properly terminated when the application exits.
