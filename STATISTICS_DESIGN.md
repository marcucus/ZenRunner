# Statistical Dashboard Design Decisions

## Overview
This document explains the design decisions made for the statistical dashboard feature that monitors CPU and RAM usage per project/process.

## Key Design Decisions

### 1. Lightweight Monitoring
**Decision**: Use 2-second sampling interval with limited history (30 samples)

**Rationale**: 
- Minimizes CPU overhead from system calls
- Keeps memory footprint minimal (~820 bytes per process)
- Provides sufficient granularity for user feedback
- Maintains ZenRunner's < 30MB RAM target

**Alternative Considered**: 1-second sampling
- Rejected due to increased CPU overhead
- Would double the number of system calls
- Marginal benefit for developer use case

### 2. Circular Buffer for History
**Decision**: Store last 30 data points in a deque with fixed max size

**Rationale**:
- Bounded memory usage
- Efficient insertion/deletion at both ends
- Automatically discards old data
- Aligns with ZenRunner's memory optimization philosophy

**Alternative Considered**: Unbounded history
- Rejected due to memory growth over time
- Would violate memory constraints for long-running processes

### 3. Platform-Specific Implementations
**Decision**: Implement resource monitoring using native OS APIs

**Platforms**:
- **Windows**: GetProcessTimes(), GetProcessMemoryInfo()
- **Linux**: /proc/[pid]/stat, /proc/[pid]/status
- **macOS**: proc_pidinfo() with PROC_PIDTASKINFO

**Rationale**:
- Most accurate and efficient method
- Minimal overhead
- No external dependencies
- Consistent with ZenRunner's native integration philosophy

**Alternative Considered**: Cross-platform library (e.g., psutil)
- Rejected due to external dependency
- Would increase binary size
- Not available for C++ without Python bindings

### 4. Spike Detection
**Decision**: Static thresholds (80% CPU, 100MB RAM)

**Rationale**:
- Simple and predictable
- Easy to understand for users
- Suitable for typical development workloads
- No complex statistical analysis needed

**Alternative Considered**: Adaptive thresholds based on baseline
- Rejected as too complex for initial implementation
- Can be added as enhancement later

### 5. Architecture: Monitor + ViewModel + QML
**Decision**: Three-layer architecture with clear separation

```
Core Layer: ProcessResourceMonitor (C++)
    ↓
UI Layer: StatisticsViewModel (C++ with Qt properties)
    ↓
View Layer: Statistics.qml (QML/JavaScript)
```

**Rationale**:
- Separation of concerns
- Testable components
- Follows ZenRunner's existing architecture
- Easy to extend or replace components

**Alternative Considered**: Direct QML to Core binding
- Rejected due to mixing concerns
- Makes testing harder
- Less maintainable

### 6. QML List Model for Display
**Decision**: Use QAbstractListModel for process statistics

**Rationale**:
- Native Qt integration
- Efficient updates with dataChanged signals
- Supports dynamic list updates
- Familiar pattern for Qt developers

**Alternative Considered**: Manual QVariantList updates
- Rejected due to inefficiency
- Would require full list rebuilding on each update
- Poor performance with many processes

### 7. No Historical Graphs
**Decision**: Show only current values and simple trend bars

**Rationale**:
- Maintains lightweight UI
- Avoids Qt Charts dependency (adds ~5MB to binary)
- Sufficient for identifying issues
- Keeps within performance budget

**Alternative Considered**: Qt Charts integration
- Rejected due to binary size increase
- Would impact memory usage
- Can be added as optional feature later

### 8. Async Signal-Based Updates
**Decision**: Use Qt signals/slots for stats updates

**Rationale**:
- Non-blocking
- Follows Qt best practices
- Maintains UI responsiveness
- Compatible with QML data binding

**Alternative Considered**: Polling from QML
- Rejected due to coupling
- Would require timers in QML
- Less efficient

### 9. Spike Notifications via Signals
**Decision**: Emit signal when spike detected, QML handles display

**Rationale**:
- Decoupled notification logic
- Allows future extensions (system notifications, logs)
- QML can choose how to display
- Follows reactive programming model

**Alternative Considered**: Direct QML popup
- Rejected due to tight coupling
- Hard to test
- Limited flexibility

### 10. Process ID Format: "projectName:scriptName"
**Decision**: Use colon-separated format for processId

**Rationale**:
- Easy to parse for display
- Human-readable
- Allows grouping by project
- Compatible with existing ProcessManager

**Alternative Considered**: Separate project and script fields
- Rejected to maintain compatibility
- Would require changes to existing code
- Current format is sufficient

## Memory Impact Analysis

Per monitored process:
- MonitoredProcess struct: ~160 bytes
- History (30 samples * 32 bytes): ~960 bytes
- **Total: ~1120 bytes per process**

For 10 concurrent processes: **~11 KB** (negligible compared to 30MB target)

## CPU Impact Analysis

Per sampling cycle (every 2 seconds):
- System call overhead: ~100 microseconds per process
- CPU calculation: ~50 microseconds
- Qt signal emission: ~10 microseconds
- **Total: ~160 microseconds per process**

For 10 processes: **~1.6 milliseconds every 2 seconds** (0.08% CPU on single core)

## Trade-offs Accepted

1. **Static Thresholds**: Not adaptive, may not suit all workloads
   - Future enhancement: User-configurable thresholds
   
2. **Limited History**: Only 1 minute of data
   - Future enhancement: Persistent storage option
   
3. **No Real-time Graphs**: Simple bars instead of charts
   - Future enhancement: Optional Qt Charts integration
   
4. **Polling-based**: 2-second intervals, not event-driven
   - OS doesn't provide event-based notifications for CPU/RAM changes
   - Polling is the industry-standard approach

## Future Enhancements

Potential improvements that maintain design principles:

1. **User-configurable thresholds**: Settings UI for spike detection
2. **Export to CSV**: Save statistics for external analysis
3. **Process comparison**: Side-by-side comparison view
4. **Persistent history**: Optional long-term storage (with limits)
5. **System tray integration**: Show alerts in system tray
6. **Per-core CPU breakdown**: On multi-core systems
7. **Memory leak detection**: Trend analysis for growing memory
8. **GPU usage tracking**: For GPU-accelerated processes

## Conclusion

The statistical dashboard design prioritizes:
- **Minimal overhead**: ~1KB per process, negligible CPU usage
- **Native implementation**: Platform-specific APIs for accuracy
- **Clean architecture**: Testable, maintainable, extensible
- **User experience**: Clear visual feedback, spike detection

These decisions align with ZenRunner's core philosophy of native performance with modern UX.
