# ZenRunner UI - QML Architecture & Performance

## Overview

The ZenRunner UI is built with Qt Quick (QML) and optimized for consistent 60 FPS performance across all platforms. This document outlines the QML structure and performance optimizations.

## File Structure

```
src/ui/
├── Main.qml              - Application window and background
├── Dashboard.qml         - Main dashboard view
├── LogConsole.qml        - Terminal-style log viewer
├── ProjectView.qml       - Individual project detail view
├── WorkspaceManager.qml  - Workspace management interface
├── components/
│   ├── GlassCard.qml    - Reusable glassmorphism card
│   ├── GlassButton.qml  - Styled button component
│   └── qmldir           - QML module definition
└── README.md            - This file
```

## Performance Optimizations

### 1. GPU Acceleration (Scene Graph)

All UI components leverage Qt Quick's Scene Graph with hardware acceleration:

- **RHI Backend**: Uses Vulkan/Metal/Direct3D based on platform
- **Threaded Render Loop**: Rendering happens on dedicated GPU thread
- **Layer Compositing**: Complex effects pre-rendered to GPU textures

#### Layer Usage

Components can use `layer.enabled: true` for GPU compositing when needed:

```qml
Rectangle {
    layer.enabled: true      // Render to texture
    layer.smooth: true       // Bilinear filtering
    layer.samples: 4         // 4x MSAA antialiasing
}
```

**Applied selectively in**:
- Glass cards with effects (GlassCard.qml) - configurable via `enableLayer` property

**Note**: Layer compositing is now configurable in GlassCard to prevent performance issues when many cards are rendered simultaneously. Delegates and simple animated items do not use layers to conserve GPU memory.

### 2. ListView Optimization (LogConsole.qml)

The log console is heavily optimized for scrolling large log files:

```qml
ListView {
    cacheBuffer: 1000           // Cache 1000px of delegates
    asynchronous: true          // Load delegates off-thread
    pixelAligned: true          // Align to pixel boundaries
    maximumFlickVelocity: 5000  // Support fast scrolling
}
```

**Delegate optimization**:
- `layer.enabled: true` on delegate root
- `renderType: Text.NativeRendering` for text
- Minimal bindings in delegates
- Simple geometry

### 3. Animation Tuning

Animations are optimized to reduce GPU load:

**Particle System (Main.qml)**:
- Reduced from 20 to 10 particles
- Longer durations (4-7 seconds vs 3-5 seconds)
- Simplified rendering without GPU layers for better memory efficiency

**Status Indicators (Dashboard.qml)**:
- Optimized easing curves
- Consistent timing
- Opacity-only animations (GPU-friendly)

### 4. Text Rendering

Text uses native rendering for better performance:

```qml
Text {
    renderType: Text.NativeRendering  // Better performance
    font.family: "monospace"
    antialiasing: true
}
```

### 5. Glassmorphism Design

The glass card effect is optimized for performance:

**GlassCard.qml**:
- Single layer for entire effect
- Minimized nested rectangles
- Optimized gradient calculations
- Smooth transitions with proper easing

## Component Usage

### GlassCard

Reusable container with glassmorphism effect:

```qml
import "./components"

GlassCard {
    width: 300
    height: 200
    glassOpacity: 0.15        // Transparency level
    cornerRadius: 16          // Rounded corners
    enableLayer: true         // Enable GPU compositing for complex effects
    
    // Content goes here
    Text {
        text: "Card content"
        color: "#ffffff"
    }
}
```

**Performance tip**: Only set `enableLayer: true` for cards with complex visual effects or animations. Leave it as `false` (default) for simple cards to conserve GPU memory.

### GlassButton

Styled button with glass effect:

```qml
import "./components"

GlassButton {
    text: "Click Me"
    accentColor: "#4a90e2"    // Accent color
    onClicked: {
        // Handle click
    }
}
```

## Development Guidelines

### Do's

✅ Use `layer.enabled` for complex visual effects
✅ Use `asynchronous: true` for ListView delegates
✅ Use `renderType: Text.NativeRendering` for static text
✅ Animate opacity, x, y, scale, rotation
✅ Keep delegates lightweight
✅ Cache expensive calculations
✅ Use proper easing curves

### Don'ts

❌ Don't animate width, height, or anchors
❌ Don't use layer.enabled on simple items
❌ Don't create complex bindings in delegates
❌ Don't use synchronous operations in QML
❌ Don't nest too many layers
❌ Don't create items dynamically if avoidable

## Performance Testing

### Visual Debugging

Enable visual debugging with environment variables:

```bash
# Show overdraw (red = bad)
QSG_VISUALIZE=overdraw ./ZenRunner

# Show batches (colored rectangles)
QSG_VISUALIZE=batches ./ZenRunner

# Show render timing
QSG_RENDER_TIMING=1 ./ZenRunner
```

### Expected Metrics

| Component | Target | Optimization |
|-----------|--------|--------------|
| Main.qml | 60 FPS | Layer compositing, reduced particles |
| Dashboard.qml | 60 FPS | Optimized animations |
| LogConsole.qml | 60 FPS | Large cache buffer, async loading |
| GlassCard.qml | 60 FPS | Single layer, optimized gradients |

## Future Optimizations

Planned optimizations for future releases:

1. **Shader Effects**: Custom GLSL shaders for blur effects
2. **Worker Scripts**: Off-thread data processing
3. **Item Pooling**: Reuse delegate instances
4. **Texture Atlas**: Combine small textures
5. **Font Caching**: Pre-render common text

## References

- [Qt Quick Scene Graph](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html)
- [Qt Quick Performance](https://doc.qt.io/qt-6/qtquick-performance.html)
- [QML Best Practices](https://doc.qt.io/qt-6/qtquick-bestpractices.html)
- [Scene Graph Rendering](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph-renderer.html)
