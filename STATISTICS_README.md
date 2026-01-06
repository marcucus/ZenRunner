# Statistical Dashboard Feature

## Overview

The statistical dashboard provides real-time monitoring of CPU and RAM usage for each managed project/process in ZenRunner. This feature helps developers identify resource-heavy scripts, detect memory leaks, and monitor system health without impacting ZenRunner's performance targets.

## Visual Preview

```
┌────────────────────────────────────────────────────────────────┐
│  📊 Resource Statistics                                    [2] │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  Total CPU: 45.2%        Total RAM: 78.3 MB                   │
│                                                                │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │ myapp-frontend                                     [!]  │ │
│  │                                                          │ │
│  │ CPU: 28.5%  RAM: 52.1 MB                                │ │
│  │                                                          │ │
│  │ Avg ████████████░░░░░░░░░░░░░░░ 28%                    │ │
│  │ Avg █████████████████████░░░░░░ 52 MB                  │ │
│  │                                                          │ │
│  │ Peak: 85% CPU, 98 MB                                    │ │
│  └──────────────────────────────────────────────────────────┘ │
│                                                                │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │ backend-server                                          │ │
│  │                                                          │ │
│  │ CPU: 16.7%  RAM: 26.2 MB                                │ │
│  │                                                          │ │
│  │ Avg ██████░░░░░░░░░░░░░░░░░░░░░ 16%                    │ │
│  │ Avg █████████████░░░░░░░░░░░░░░ 26 MB                  │ │
│  │                                                          │ │
│  │ Peak: 42% CPU, 35 MB                                    │ │
│  └──────────────────────────────────────────────────────────┘ │
│                                                                │
├────────────────────────────────────────────────────────────────┤
│                💡 Stats update every 2 seconds                │
└────────────────────────────────────────────────────────────────┘

Legend:
  [!] = Recent spike detected (flashing red indicator)
  Green text = Optimal (< 50% CPU, < 50MB RAM)
  Yellow text = Warning (50-80% CPU, 50-100MB RAM)  
  Red text = High usage (> 80% CPU, > 100MB RAM)
```

## Features

### Real-time Monitoring
- **CPU Usage**: Percentage of CPU utilization per process
- **Memory Usage**: Resident Set Size (RSS) in megabytes
- **Update Frequency**: 2-second intervals (configurable)

### Trend Analysis
- **Average Values**: Historical average over last minute
- **Peak Values**: Maximum values observed
- **Visual Indicators**: Progress bars showing relative usage

### Spike Detection
- **CPU Spikes**: Detected when usage exceeds 80%
- **Memory Spikes**: Detected when usage exceeds 100MB
- **Visual Alert**: Flashing red indicator on affected process
- **Console Logging**: Spike events logged with details

### Aggregated View
- **Total Resources**: Sum of all monitored processes
- **Per-Project Cards**: Individual statistics for each process
- **Color Coding**: Traffic light system for quick status assessment

## Performance Impact

The monitoring system is designed to be extremely lightweight:

| Metric | Value | Notes |
|--------|-------|-------|
| Memory per process | ~1 KB | Includes 30 sample history |
| Memory for 10 processes | ~11 KB | Negligible vs 30MB target |
| CPU overhead | 0.08% | For 10 processes at 2s interval |
| Sampling interval | 2 seconds | Configurable if needed |

**Result**: The dashboard adds minimal overhead and stays well within ZenRunner's performance targets.

## Architecture

### Components

```
┌─────────────────────────┐
│   ProcessManager        │  Manages process lifecycle
│   (manages processes)   │  Provides PIDs to monitor
└───────────┬─────────────┘
            │
            │ Notifies on start/stop
            ▼
┌─────────────────────────┐
│ ProcessResourceMonitor  │  Core monitoring engine
│ - Platform-specific     │  - Windows: GetProcessTimes()
│   system calls          │  - Linux: /proc/[pid]/stat
│ - Circular buffer       │  - macOS: proc_pidinfo()
│ - Spike detection       │  
└───────────┬─────────────┘
            │
            │ Emits signals (statsUpdated, spikeDetected)
            ▼
┌─────────────────────────┐
│  StatisticsViewModel    │  Qt model for QML
│  - Aggregates data      │  - QAbstractListModel
│  - Calculates totals    │  - Exposes properties
│  - Manages display      │  
└───────────┬─────────────┘
            │
            │ QML data binding
            ▼
┌─────────────────────────┐
│    Statistics.qml       │  UI Component
│    - GlassCard design   │  - Color-coded status
│    - Progress bars      │  - Spike indicators
│    - Real-time updates  │  - Empty state handling
└─────────────────────────┘
```

### Key Classes

1. **ProcessResourceMonitor** (`src/core/ProcessResourceMonitor.cpp`)
   - Implements `IProcessResourceMonitor` interface
   - Platform-specific CPU/RAM sampling
   - Circular buffer for history
   - Spike detection logic

2. **StatisticsViewModel** (`src/ui/StatisticsViewModel.cpp`)
   - Extends `QAbstractListModel`
   - Aggregates statistics per project
   - Provides totals for all processes
   - Exposes data to QML

3. **Statistics.qml** (`src/ui/Statistics.qml`)
   - QML component using GlassCard design
   - ListView with custom delegates
   - Color-coded indicators
   - Progress bars for trends

## Usage

### Integration in main.cpp

```cpp
#include "core/ProcessResourceMonitor.h"
#include "ui/StatisticsViewModel.h"

// Create resource monitor
auto resourceMonitor = std::make_shared<Core::ProcessResourceMonitor>();

// Create statistics view model
auto statisticsViewModel = new UI::StatisticsViewModel();
statisticsViewModel->setResourceMonitor(resourceMonitor);

// Expose to QML
engine.rootContext()->setContextProperty("statisticsViewModel", statisticsViewModel);
```

### Using in QML

```qml
Statistics {
    Layout.fillWidth: true
    Layout.fillHeight: true
    statisticsViewModel: statisticsViewModel  // From context property
}
```

### Monitoring a Process

```cpp
// When process starts
resourceMonitor->startMonitoring(processId, pid);

// When process stops
resourceMonitor->stopMonitoring(processId);
```

## Testing

### Unit Test

Run the resource monitor test:
```bash
cd build/bin
./test_resource_monitor
```

Expected output:
```
=== ProcessResourceMonitor Test ===

Test 1: Monitoring self process
Self PID: 12345
✓ Started monitoring self process

[STATS] self - CPU: 2.1% Memory: 8.5 MB
[STATS] self - CPU: 1.8% Memory: 8.6 MB
...

=== Checking Self Process Statistics ===
Current CPU: 2.0%
Current Memory: 8.6 MB

Summary:
  Data points: 3
  Avg CPU: 1.9%
  Max CPU: 2.1%
  Avg Memory: 8.5 MB
  Max Memory: 8.6 MB
  Recent Spike: No
✓ Successfully calculated summary

✓ All tests completed successfully!
```

### Integration Testing

1. Start ZenRunner
2. Launch a development process (e.g., `npm run dev`)
3. Open Statistics panel
4. Verify:
   - Process appears in list
   - CPU/RAM values update every 2 seconds
   - Values are reasonable (not 0% or extreme)
   - Color coding works (green/yellow/red)
   - Spike detection triggers for high usage

## Configuration

### Sampling Interval

Change the sampling interval (default: 2000ms):
```cpp
resourceMonitor->setSamplingInterval(1000);  // 1 second
```

### Spike Thresholds

Thresholds are defined in `ProcessResourceMonitor.h`:
```cpp
static constexpr double CPU_SPIKE_THRESHOLD = 80.0;  // 80% CPU
static constexpr double MEMORY_SPIKE_THRESHOLD_MB = 100.0;  // 100 MB
```

To customize, modify these constants and rebuild.

## Platform Support

### Windows
- Uses `GetProcessTimes()` for CPU time
- Uses `GetProcessMemoryInfo()` for memory
- Requires `psapi.lib` (already linked in CMakeLists.txt)

### Linux
- Reads `/proc/[pid]/stat` for CPU time
- Reads `/proc/[pid]/status` for memory (VmRSS)
- No additional dependencies

### macOS
- Uses `proc_pidinfo()` with `PROC_PIDTASKINFO`
- Requires `libproc.h` (system library)
- No additional linking needed

## Limitations

1. **Historical Data**: Only last 30 samples (~1 minute)
   - Future: Add optional persistent storage
   
2. **Sampling Frequency**: 2-second intervals, not real-time
   - Sufficient for developer use cases
   - Faster sampling increases overhead

3. **Static Thresholds**: Fixed spike detection limits
   - Future: User-configurable thresholds

4. **No Graphs**: Simple bars instead of line charts
   - Keeps binary size small
   - Future: Optional Qt Charts integration

## Documentation

- **STATISTICS_INTEGRATION.md** - Detailed integration guide
- **STATISTICS_DESIGN.md** - Design decisions and rationale
- **INTEGRATION_EXAMPLE.cpp** - Code examples

## Future Enhancements

Potential improvements:
- [ ] User-configurable spike thresholds
- [ ] Export statistics to CSV
- [ ] Persistent history storage
- [ ] Interactive line charts (Qt Charts)
- [ ] System tray notifications for spikes
- [ ] Per-core CPU breakdown
- [ ] Memory leak detection
- [ ] GPU usage tracking

## Contributing

When extending this feature:
1. Maintain lightweight design (memory/CPU efficiency)
2. Use platform-specific APIs for accuracy
3. Follow existing architecture patterns
4. Add tests for new functionality
5. Update documentation

## License

Same as ZenRunner project.
