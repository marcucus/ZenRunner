# QML Performance Optimization Guide

This document outlines the optimization strategies implemented in ZenRunner's QML UI to achieve 60 FPS rendering and minimal memory footprint.

## Memory Optimization

### 1. Efficient Component Hierarchy

**Problem**: Deep QML hierarchies increase memory overhead.

**Solution**:
```qml
// ❌ BAD: Unnecessary nesting
Rectangle {
    Rectangle {
        Rectangle {
            Text { text: "Hello" }
        }
    }
}

// ✅ GOOD: Flat hierarchy
Rectangle {
    Text { text: "Hello" }
}
```

**Implementation in ZenRunner**:
- Maximum nesting depth: 3-4 levels
- Use of `Item` containers instead of `Rectangle` when no visual properties needed
- Direct child placement in parent containers

### 2. ListView Optimization

**Circular Buffer for Logs** (`LogConsole.qml`):
```qml
ListView {
    id: logView
    cacheBuffer: 500  // Pre-cache 500px of content
    
    // Performance: O(1) access to visible items
    // Memory: Only visible items + cache buffer in memory
}
```

**Key optimizations**:
- `cacheBuffer` limits memory usage
- Delegates are created/destroyed as they scroll in/out
- Maximum 5000 log lines (enforced by circular buffer backend)

### 3. Lazy Loading

**Conditional Component Creation**:
```qml
// Only create complex components when needed
Loader {
    id: detailLoader
    active: false
    source: "ProjectDetailView.qml"
}

// Activate on demand
onClicked: detailLoader.active = true
```

**Used in**:
- Project detail views (loaded on selection)
- Workspace configurations (loaded on edit)
- Advanced settings panels

### 4. Property Bindings

**Avoid unnecessary bindings**:
```qml
// ❌ BAD: Binding updates every frame
width: parent.width * 0.5 + someValue * 2

// ✅ GOOD: Simple binding or explicit value
width: parent.width / 2
```

**Best practices in ZenRunner**:
- Direct property assignments where possible
- Minimize computed properties in bindings
- Use `Binding` object for complex conditional bindings

## GPU Acceleration

### 1. Qt Quick Scene Graph

**Automatic GPU acceleration** for:
- Rectangle rendering
- Text rendering with distance field fonts
- Image transformations
- Opacity and transform animations

**Implementation**:
```qml
// Leverages GPU batching
Rectangle {
    layer.enabled: false  // Let Qt decide batching
    // Qt Scene Graph batches multiple rectangles in single draw call
}
```

### 2. Shader Effects

**Native Platform Effects** (offloaded to OS compositor):
```cpp
// Windows: Mica effect uses DWM
DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, ...);

// macOS: Vibrancy uses NSVisualEffectView
NSVisualEffectView with hardware-accelerated blur
```

**Benefits**:
- Zero CPU/GPU cost for blur effects
- Consistent with system appearance
- Better battery life on laptops

### 3. Layer Optimization

**When to enable layers**:
```qml
Item {
    // Enable layer for complex items that change together
    layer.enabled: true
    layer.smooth: true
    
    // Multiple children rendered to single texture
    Rectangle { }
    Text { }
    Image { }
}
```

**ZenRunner usage**:
- Disabled by default (let Qt optimize)
- Enabled for animated particles (background effect)
- Enabled for complex glassmorphism overlays

## Animation Performance

### 1. Property Animations

**Use Behavior for smooth transitions**:
```qml
Rectangle {
    color: hovered ? "#ffffff" : "#000000"
    
    // GPU-accelerated color interpolation
    Behavior on color {
        ColorAnimation { duration: 150; easing.type: Easing.OutQuad }
    }
}
```

**Optimized properties**:
- `opacity`: GPU-accelerated
- `scale`: GPU-accelerated
- `rotation`: GPU-accelerated
- `color`: Interpolated on GPU

### 2. Animation Best Practices

**Do's**:
- Use `NumberAnimation` for numeric properties
- Keep animation duration reasonable (150-300ms)
- Use appropriate easing curves
- Prefer transform properties over position

**Don'ts**:
- Avoid animating `width`/`height` (triggers layout)
- Don't animate computed properties
- Avoid animating large lists

### 3. Sequential vs Parallel

```qml
// Particles use SequentialAnimation for organic motion
SequentialAnimation on y {
    loops: Animation.Infinite
    NumberAnimation { duration: 3000; easing.type: Easing.InOutSine }
    NumberAnimation { duration: 3000; easing.type: Easing.InOutSine }
}

// Use ParallelAnimation for combined effects
ParallelAnimation {
    NumberAnimation { property: "opacity"; to: 0 }
    NumberAnimation { property: "scale"; to: 0 }
}
```

## Rendering Optimization

### 1. Clip Only When Necessary

```qml
// ❌ BAD: Unnecessary clipping
Item {
    clip: true  // Expensive if content doesn't overflow
}

// ✅ GOOD: Clip only ListView/ScrollView
ListView {
    clip: true  // Required for scrolling
}
```

### 2. Opacity Optimization

```qml
// ✅ GOOD: Use opacity for hide/show
Item {
    opacity: visible ? 1.0 : 0.0
    visible: opacity > 0  // Still in hierarchy, but fast
}

// Even better: Remove from scene
Item {
    visible: someCondition  // Completely removed when false
}
```

### 3. Anti-aliasing

```qml
// Disable AA on simple rectangles
Rectangle {
    antialiasing: false  // Fast rendering for straight edges
}

// Enable for rotated/scaled items
Rectangle {
    antialiasing: true
    rotation: 45  // Needs AA for smooth edges
}
```

## JavaScript Performance

### 1. Minimize JS in QML

```qml
// ❌ BAD: Complex JS in binding
property var result: {
    let sum = 0;
    for (let i = 0; i < items.length; i++) {
        sum += items[i].value * 2;
    }
    return sum;
}

// ✅ GOOD: Move to C++ backend
property var result: projectModel.calculateSum()
```

### 2. Cache Calculations

```qml
Item {
    property int itemCount: model.count
    property int halfCount: itemCount / 2  // Cached
    
    // Reuse cached value
    width: parent.width / halfCount
}
```

### 3. Avoid Creating Objects

```qml
// ❌ BAD: Creates array every evaluation
property var items: [1, 2, 3, 4, 5]

// ✅ GOOD: Use model from C++
property var items: projectModel.items
```

## Memory Profiling

### Tools

1. **QML Profiler** (Qt Creator):
   - Shows memory allocations
   - Identifies memory leaks
   - Profiles rendering performance

2. **Valgrind** (Linux):
   ```bash
   valgrind --tool=massif ./ZenRunner
   ```

3. **Instruments** (macOS):
   - Allocations instrument
   - Leaks instrument
   - Time Profiler

### Target Metrics

| Metric | Target | Achieved |
|--------|--------|----------|
| Idle Memory | < 15 MB | TBD |
| Active Memory | < 30 MB | TBD |
| Framerate | 60 FPS | TBD |
| Startup Time | < 1 sec | TBD |

## Best Practices Summary

1. **Keep hierarchy flat** - Maximum 3-4 levels deep
2. **Use ListView for long lists** - Enables item recycling
3. **Enable cacheBuffer** - Pre-cache scrollable content
4. **Leverage GPU** - Use transform properties for animation
5. **Native effects** - Offload blur/transparency to OS
6. **Minimize JavaScript** - Move logic to C++ backend
7. **Profile regularly** - Use QML Profiler to identify bottlenecks
8. **Test on target hardware** - Optimize for actual deployment platforms

## Continuous Optimization

As the project grows:
- Run QML Profiler during development
- Monitor memory usage with each feature
- Benchmark framerate on target devices
- Review component hierarchy regularly
- Refactor deep nesting when found

## References

- [Qt Quick Performance Guide](https://doc.qt.io/qt-6/qtquick-performance.html)
- [QML Best Practices](https://doc.qt.io/qt-6/qml-codingconventions.html)
- [Qt Quick Scene Graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html)

---

Last Updated: 2026-01-05
