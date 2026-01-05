# ZenRunner Performance Optimization Guide

## 60 FPS UI Performance Target

ZenRunner is designed to maintain a consistent 60 FPS (frames per second) across all supported platforms. This document details the technical optimizations implemented to achieve this goal.

## GPU Acceleration with RHI (Rendering Hardware Interface)

### Overview

Qt 6's Rendering Hardware Interface (RHI) provides a hardware abstraction layer that allows Qt Quick to use modern graphics APIs:

- **Windows**: Direct3D 11/12
- **macOS**: Metal
- **Linux**: Vulkan (preferred on systems with modern drivers, falls back to OpenGL on older systems)
- **Universal Fallback**: OpenGL ES 2.0 (ensures compatibility on all platforms)

### Configuration in main.cpp

The application configures GPU acceleration before the QGuiApplication is created:

```cpp
// Use threaded render loop for consistent 60 FPS
qputenv("QSG_RENDER_LOOP", "threaded");

// Enable RHI - Qt automatically selects the best available API
QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::Unknown);

// Set high quality antialiasing
QSurfaceFormat format;
format.setSamples(4);  // 4x MSAA for smooth edges
QSurfaceFormat::setDefaultFormat(format);
```

By using `GraphicsApi::Unknown`, Qt will automatically select:
- **Vulkan** on Linux (if available, otherwise OpenGL)
- **Metal** on macOS
- **Direct3D 11/12** on Windows
- **OpenGL ES 2.0** as fallback on any platform

### Threaded Render Loop

The threaded render loop is the most performant option for desktop applications:

- **Separation of Concerns**: Rendering happens on a dedicated thread
- **Consistent Frame Rate**: GPU rendering doesn't block the main UI thread
- **Better Resource Utilization**: CPU and GPU work in parallel

## QML Performance Optimizations

### 1. Layer-Based Compositing

Complex visual effects are rendered to GPU textures (layers) which are then composited:

```qml
Rectangle {
    layer.enabled: true      // Enable GPU layer
    layer.smooth: true       // Use bilinear filtering
    layer.samples: 4         // 4x MSAA antialiasing
}
```

**Benefits**:
- Reduces overdraw by pre-rendering complex effects
- Leverages GPU texture caching
- Improves animation performance

**Applied to**:
- Glass cards when needed (GlassCard.qml - configurable via `enableLayer` property)

**Note**: Layer compositing is used sparingly to avoid GPU memory overhead. Simple animations and delegates render directly without layers for better memory efficiency.

### 2. ListView Optimizations

The log console ListView is heavily optimized for scrolling performance:

```qml
ListView {
    cacheBuffer: 1000           // Cache 1000px worth of delegates
    asynchronous: true          // Load delegates asynchronously
    pixelAligned: true          // Align to pixel boundaries
    maximumFlickVelocity: 5000  // Fast scrolling support
}
```

**Key optimizations**:
- **Increased Cache Buffer**: Reduces delegate creation during scrolling
- **Asynchronous Loading**: Delegates load off the main thread
- **Pixel Alignment**: Eliminates sub-pixel rendering artifacts
- **Native Text Rendering**: `renderType: Text.NativeRendering` for better performance

### 3. Animation Optimizations

Animations are tuned to minimize GPU load:

**Background Particles**:
- Reduced count from 20 to 10 particles
- Longer animation durations (4-7 seconds instead of 3-5 seconds)
- Simplified easing functions where possible

**Status Indicators**:
- Added easing curves to all animations
- Consistent animation timing for predictable GPU load

### 4. Reduced Visual Complexity

Where appropriate, visual complexity has been reduced:

- Optimized gradients and borders
- Simplified shader effects
- Minimal use of transparency layers

## Performance Monitoring

### Debug Information

To monitor performance in development, set these environment variables:

```bash
# Show Scene Graph rendering information
export QSG_INFO=1

# Show frame timing
export QSG_RENDER_TIMING=1

# Visualize overdraw
export QSG_VISUALIZE=overdraw

# Visualize batching
export QSG_VISUALIZE=batches
```

### Expected Performance Metrics

| Metric | Target | Acceptable Range |
|--------|--------|------------------|
| Frame Rate | 60 FPS | 58-60 FPS |
| Frame Time | 16.67 ms | 14-17 ms |
| RAM Usage (Idle) | < 15 MB | 10-20 MB |
| RAM Usage (Active) | < 30 MB | 25-35 MB |

## Platform-Specific Optimizations

### Windows

- Uses Direct3D 11/12 via RHI
- Supports Mica material effects (Windows 11+)
- Hardware-accelerated composition

### macOS

- Uses Metal API via RHI
- Supports Vibrancy effects
- Optimized for Retina displays
- Special handling for Notch areas on MacBook Pro

### Linux

- Uses Vulkan via RHI (if available)
- Falls back to OpenGL if Vulkan not supported
- Optimized for X11 and Wayland compositors

## Troubleshooting Performance Issues

### Issue: Low FPS (< 58 FPS)

**Diagnosis**:
```bash
QSG_INFO=1 QSG_RENDER_TIMING=1 ./ZenRunner
```

**Common Causes**:
1. GPU driver issues - Update graphics drivers
2. Compositor conflicts on Linux - Try different render loop
3. Too many visible items - Increase ListView cacheBuffer

**Solutions**:
```bash
# Try basic render loop
QSG_RENDER_LOOP=basic ./ZenRunner

# Force OpenGL backend
QSG_RHI_BACKEND=opengl ./ZenRunner

# Disable vsync (testing only)
QSG_NO_VSYNC=1 ./ZenRunner
```

### Issue: High Memory Usage

**Diagnosis**:
- Check number of cached delegates in ListView
- Monitor particle count in animations
- Review layer.enabled usage

**Solutions**:
- Reduce ListView cacheBuffer
- Disable background particles
- Limit layer.enabled to essential components

### Issue: Stuttering Animations

**Causes**:
- Main thread blocking
- Synchronous operations
- Large model updates

**Solutions**:
- Ensure all backend operations are asynchronous
- Use WorkerScript for heavy computations
- Batch model updates

## Best Practices for Future Development

1. **Always use layer.enabled for**:
   - Complex visual effects (blur, shadows)
   - Items with multiple children and opacity
   - Frequently animated items

2. **Avoid layer.enabled for**:
   - Simple rectangles or text
   - Items that change size frequently
   - Deeply nested hierarchies
   - Multiple simple cards (use `enableLayer: false` on GlassCard)

3. **ListView optimization**:
   - Keep delegates lightweight
   - Use cached properties
   - Avoid bindings in delegates when possible

4. **Animation guidelines**:
   - Prefer animating opacity, x, y, scale, rotation
   - Avoid animating width, height, or anchors
   - Use NumberAnimation with proper easing

5. **Text rendering**:
   - Use `renderType: Text.NativeRendering` for static text
   - Use `renderType: Text.QtRendering` for scaled text
   - Cache font metrics when possible

## Measuring Success

Run the application with performance monitoring:

```bash
QSG_RENDER_TIMING=1 ./ZenRunner
```

Expected output should show:
```
Frame rendered with 60 FPS
Sync: 1.2ms, Render: 8.5ms, Swap: 2.1ms, Total: 11.8ms
```

A well-optimized frame should have:
- Sync time: < 2 ms
- Render time: < 10 ms
- Swap time: < 3 ms
- Total time: < 16 ms (leaving headroom)

## References

- [Qt Quick Scene Graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html)
- [Qt Quick Performance](https://doc.qt.io/qt-6/qtquick-performance.html)
- [Qt RHI Documentation](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph-renderer.html)
- [QML Profiler](https://doc.qt.io/qt-6/qtquick-tools-and-utilities.html#qml-profiler)
