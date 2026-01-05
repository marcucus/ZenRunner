# Quick Reference: ZenRunner Interfaces

## Overview

ZenRunner implements a modular architecture with 17 interface files across 4 layers, plus a factory pattern for object creation.

## Interface Structure

```
include/
├── IApplicationFactory.hpp      # Factory for all interfaces
├── core/                         # Business logic (5 interfaces)
│   ├── IProcessManager.hpp       # Process execution & monitoring
│   ├── IProject.hpp              # Project data model
│   ├── IWorkspace.hpp            # Workspace management
│   ├── IJsonParser.hpp           # JSON parsing
│   └── ILogBuffer.hpp            # Circular log buffer
├── platform/                     # OS integration (4 interfaces)
│   ├── IPlatformEffect.hpp       # Native visual effects
│   ├── ISystemTray.hpp           # System tray
│   ├── INativeNotifications.hpp  # Native notifications
│   └── IPlatformUtils.hpp        # Platform utilities
├── storage/                      # Data persistence (3 interfaces)
│   ├── ISettingsManager.hpp      # Application settings
│   ├── IProjectRepository.hpp    # Project persistence
│   └── IWorkspaceRepository.hpp  # Workspace persistence
├── ui/                           # QML view models (3 interfaces)
│   ├── ILogViewModel.hpp         # Log display model
│   ├── IProjectViewModel.hpp     # Project view model
│   └── IWorkspaceViewModel.hpp   # Workspace view model
└── types/
    └── CommonTypes.hpp           # Shared types

Total: 17 files, 4 layers
```

## Quick Start

### 1. Get the Factory

```cpp
#include "IApplicationFactory.hpp"
auto& factory = ZenRunner::getApplicationFactory();
```

### 2. Create Objects

```cpp
// Core
auto processManager = factory.createProcessManager();
auto project = factory.createProject("/path");
auto workspace = factory.createWorkspace("name");
auto jsonParser = factory.createJsonParser();
auto logBuffer = factory.createLogBuffer(5000);

// Platform
auto platformEffect = factory.createPlatformEffect();
auto systemTray = factory.createSystemTray();
auto notifications = factory.createNativeNotifications();
auto platformUtils = factory.createPlatformUtils();

// Storage
auto settings = factory.createSettingsManager();
auto projectRepo = factory.createProjectRepository();
auto workspaceRepo = factory.createWorkspaceRepository();

// UI
auto logViewModel = factory.createLogViewModel(logBuffer);
auto projectViewModel = factory.createProjectViewModel(project);
auto workspaceViewModel = factory.createWorkspaceViewModel(workspace);
```

### 3. Use the Interfaces

```cpp
// Start a process
processManager->startProcess("id", "npm", {"run", "dev"}, "/path");

// Pin a script
project->pinScript("dev");

// Add project to workspace
workspace->addProject(project);

// Save to disk
projectRepo->saveProject(*project);

// Apply platform effect
platformEffect->applyEffect(windowId, EffectType::Mica);
```

## Layer Dependencies

```
┌──────────────┐
│   UI Layer   │  depends on Core
└──────┬───────┘
       ↓
┌──────────────┐
│  Core Layer  │  foundation
└──────┬───────┘
       ↑
┌──────────────┐
│Storage Layer │  depends on Core
└──────────────┘

┌──────────────┐
│Platform Layer│  independent
└──────────────┘
```

## Key Concepts

### Smart Pointers

- `unique_ptr`: Exclusive ownership (managers, repositories)
- `shared_ptr`: Shared ownership (projects, workspaces)

### Qt Integration

- `QObject`: For signals/slots
- `Q_INVOKABLE`: For QML methods
- `QAbstractListModel`: For QML lists

### C++20 Features

- `[[nodiscard]]`: Critical return values
- Smart pointers
- Designated initializers
- Concepts-ready

## Common Patterns

### Process Management

```cpp
processManager->startProcess(id, cmd, args, path);
processManager->onStandardOutput(id, [](const QString& out) { });
processManager->onStandardError(id, [](const QString& err) { });
processManager->stopProcess(id);
```

### Project Operations

```cpp
auto project = jsonParser->createProjectFromPackageJson(path);
project->pinScript("dev");
auto scripts = project->getScripts();
projectRepo->saveProject(*project);
```

### Workspace Management

```cpp
workspace->addProject(project);
workspace->startAll("dev");
workspace->stopAll();
workspaceRepo->saveWorkspace(*workspace);
```

### Settings

```cpp
settings->setValue("key", value);
auto val = settings->getValue("key", default);
settings->sync();
```

### Platform Effects

```cpp
if (platformEffect->isEffectSupported(EffectType::Mica)) {
    platformEffect->applyEffect(windowId, EffectType::Mica);
}
```

### System Tray

```cpp
systemTray->setIcon(icon);
systemTray->setState(TrayIconState::Active);
systemTray->onActivated([]() { /* clicked */ });
systemTray->show();
```

### Notifications

```cpp
auto id = notifications->showNotification(title, message);
notifications->onNotificationClicked([](QString id) { });
```

### QML Integration

```cpp
// C++
engine.rootContext()->setContextProperty("model", viewModel.get());

// QML
Button {
    text: "Run"
    onClicked: model.runScript("dev")
}
```

## Testing

### Mock Example

```cpp
class MockProcessManager : public IProcessManager {
    MOCK_METHOD(bool, startProcess, (...), (override));
    // ...
};

TEST(Test, Case) {
    MockProcessManager mock;
    EXPECT_CALL(mock, startProcess(...)).WillOnce(Return(true));
}
```

## Documentation

- **ARCHITECTURE.md**: Full architecture design
- **include/INTERFACES.md**: Interface guide
- **IMPLEMENTATION_SUMMARY.md**: What was implemented
- **USAGE_EXAMPLES.md**: 10 detailed examples
- **This file**: Quick reference

## Interface Categories

### Process & Execution
- `IProcessManager`: Run external processes
- `ILogBuffer`: Store process output

### Data Models
- `IProject`: Project with scripts
- `IWorkspace`: Collection of projects
- `IJsonParser`: Parse configurations

### Persistence
- `ISettingsManager`: App settings
- `IProjectRepository`: Project storage
- `IWorkspaceRepository`: Workspace storage

### Platform Integration
- `IPlatformEffect`: Native effects
- `ISystemTray`: System tray
- `INativeNotifications`: Notifications
- `IPlatformUtils`: Platform info

### UI/QML
- `ILogViewModel`: Log display
- `IProjectViewModel`: Project UI
- `IWorkspaceViewModel`: Workspace UI

## Best Practices

1. ✅ Always use factory to create objects
2. ✅ Check for nullptr when loading
3. ✅ Use const references for parameters
4. ✅ Register callbacks before starting
5. ✅ Save state frequently
6. ✅ Use smart pointers (no raw new/delete)
7. ✅ Respect layer boundaries
8. ✅ Mock interfaces for testing

## Next Steps

To implement ZenRunner:

1. **Implement Core Layer** (ProcessManager, Project, etc.)
2. **Implement Platform Layer** (per-OS effects)
3. **Implement Storage Layer** (QSettings, JSON)
4. **Implement UI Layer** (QML view models)
5. **Implement Factory** (object creation)
6. **Add Tests** (unit & integration)
7. **Build & Run** (CMake configuration ready)

## File Count

- Interface files: 17
- Documentation files: 5
- Lines of interface code: ~1500
- Lines of documentation: ~3000

## Technologies

- C++20 (smart pointers, concepts)
- Qt 6 (Core, Quick, Widgets)
- CMake 3.21+
- Platform APIs (Windows/macOS/Linux)

## Status

✅ **Complete**: All interfaces defined and documented
⏳ **Next**: Implement concrete classes

All interfaces are production-ready and can be implemented independently.
