# ZenRunner UI Component Architecture

## Component Hierarchy

```
┌─────────────────────────────────────────────────────────────────┐
│                        ApplicationWindow                         │
│                          (Main.qml)                              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                   Background Layer                         │  │
│  │  • Gradient (dark theme: #1a1a2e → #0f0f1e)              │  │
│  │  • Animated particles (20x floating dots)                 │  │
│  └───────────────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                    Dashboard.qml                           │  │
│  │  ┌─────────────────────────────────────────────────────┐  │  │
│  │  │              Header (GlassCard)                      │  │  │
│  │  │  • Title: "ZenRunner"                               │  │  │
│  │  │  • Buttons: Import Project, New Workspace           │  │  │
│  │  └─────────────────────────────────────────────────────┘  │  │
│  │  ┌───────────────────────────┬─────────────────────────┐  │  │
│  │  │   Projects (GlassCard)    │  Sidebar (Column)       │  │  │
│  │  │  • Project list           │  ┌───────────────────┐  │  │  │
│  │  │  • Empty state            │  │ Workspaces (Glass)│  │  │  │
│  │  │  • Scan button            │  │ • Workspace list  │  │  │  │
│  │  │                           │  └───────────────────┘  │  │  │
│  │  │                           │  ┌───────────────────┐  │  │  │
│  │  │                           │  │ System Status     │  │  │  │
│  │  │                           │  │ • Memory: <15MB   │  │  │  │
│  │  │                           │  │ • Processes: 0    │  │  │  │
│  │  │                           │  │ • FPS: 60         │  │  │  │
│  │  │                           │  └───────────────────┘  │  │  │
│  │  └───────────────────────────┴─────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Glassmorphism Components

### GlassCard Structure
```
┌──────────────────────────────────────────┐
│ Outer Rectangle (rounded corners)        │
│  color: rgba(255, 255, 255, 0.15)       │
│  border: 1px rgba(255, 255, 255, 0.3)   │
│  ┌────────────────────────────────────┐ │
│  │ Inner Rectangle (highlight)         │ │
│  │  border: 1px rgba(255, 255, 255, 0.1) │
│  └────────────────────────────────────┘ │
│  ┌────────────────────────────────────┐ │
│  │ Gradient Overlay                    │ │
│  │  top: rgba(255, 255, 255, 0.05)    │ │
│  │  bottom: rgba(0, 0, 0, 0.05)       │ │
│  └────────────────────────────────────┘ │
│  ┌────────────────────────────────────┐ │
│  │ Content Area (children)             │ │
│  │  • Text                             │ │
│  │  • Buttons                          │ │
│  │  • Other components                 │ │
│  └────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

### GlassButton States
```
Normal State:           Hovered State:          Pressed State:
┌────────────┐         ┌────────────┐          ┌────────────┐
│▌ Button   │         │▌ Button   │          │▌ Button   │
└────────────┘         └────────────┘          └────────────┘
opacity: 0.15          opacity: 0.25           opacity: 0.35
accent: visible        accent: bright          accent: very bright
```

## View Compositions

### Dashboard Layout
```
┌─────────────────────────────────────────────────────────┐
│ Header: ZenRunner                 [Import] [Workspace]  │
├───────────────────────────────┬─────────────────────────┤
│                               │                         │
│  Projects Panel (60%)         │  Sidebar (40%)         │
│  ┌─────────────────────────┐  │  ┌──────────────────┐  │
│  │ Projects      [0]    📦 │  │  │ Workspaces       │  │
│  ├─────────────────────────┤  │  │  🗂️ No workspaces│  │
│  │                         │  │  └──────────────────┘  │
│  │   📦 No projects yet    │  │  ┌──────────────────┐  │
│  │   Import to start       │  │  │ System Status    │  │
│  │                         │  │  │ ● Memory <15MB   │  │
│  │                         │  │  │ ● Processes: 0   │  │
│  │                         │  │  │ ● FPS: 60        │  │
│  └─────────────────────────┘  │  └──────────────────┘  │
│                               │                         │
└───────────────────────────────┴─────────────────────────┘
```

### ProjectView Layout
```
┌────────────────────────────────────────────────┐
│ 📦 Sample Project                    ● Running │
│ /path/to/project                               │
├────────────────────────────────────────────────┤
│ Available Scripts                              │
│ ┌─────┐ ┌─────┐ ┌─────┐                       │
│ │⚡dev│ │🔨bld│ │✓tst│                       │
│ └─────┘ └─────┘ └─────┘                       │
│ ┌─────┐ ┌─────┐ ┌─────┐                       │
│ │🔍lnt│ │▶sta│ │🚀dpl│                       │
│ └─────┘ └─────┘ └─────┘                       │
├────────────────────────────────────────────────┤
│ [View Logs] [Terminal] [Settings]             │
└────────────────────────────────────────────────┘
```

### LogConsole Layout
```
┌────────────────────────────────────────────────┐
│ 📋 Console Logs              🔍 🗑️            │
├────────────────────────────────────────────────┤
│ ┌────────────────────────────────────────────┐ │
│ │ 00:00:00 ● [ZenRunner] Initialized        │ │
│ │ 00:00:01 ● [System] Ready                 │ │
│ │ 00:00:02 ● [Info] Waiting for commands    │ │
│ │ ...                                        │ │
│ │                                            │ │
│ │                                            │ │
│ └────────────────────────────────────────────┘ │
│ Lines: 3/5000 | Auto-scroll: ON | ⚡ Buffer   │
└────────────────────────────────────────────────┘
```

### WorkspaceManager Layout
```
┌────────────────────────────────────────────────┐
│ 🗂️ Workspace Manager      [+ Create Workspace]│
├────────────────────────────────────────────────┤
│ ┌──────────────────────────────────────────┐  │
│ │ F  Full Stack Development        ● Active│  │
│ │    3 projects                            │  │
│ │    [⏸ Stop All] [⚙️] [📊]               │  │
│ └──────────────────────────────────────────┘  │
│ ┌──────────────────────────────────────────┐  │
│ │ M  Microservices                 ○ Idle  │  │
│ │    5 projects                            │  │
│ │    [▶ Start All] [⚙️] [📊]              │  │
│ └──────────────────────────────────────────┘  │
│ ┌──────────────────────────────────────────┐  │
│ │ F  Frontend Only                 ○ Idle  │  │
│ │    1 project                             │  │
│ │    [▶ Start All] [⚙️] [📊]              │  │
│ └──────────────────────────────────────────┘  │
└────────────────────────────────────────────────┘
```

## Platform Integration Architecture

### Windows 11 - Mica Effect
```
┌─────────────────────────────────────────┐
│  ZenRunner Window (HWND)                │
│  ┌───────────────────────────────────┐  │
│  │ QML Content (transparent)         │  │
│  │  • Dashboard                      │  │
│  │  • GlassCards (semi-transparent) │  │
│  └───────────────────────────────────┘  │
│         ↕ Composited by                 │
│  ┌───────────────────────────────────┐  │
│  │ DWM Mica Backdrop                 │  │
│  │ (samples desktop wallpaper)       │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### macOS - Vibrancy Effect
```
┌─────────────────────────────────────────┐
│  ZenRunner NSWindow                     │
│  ┌───────────────────────────────────┐  │
│  │ QML Content (transparent)         │  │
│  │  • Dashboard                      │  │
│  │  • GlassCards (semi-transparent) │  │
│  └───────────────────────────────────┘  │
│         ↕ Rendered over                 │
│  ┌───────────────────────────────────┐  │
│  │ NSVisualEffectView                │  │
│  │ (blurs background content)        │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### Generic Fallback
```
┌─────────────────────────────────────────┐
│  ZenRunner Window                       │
│  ┌───────────────────────────────────┐  │
│  │ Solid Background (#1a1a2e)        │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ QML Content                 │  │  │
│  │  │  • Dashboard                │  │  │
│  │  │  • GlassCards (visual only) │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## Color Palette

### Primary Colors
```
#4a90e2 ████ Blue (Primary actions)
#7c4dff ████ Purple (Workspaces)
#4ade80 ████ Green (Success/Running)
#fbbf24 ████ Yellow (Warnings)
#ef4444 ████ Red (Errors/Stop)
```

### Theme Colors
```
#ffffff ████ White (Text)
#888888 ████ Gray (Subtitles)
#1a1a2e ████ Dark Blue (Background top)
#0f0f1e ████ Darker Blue (Background bottom)
#0a0a0a ████ Black (Console background)
```

### Glass Effect
```
Opacities:
- Background: 15% (0.15)
- Border: 30% (0.3)
- Hover: 25% (0.25)
- Pressed: 35% (0.35)
```

## Animation Timing

```
Transitions:
  Color changes: 150ms OutQuad
  Opacity fades: 200ms OutQuad
  Scale changes: 200ms OutQuad

Particles:
  Float cycle: 3000-5000ms InOutSine
  Opacity pulse: 2000-3000ms InOutQuad
```

## Memory Layout

```
Application Memory (Target: < 30 MB)
├── Qt Framework (≈10 MB)
├── QML Engine (≈5 MB)
├── Scene Graph (≈3 MB)
├── UI Components (≈2 MB)
│   ├── Main window
│   ├── Dashboard
│   ├── Components (cached)
│   └── Resources
├── Platform Layer (≈1 MB)
│   ├── Native effects
│   └── OS integration
└── Application Logic (≈5 MB)
    ├── Models (when connected)
    └── Data buffers
```

## Performance Characteristics

### Rendering Pipeline
```
1. QML Declaration
   ↓
2. Qt Quick Scene Graph
   ↓
3. GPU Batching (via RHI)
   ↓
4. Hardware Rendering
   • Direct3D (Windows)
   • Metal (macOS)
   • OpenGL/Vulkan (Linux)
   ↓
5. Compositor Integration
   • DWM (Windows)
   • Window Server (macOS)
   • X11/Wayland (Linux)
```

### Frame Budget (16.67ms for 60 FPS)
```
QML Update:      2 ms
Scene Graph:     8 ms
GPU Rendering:   5 ms
Compositor:      1 ms
Buffer Swap:     0.67 ms
─────────────────────
Total:          16.67 ms ✓
```

---

This architecture achieves the goals of:
- Modern glassmorphism aesthetic
- Native platform integration
- Optimal performance (60 FPS)
- Minimal memory footprint (< 30 MB)
- Maintainable component structure
