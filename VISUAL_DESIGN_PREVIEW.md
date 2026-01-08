# Visual Design Preview: Script Feedback Implementation

## Before vs After Comparison

### BEFORE (No Feedback)
```
┌─────────────────────────────────────────────────────┐
│  MyProject                              3 scripts   │
│  /home/user/projects/myproject                      │
│                                                      │
│  [dev]  [build]  [test]                            │
│   ↑       ↑        ↑                                │
│   Click but no feedback...                          │
└─────────────────────────────────────────────────────┘
```

### AFTER (Full Feedback) - When Starting Script
```
┌─────────────────────────────────────────────────────┐
│  MyProject                              3 scripts   │
│  /home/user/projects/myproject                      │
│                                                      │
│  [dev ●]  [build]  [test]                          │
│   ↑ Green accent color                              │
│   Running indicator                                 │
│                                                      │
│                                                      │
│             ┌─────────────────────────────┐         │
│             │ Started 'dev' in MyProject  │ ← Toast │
│             └─────────────────────────────┘         │
└─────────────────────────────────────────────────────┘

OS Notification: "Script Started - Running 'dev' in MyProject"
```

### AFTER - Multiple Scripts Running
```
┌─────────────────────────────────────────────────────┐
│  MyProject                              3 scripts   │
│  /home/user/projects/myproject                      │
│                                                      │
│  [dev ●]  [build ●]  [test]                        │
│   Green    Green      Normal                        │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### AFTER - Error State
```
┌─────────────────────────────────────────────────────┐
│  MyProject                              3 scripts   │
│  /home/user/projects/myproject                      │
│                                                      │
│  [dev]  [build]  [test]                            │
│                                                      │
│                                                      │
│             ┌─────────────────────────────┐         │
│             │ Failed to start 'test'      │ ← Toast │
│             │        (Red background)     │   (Red) │
│             └─────────────────────────────┘         │
└─────────────────────────────────────────────────────┘

OS Notification: "Failed to Start - Could not start 'test' in MyProject"
```

## Toast Animation Sequence

```
Frame 1 (0ms):      Frame 2 (100ms):    Frame 3 (200ms):    Frame 4 (3000ms):   Frame 5 (3200ms):
┌─────────┐         ┌─────────┐         ┌─────────┐         ┌─────────┐         
│         │         │         │         │         │         │         │         
│         │         │         │         │  Toast  │         │  Toast  │         
│         │         │  Toast  │         │ Visible │         │ Visible │         
│         │         │ Sliding │         │         │         │         │         
└─────────┘         └─────────┘         └─────────┘         └─────────┘         (Hidden)
Opacity: 0%         Opacity: 50%        Opacity: 100%       Opacity: 100%       Opacity: 0%
Bottom: 60px        Bottom: 70px        Bottom: 80px        Bottom: 80px        Bottom: 100px
```

## Color Coding

### Toast Backgrounds
- **Success (Green)**: `rgba(51, 153, 77, 0.95)` - Script started successfully
- **Error (Red)**: `rgba(204, 51, 51, 0.95)` - Script failed to start or crashed
- **Warning (Yellow)**: `rgba(204, 153, 51, 0.95)` - Non-zero exit code

### Button States
- **Idle**: Normal blue accent `#4a90e2`
- **Running**: Green accent `#4ade80` with ● indicator
- **Script Type Colors**:
  - dev/start: Green `#4ade80`
  - test: Yellow `#fbbf24`
  - build: Blue `#60a5fa`
  - lint: Purple `#a78bfa`

## Toast Component Specifications

```
┌───────────────────────────────────────┐
│  ● Started 'dev' in MyProject         │ ← Text (white)
└───────────────────────────────────────┘
  ↑                                     ↑
  Border (white, 10% opacity)           Background (colored, 95% opacity)
  Radius: 12px                          Min Width: parent.width - 40
  Height: 50px                          Max Width: text width + 40
```

## Button Component Enhancements

### Before:
```qml
GlassButton {
    text: "dev"
    accentColor: "#4ade80"
}
```

### After:
```qml
GlassButton {
    text: isRunning ? "dev ●" : "dev"
    accentColor: isRunning ? "#4ade80" : originalColor
    
    // State tracked via Connections to processManager
}
```

## Signal Flow Diagram

```
User Action          ProcessManager              UI Components
    │                       │                         │
    │ Click [dev]           │                         │
    ├──────────────────────>│                         │
    │                       │ runScript()             │
    │                       ├─────────────────────────>│
    │                       │                         │ Show Toast (green)
    │                       │                         │ Show Native Notification
    │                       │                         │
    │                       │ Process starts          │
    │                       │ emit processStateChanged│
    │                       ├─────────────────────────>│
    │                       │                         │ Update button: "dev ●"
    │                       │                         │ Change color to green
    │                       │                         │
    │                       │ Process completes       │
    │                       │ emit processFinished    │
    │                       ├─────────────────────────>│
    │                       │                         │ Show Toast (completion)
    │                       │                         │ Reset button state
```

## User Journey Map

### Happy Path (Success)
1. User sees project with scripts: [dev] [build] [test]
2. User clicks [dev] button
3. **Immediate**: Toast slides up from bottom (green)
4. **Immediate**: Native OS notification appears
5. **<100ms**: Button updates to [dev ●] with green color
6. Script runs in background...
7. **On completion**: Toast shows exit code
8. **On completion**: Button returns to [dev] normal state

### Error Path (Failure)
1. User clicks [test] button
2. **Immediate**: Process fails to start
3. **Immediate**: Red toast appears: "Failed to start 'test'"
4. **Immediate**: Native error notification
5. Button remains in normal state (no running indicator)

### Crash Path (Critical)
1. User has [dev ●] running
2. Process crashes unexpectedly
3. **Immediate**: Red toast: "Process crashed!"
4. **Immediate**: Native critical notification
5. **Immediate**: System tray icon turns red
6. Button returns to [dev] normal state

## Accessibility Considerations

- **Color coding** is supplemented by text ("Started", "Failed", "Crashed")
- **Visual indicators** (● symbol) provide additional state information
- **Multiple feedback levels** ensure users don't miss important events
- **Animations** are smooth but not distracting (200ms duration)
- **Auto-hide** prevents screen clutter (3 second timeout)

## Performance Impact

- **Minimal**: Toast uses GPU-accelerated Qt Quick rendering
- **Efficient**: Only active when needed (opacity: 0 when hidden)
- **Lightweight**: Simple Rectangle + Text components
- **Non-blocking**: All animations run on render thread
- **Memory**: <1KB per Toast instance

## Browser Compatibility (N/A)
This is a Qt Quick desktop application, not a web application.
All rendering is handled by Qt's native engine.
