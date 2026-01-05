# ZenRunner Build and Performance Testing Guide

## Prerequisites

### Required Tools

- **CMake**: 3.21 or higher
- **Qt 6**: 6.2 or higher (with Qt Quick module)
- **C++20 Compiler**:
  - GCC 10+
  - Clang 12+
  - MSVC 2019+

### Installing Qt 6

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev
```

#### macOS (Homebrew)
```bash
brew install qt@6
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"
```

#### Windows
Download and install Qt from: https://www.qt.io/download-open-source-software

## Building ZenRunner

### Standard Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Run
./bin/ZenRunner
```

### Platform-Specific Builds

#### Windows with Visual Studio
```bash
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### macOS with Xcode
```bash
cmake .. -G Xcode -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### Linux with Ninja
```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

## Performance Testing

### Visual Performance Debugging

Qt provides several environment variables for performance analysis:

#### 1. Scene Graph Rendering Information
```bash
export QSG_INFO=1
./bin/ZenRunner
```

**Output shows**:
- Graphics API used (Vulkan/Metal/Direct3D/OpenGL)
- Render loop type
- Batch statistics
- Texture uploads

#### 2. Frame Timing
```bash
export QSG_RENDER_TIMING=1
./bin/ZenRunner
```

**Expected output**:
```
Frame rendered with 60 FPS
Sync: 1-2ms, Render: 8-10ms, Swap: 2-3ms, Total: 11-15ms
```

**Frame time breakdown**:
- **Sync**: CPU→GPU synchronization (should be < 2ms)
- **Render**: GPU rendering time (should be < 10ms)
- **Swap**: Buffer swap time (should be < 3ms)
- **Total**: Should be < 16ms for 60 FPS

#### 3. Visualize Overdraw
```bash
export QSG_VISUALIZE=overdraw
./bin/ZenRunner
```

**Color coding**:
- Blue: Drawn once
- Green: Drawn 2-3 times
- Yellow: Drawn 4-5 times
- Red: Drawn 6+ times (BAD - indicates performance issues)

**Target**: Most of the screen should be blue/green

#### 4. Visualize Batches
```bash
export QSG_VISUALIZE=batches
./bin/ZenRunner
```

Shows how Qt batches draw calls. Fewer batches = better performance.

#### 5. Visualize Clipping
```bash
export QSG_VISUALIZE=clip
./bin/ZenRunner
```

Shows clipping regions. Yellow = clipped areas.

#### 6. All Debug Info
```bash
export QSG_INFO=1
export QSG_RENDER_TIMING=1
export QSG_VISUALIZE=batches
./bin/ZenRunner
```

### Performance Metrics to Monitor

#### 1. Frame Rate
Target: **Consistent 60 FPS**

Test scenarios:
- Idle dashboard (should be 60 FPS)
- Background particle animations (should maintain 60 FPS)
- Log console with scrolling (should maintain 60 FPS)
- Opening/closing glass cards (smooth transitions)

#### 2. Memory Usage
Target: **< 30 MB active, < 15 MB idle**

```bash
# Linux
ps aux | grep ZenRunner | awk '{print $6/1024 " MB"}'

# macOS
ps -o rss= -p $(pgrep ZenRunner) | awk '{print $1/1024 " MB"}'

# Windows (PowerShell)
Get-Process ZenRunner | Select-Object WorkingSet64
```

#### 3. CPU Usage
Target: **< 5% idle, < 15% active**

- Should drop to near-zero when idle
- Should spike briefly during animations
- Should be low during steady-state scrolling

### QML Profiler

For detailed performance analysis:

```bash
# Enable QML profiler
export QT_QML_DEBUG_SERVICE=QmlProfiler
./bin/ZenRunner
```

Then connect with Qt Creator's QML Profiler:
1. Open Qt Creator
2. Go to Analyze → QML Profiler
3. Attach to running process

**What to look for**:
- Long-running JavaScript functions (> 5ms)
- Frequent property bindings evaluations
- Large model updates
- Texture uploads

## Validation Checklist

### Build Validation

- [ ] CMake configures without errors
- [ ] All source files compile
- [ ] Qt libraries link successfully
- [ ] QML files are embedded in resources
- [ ] Executable runs without crashing

### Performance Validation

#### Startup Performance
- [ ] Application starts in < 2 seconds
- [ ] UI appears immediately
- [ ] No visible lag or stuttering on first render

#### Animation Performance
- [ ] Background particles animate smoothly at 60 FPS
- [ ] Glass card hover effects are smooth
- [ ] Button press animations are instant
- [ ] Status indicator pulses are smooth

#### Scrolling Performance
- [ ] Log console scrolls smoothly with 1000+ lines
- [ ] No frame drops during fast scrolling
- [ ] New log entries don't cause stuttering
- [ ] ListView delegates load asynchronously

#### GPU Acceleration
- [ ] RHI reports correct graphics API:
  - Windows: Direct3D 11/12
  - macOS: Metal
  - Linux: Vulkan (or OpenGL fallback)
- [ ] Threaded render loop is active
- [ ] Layer compositing is working (check with QSG_INFO=1)

#### Memory Management
- [ ] Memory usage stays < 30 MB
- [ ] No memory leaks during 10 minutes of use
- [ ] Log buffer respects 5000 line limit
- [ ] No texture memory leaks

### Visual Quality Validation

- [ ] 4x MSAA provides smooth edges
- [ ] Text is crisp and readable
- [ ] Glassmorphism effects look good
- [ ] Colors match design specifications
- [ ] No visual artifacts or glitches

## Troubleshooting

### Issue: Build Fails with "Qt6 not found"

**Solution**:
```bash
# Set Qt installation path
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64

# Or on macOS with Homebrew
export CMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

# Then reconfigure
cmake .. -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH
```

### Issue: Application crashes on startup

**Diagnosis**:
```bash
# Run with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run with debugger
gdb ./bin/ZenRunner
# or
lldb ./bin/ZenRunner
```

### Issue: Low FPS (< 58 FPS)

**Diagnosis**:
```bash
QSG_INFO=1 QSG_RENDER_TIMING=1 ./bin/ZenRunner
```

**Common causes**:
1. Wrong graphics API - Check QSG_INFO output
2. Basic render loop instead of threaded
3. Too many layers enabled
4. Driver issues

**Solutions**:
```bash
# Force threaded render loop
export QSG_RENDER_LOOP=threaded

# Try different graphics backend
export QSG_RHI_BACKEND=vulkan  # or opengl, metal, d3d11

# Disable layers temporarily for testing
export QML_DISABLE_DISTANCEFIELD=1
```

### Issue: High memory usage

**Diagnosis**:
- Check ListView cacheBuffer (too high?)
- Monitor texture uploads with QSG_INFO=1
- Check for leaked QML objects

**Solutions**:
- Reduce ListView cacheBuffer to 500
- Disable particle animations
- Limit layer.enabled usage

### Issue: Stuttering animations

**Causes**:
- Main thread blocking
- Synchronous operations
- Too many property bindings

**Solutions**:
- Profile with QML Profiler
- Move computations to WorkerScript
- Simplify bindings in delegates

## Continuous Integration

### Automated Performance Tests

Create a test script `test_performance.sh`:

```bash
#!/bin/bash

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run with metrics
export QSG_RENDER_TIMING=1
timeout 60s ./build/bin/ZenRunner > perf.log 2>&1 &
PID=$!

# Wait and collect metrics
sleep 30

# Check if process is still running (no crash)
if kill -0 $PID 2>/dev/null; then
    echo "✓ Application runs without crashing"
    
    # Check memory
    MEM=$(ps -o rss= -p $PID | awk '{print $1/1024}')
    if (( $(echo "$MEM < 50" | bc -l) )); then
        echo "✓ Memory usage OK: ${MEM}MB"
    else
        echo "✗ Memory usage too high: ${MEM}MB"
    fi
    
    # Check for frame timing
    if grep -q "60 FPS" perf.log; then
        echo "✓ 60 FPS achieved"
    else
        echo "✗ FPS target not met"
    fi
    
    kill $PID
else
    echo "✗ Application crashed"
    exit 1
fi
```

## Benchmarking

### Performance Baseline

Record these metrics for comparison:

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| Startup time | < 2s | | |
| Idle FPS | 60 | | |
| Animation FPS | 60 | | |
| Scroll FPS | 60 | | |
| Idle RAM | < 15MB | | |
| Active RAM | < 30MB | | |
| Idle CPU | < 2% | | |
| Active CPU | < 15% | | |

### Test Scenarios

1. **Idle Performance**
   - Leave application idle for 5 minutes
   - Check FPS, RAM, CPU
   - Verify no memory leaks

2. **Animation Stress**
   - Observe background particles
   - Open/close multiple cards
   - Verify smooth 60 FPS

3. **Log Console Stress**
   - Generate 5000 log entries
   - Scroll through logs rapidly
   - Verify no frame drops

4. **Memory Stress**
   - Run for 30 minutes
   - Perform various operations
   - Check for memory leaks

## References

- [Qt Quick Scene Graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html)
- [Qt RHI Documentation](https://doc.qt.io/qt-6/rhi.html)
- [QML Performance](https://doc.qt.io/qt-6/qtquick-performance.html)
- [QML Profiler](https://doc.qt.io/qtcreator/creator-qml-performance-monitor.html)
