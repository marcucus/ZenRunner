# ZenRunner Architecture: Modular Design with Strict API Interfaces

## Overview

ZenRunner is designed with a fully modular, extensible architecture where all communication between layers occurs through strict, well-defined interfaces. This design ensures:

- **Loose Coupling**: Each layer can be developed, tested, and modified independently
- **High Cohesion**: Related functionality is grouped together within layers
- **Testability**: Interfaces can be easily mocked for unit testing
- **Extensibility**: New implementations can be added without modifying existing code
- **Type Safety**: C++20 features ensure compile-time safety

## Architectural Layers

The system is divided into four primary layers, each with its own responsibilities:

### 1. Core Layer (`include/core/`, `src/core/`)

**Responsibilities:**
- Business logic and data management
- Process lifecycle management
- Project and workspace data models
- JSON parsing and configuration management
- Log buffering and management

**Interfaces:**
- `IProcessManager`: Process execution and monitoring
- `IProject`: Project data model and operations
- `IWorkspace`: Workspace management for grouped projects
- `IJsonParser`: Configuration file parsing
- `ILogBuffer`: Efficient circular buffer for log storage

**Dependencies:** None (bottom layer)

### 2. Platform Layer (`include/platform/`, `src/platform/`)

**Responsibilities:**
- OS-specific integrations
- Native visual effects (Mica, Vibrancy)
- System tray and notifications
- Platform utilities and detection

**Interfaces:**
- `IPlatformEffect`: OS-native visual effects
- `ISystemTray`: System tray/menu bar integration
- `INativeNotifications`: Native system notifications
- `IPlatformUtils`: Platform detection and utilities

**Dependencies:** Qt Core only (no Core layer dependencies)

### 3. Storage Layer (`include/storage/`, `src/storage/`)

**Responsibilities:**
- Persistent data storage
- Application settings management
- Project and workspace persistence
- State serialization and deserialization

**Interfaces:**
- `ISettingsManager`: Application settings (QSettings wrapper)
- `IProjectRepository`: Project data persistence
- `IWorkspaceRepository`: Workspace data persistence

**Dependencies:** Core layer interfaces (for data types)

### 4. UI Layer (`include/ui/`, `src/ui/`)

**Responsibilities:**
- User interface presentation
- QML view models
- User interaction handling
- Data formatting for display

**Interfaces:**
- `ILogViewModel`: Exposes log data to QML
- `IProjectViewModel`: Exposes project data to QML
- `IWorkspaceViewModel`: Exposes workspace data to QML

**Dependencies:** Core layer interfaces (via view models)

## Interface Design Principles

### 1. Pure Virtual Interfaces

All interfaces are abstract classes with pure virtual methods:

```cpp
class IProcessManager {
public:
    virtual ~IProcessManager() = default;
    virtual bool startProcess(...) = 0;
    // ...
};
```

### 2. Forward Declarations

Interfaces use forward declarations and pointers/references to minimize header dependencies:

```cpp
// Forward declaration
namespace Core { class IProject; }

// Usage in interface
virtual std::shared_ptr<Core::IProject> getProject() const = 0;
```

### 3. Smart Pointers

- `std::unique_ptr<T>`: Exclusive ownership (e.g., managers, repositories)
- `std::shared_ptr<T>`: Shared ownership (e.g., projects, workspaces)
- Raw pointers: Never used for ownership

### 4. Qt Integration

Interfaces that need Qt's signal/slot mechanism inherit from `QObject`:

```cpp
class ILogViewModel : public QAbstractListModel {
    Q_OBJECT
public:
    // ...
signals:
    void logsAdded();
};
```

### 5. Const Correctness

Read-only operations are marked `const`:

```cpp
virtual QString getName() const = 0;
virtual void setName(const QString& name) = 0;
```

## Dependency Flow

```
┌─────────────────────────────────────────────────────────┐
│                     UI Layer (QML)                      │
│  - ILogViewModel, IProjectViewModel, IWorkspaceViewModel│
└────────────────────────┬────────────────────────────────┘
                         │ uses
                         ↓
┌─────────────────────────────────────────────────────────┐
│                      Core Layer                         │
│  - IProcessManager, IProject, IWorkspace, IJsonParser   │
│  - ILogBuffer                                           │
└────────────────────────┬────────────────────────────────┘
                         │ used by
                         ↓
┌─────────────────────────────────────────────────────────┐
│                   Storage Layer                         │
│  - ISettingsManager, IProjectRepository                 │
│  - IWorkspaceRepository                                 │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                   Platform Layer                        │
│  - IPlatformEffect, ISystemTray                         │
│  - INativeNotifications, IPlatformUtils                 │
│  (Independent - used by Application layer)              │
└─────────────────────────────────────────────────────────┘
```

## Factory Pattern

The `IApplicationFactory` interface provides a central point for creating all layer objects:

```cpp
// Get factory instance
auto& factory = getApplicationFactory();

// Create objects through factory
auto processManager = factory.createProcessManager();
auto project = factory.createProject("/path/to/project");
auto platformEffect = factory.createPlatformEffect();
```

**Benefits:**
- Centralized object creation
- Easy dependency injection
- Simplified testing (mock factory)
- Runtime platform selection

## Communication Between Layers

### Core → Storage
Storage implementations depend on Core interfaces to persist data:

```cpp
class IProjectRepository {
    virtual bool saveProject(const Core::IProject& project) = 0;
};
```

### Core → UI
UI view models wrap Core interfaces for QML consumption:

```cpp
class IProjectViewModel : public QObject {
    virtual void setProject(std::shared_ptr<Core::IProject> project) = 0;
};
```

### Platform → Application
Platform services are used by the application layer but are independent:

```cpp
auto platformEffect = factory.createPlatformEffect();
platformEffect->applyEffect(windowId, EffectType::Mica);
```

### No Cross-Layer Dependencies
- UI layer never directly accesses Storage
- Platform layer never accesses Core
- All communication goes through defined interfaces

## Testing Strategy

### Unit Testing
Each interface can be independently tested with mock implementations:

```cpp
class MockProcessManager : public IProcessManager {
public:
    bool startProcess(...) override {
        // Mock implementation
        return true;
    }
};
```

### Integration Testing
Test interactions between layers through interfaces:

```cpp
// Create real implementations
auto project = factory.createProject("/test/path");
auto repository = factory.createProjectRepository();

// Test integration
repository->saveProject(*project);
auto loaded = repository->loadProject(project->getId());
```

### Layer Isolation
Each layer can be built and tested independently by providing mock implementations of dependency interfaces.

## C++20 Features Used

### Concepts
Used for template constraints in factory implementations:

```cpp
template<typename T>
concept IsInterface = std::is_abstract_v<T>;
```

### Modules (Future)
Interfaces are designed to work with C++20 modules:

```cpp
export module zenrunner.core;
export import :IProcessManager;
export import :IProject;
```

### Designated Initializers
Used in result structures:

```cpp
ParseResult result {
    .success = true,
    .errorMessage = "",
    .data = jsonObject
};
```

### [[nodiscard]]
Used for critical return values:

```cpp
[[nodiscard]] bool isSuccess() const { return success; }
```

## Implementation Guidelines

### Adding a New Interface

1. **Define the interface** in `include/<layer>/INewInterface.hpp`
2. **Document all methods** with Doxygen comments
3. **Add factory method** to `IApplicationFactory`
4. **Implement the interface** in `src/<layer>/NewImplementation.cpp`
5. **Update factory** to create the implementation
6. **Write unit tests** for the implementation

### Implementing an Interface

1. **Inherit from the interface:**
   ```cpp
   class ProcessManager : public IProcessManager {
   ```

2. **Override all pure virtual methods:**
   ```cpp
   bool startProcess(...) override;
   ```

3. **Maintain const correctness:**
   ```cpp
   ProcessState getProcessState(const QString& id) const override;
   ```

4. **Use smart pointers appropriately:**
   ```cpp
   std::shared_ptr<IProject> project_;
   ```

### Modifying an Interface

1. **Add new methods at the end** (preserves ABI compatibility)
2. **Provide default implementations** when possible:
   ```cpp
   virtual void newMethod() { /* default */ }
   ```
3. **Update all implementations**
4. **Update documentation**
5. **Version the interface** if breaking changes are needed

## Benefits of This Architecture

1. **Modularity**: Each layer is a self-contained module
2. **Testability**: Easy to mock interfaces for testing
3. **Flexibility**: Implementations can be swapped at runtime
4. **Maintainability**: Clear boundaries reduce complexity
5. **Scalability**: New features can be added without affecting existing code
6. **Performance**: Interface calls compile to vtable lookups (minimal overhead)
7. **Type Safety**: Compile-time checking of interface contracts

## Future Enhancements

### Plugin System
The interface-based design supports dynamic plugin loading:

```cpp
class IPlugin {
    virtual void initialize(IApplicationFactory& factory) = 0;
};
```

### Remote Interfaces
Interfaces can be exposed over IPC/network for distributed architecture:

```cpp
// Server side
auto processManager = factory.createProcessManager();
rpcServer.expose(processManager);

// Client side
auto remoteProcessManager = rpcClient.connect<IProcessManager>("localhost");
```

### Scripting Integration
Interfaces can be easily exposed to scripting languages (Python, Lua):

```cpp
py::class_<IProject>(m, "IProject")
    .def("getName", &IProject::getName)
    .def("getScripts", &IProject::getScripts);
```

## Summary

This modular architecture with strict API interfaces provides:

- Clear separation of concerns
- Loose coupling between layers
- High testability
- Easy extensibility
- Type-safe interfaces
- Performance-conscious design

All communication happens through well-defined interfaces, ensuring that each layer can be developed, tested, and maintained independently while working together as a cohesive system.
