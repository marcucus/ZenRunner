# Statistical Dashboard Integration Guide

This document explains how to integrate the new statistical dashboard components into the ZenRunner application.

## Overview

The statistical dashboard feature tracks CPU and RAM usage per managed project/process using:

1. **ProcessResourceMonitor** - Core monitoring engine that samples resource usage
2. **StatisticsViewModel** - UI bridge that exposes statistics to QML
3. **Statistics.qml** - UI component that displays the statistics

## Architecture

```
┌─────────────────────────┐
│   ProcessManager        │
│   (manages processes)   │
└───────────┬─────────────┘
            │
            │ When process starts
            │ share PID with
            ▼
┌─────────────────────────┐
│ ProcessResourceMonitor  │
│ - Samples CPU/RAM       │
│ - Detects spikes        │
│ - Stores history        │
└───────────┬─────────────┘
            │
            │ Emits signals
            │ statsUpdated
            ▼
┌─────────────────────────┐
│  StatisticsViewModel    │
│  - Aggregates stats     │
│  - QML model            │
└───────────┬─────────────┘
            │
            │ QML binding
            ▼
┌─────────────────────────┐
│    Statistics.qml       │
│    - Displays UI        │
│    - Shows trends       │
│    - Spike indicators   │
└─────────────────────────┘
```

## Integration Steps

### 1. Create Monitor Instance in main.cpp

```cpp
#include "core/ProcessResourceMonitor.h"
#include "ui/StatisticsViewModel.h"

// In your main() or application setup:
auto resourceMonitor = std::make_shared<ZenRunner::Core::ProcessResourceMonitor>();
auto statisticsViewModel = new ZenRunner::UI::StatisticsViewModel();
statisticsViewModel->setResourceMonitor(resourceMonitor);

// Expose to QML
engine.rootContext()->setContextProperty("statisticsViewModel", statisticsViewModel);
```

### 2. Connect ProcessManager to Monitor

When a process starts in ProcessManager, notify the resource monitor:

```cpp
// In ProcessManager::startProcess() after process successfully starts
void ProcessManager::onProcessStarted(const QString& processId) {
    const qint64 pid = process->processId();
    
    // Start monitoring
    if (resourceMonitor_) {
        resourceMonitor_->startMonitoring(processId, pid);
    }
}

// When process stops
void ProcessManager::onProcessFinished(const QString& processId) {
    if (resourceMonitor_) {
        resourceMonitor_->stopMonitoring(processId);
    }
}
```

### 3. Connect StatisticsViewModel in Dashboard.qml

The Statistics.qml component is already added to Dashboard.qml, but needs the view model:

```qml
// In Dashboard.qml, find the Statistics component:
Statistics {
    id: statisticsPanel
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    // Set the view model (exposed from C++)
    statisticsViewModel: statisticsViewModel  // From context property
}
```

## Features

### Resource Monitoring
- **Sampling Interval**: 2 seconds (configurable)
- **History Size**: 30 samples (~1 minute of data)
- **Metrics Tracked**:
  - CPU usage percentage (0-100%+)
  - Memory usage in MB (RSS - Resident Set Size)

### Spike Detection
- **CPU Spike**: > 80% CPU usage
- **Memory Spike**: > 100 MB memory usage
- Spikes are tracked in the last minute of data

### Platform Support
- **Windows**: Uses `GetProcessTimes()` and `GetProcessMemoryInfo()`
- **Linux**: Reads `/proc/[pid]/stat` and `/proc/[pid]/status`
- **macOS**: Uses `proc_pidinfo()` with PROC_PIDTASKINFO

## UI Display

The Statistics.qml component shows:

1. **Total Resources**: Sum of all monitored processes
2. **Per-Process Cards**:
   - Project name (extracted from processId)
   - Current CPU and RAM usage
   - Average and peak usage bars
   - Spike indicator (flashing red dot)
3. **Empty State**: Friendly message when no processes are running

### Color Coding
- **Green**: Optimal usage (CPU < 50%, RAM < 50MB)
- **Yellow**: Warning (CPU 50-80%, RAM 50-100MB)
- **Red**: High usage (CPU > 80%, RAM > 100MB)

## Performance Impact

The monitoring system is designed to be lightweight:

- **Memory**: ~100 bytes per monitored process + 30 samples * 24 bytes ≈ 820 bytes per process
- **CPU**: Minimal - sampling happens every 2 seconds using system APIs
- **Thread**: Uses Qt's event loop, no additional threads required

## Testing

Run the test executable:
```bash
./bin/test_resource_monitor
```

This test:
1. Monitors the test process itself
2. Spawns a child process and monitors it
3. Collects statistics over 6 seconds
4. Displays summaries and validates spike detection

## Example Output

Console output when monitoring processes:
```
[STATS] myapp:dev - CPU: 12.3% Memory: 45.2 MB
[STATS] myapp:test - CPU: 85.1% Memory: 120.5 MB
[SPIKE] myapp:test - CPU: 85.1% Memory: 120.5 MB
```

QML UI shows real-time updates with color-coded indicators and trend bars.

## Future Enhancements

Potential improvements:
- Historical graphs using Qt Charts
- Export statistics to CSV
- Configurable spike thresholds
- Process comparison view
- Alerts and notifications
- Integration with system tray status
