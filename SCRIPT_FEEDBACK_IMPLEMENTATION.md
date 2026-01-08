# Script Launch Feedback Implementation

## Problem Statement
When a user clicks a script button in the Dashboard, the script launches in the background but provides no visual feedback. This creates a poor user experience where:
- Users don't know if their button click was registered
- Users don't know if the process started successfully
- Users don't know if there was an error starting the process
- Users have no visibility into the running state of scripts

## Solution Overview
Added comprehensive feedback mechanisms at multiple levels:

1. **In-App Toast Notifications**: Immediate visual feedback within the application window
2. **Native System Notifications**: OS-level notifications for important events
3. **Visual Button State**: Buttons show running state with indicator and color change
4. **Event-Driven Updates**: Real-time feedback as process states change

## Changes Made

### 1. New Toast Component (`src/ui/components/Toast.qml`)
A reusable toast notification component with:
- Smooth slide-up animation from bottom of screen
- Auto-hide after 3 seconds (configurable)
- Colored backgrounds for different message types (success, warning, error)
- Glassmorphism styling consistent with app design
- Z-index positioning to appear above all other content

### 2. Enhanced Dashboard (`src/ui/Dashboard.qml`)

#### Added Toast Instance
```qml
Toast {
    id: toast
    z: 1000
}
```

#### Added Process Event Listeners
Connected to ProcessManager signals to show toast notifications for:
- Process completion with exit code
- Process crashes
- Process errors

#### Enhanced Script Buttons
Each script button now:
- Tracks its associated process ID
- Monitors process state changes via Connections
- Shows running indicator (● symbol) when process is active
- Changes accent color to green when running
- Shows toast + native notification on start
- Shows error toast + notification on failure

### 3. Updated Resources
- Added Toast.qml to resources.qrc
- Updated components/qmldir to register Toast component

## User Experience Flow

### When Starting a Script
1. User clicks script button
2. **Immediate Feedback**: Toast slides up showing "Started 'script-name' in project-name" with green background
3. **System Notification**: Native OS notification appears
4. **Button Updates**: Button shows "● script-name" with green accent color
5. Console logs success message

### If Start Fails
1. User clicks script button
2. **Error Feedback**: Toast slides up showing "Failed to start 'script-name'" with red background
3. **System Notification**: Native error notification appears
4. Console logs error message

### When Process Completes
1. Process finishes
2. **Completion Feedback**: Toast shows exit code (green for 0, yellow for non-zero)
3. **Button Updates**: Running indicator disappears, returns to normal state

### When Process Crashes
1. Process crashes
2. **Critical Feedback**: Red toast notification appears
3. **Native Notification**: OS notification with crash details
4. **System Tray**: Tray icon updates to error state (red)
5. Button returns to normal state

## Technical Details

### Signal Connections
The implementation uses Qt's signal/slot mechanism to connect:
- `processManager.processStateChanged` → Button state updates
- `processManager.processFinished` → Completion toast
- `processManager.processCrashed` → Crash toast
- `processManager.processError` → Error toast

### Process State Values
Based on the ProcessState enum:
- 0: NotStarted
- 1: Starting
- 2: Running (used to show running indicator)
- 3: Paused
- 4: Stopping
- 5: Stopped
- 6: Finished
- 7: Crashed

### Toast Color Coding
- Green (`Qt.rgba(0.2, 0.6, 0.3, 0.95)`): Success/start
- Red (`Qt.rgba(0.8, 0.2, 0.2, 0.95)`): Error/crash
- Yellow (`Qt.rgba(0.8, 0.6, 0.2, 0.95)`): Warning/non-zero exit

## Benefits

1. **Immediate Feedback**: Users know instantly when they click a button
2. **Error Visibility**: Failures are clearly communicated
3. **Running State Awareness**: Visual indicator shows which scripts are running
4. **Completion Notification**: Users are informed when long-running scripts finish
5. **Multi-Level Notifications**: Both in-app and system-level feedback
6. **Consistent UX**: Glassmorphism styling matches application design

## Testing Recommendations

To verify this implementation:

1. **Start Script Successfully**
   - Click any script button
   - Verify green toast appears at bottom
   - Verify native notification appears
   - Verify button shows ● indicator and green color
   - Check console for success log

2. **Script Completion**
   - Wait for script to finish
   - Verify completion toast appears with exit code
   - Verify button returns to normal state

3. **Script Failure**
   - Try starting a non-existent script
   - Verify red error toast appears
   - Verify error notification appears

4. **Script Crash**
   - Start a script that will crash
   - Verify crash toast appears
   - Verify system tray turns red

5. **Multiple Scripts**
   - Start multiple scripts
   - Verify each button shows running state independently
   - Verify toasts don't overlap (auto-hide after 3s)

## Compatibility

This implementation uses:
- Qt Quick standard components (Rectangle, Text, Timer, Animation)
- Qt.rgba() for colors (Qt 5.0+)
- Connections for signal handling (Qt standard)
- No platform-specific code in Toast component

Should work on all platforms supported by ZenRunner:
- ✓ Windows 10/11
- ✓ macOS (Intel and Apple Silicon)
- ✓ Linux (various distributions)

## Future Enhancements

Potential improvements:
1. Queue multiple toasts instead of replacing
2. Make toast dismissible by clicking
3. Add sound effects on notifications (optional)
4. Add progress bar for long-running scripts
5. Show script output preview in toast on hover
6. Add action buttons to toast (e.g., "View Logs", "Stop")
