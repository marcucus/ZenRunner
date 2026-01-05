# ZenRunner Interface Documentation

## Overview

This directory contains all public API interfaces for the ZenRunner application. These interfaces define strict contracts between different layers of the application, ensuring modularity, testability, and maintainability.

## Directory Structure

```
include/
├── core/                    # Core business logic interfaces
│   ├── IProcessManager.hpp  # Process lifecycle management
│   ├── IProject.hpp         # Project data model
│   ├── IWorkspace.hpp       # Workspace management
│   ├── IJsonParser.hpp      # JSON configuration parsing
│   └── ILogBuffer.hpp       # Circular buffer for logs
├── platform/                # Platform-specific interfaces
│   ├── IPlatformEffect.hpp  # OS-native visual effects
│   ├── ISystemTray.hpp      # System tray integration
│   ├── INativeNotifications.hpp  # Native notifications
│   └── IPlatformUtils.hpp   # Platform utilities
├── storage/                 # Data persistence interfaces
│   ├── ISettingsManager.hpp      # Application settings
│   ├── IProjectRepository.hpp    # Project persistence
│   └── IWorkspaceRepository.hpp  # Workspace persistence
├── ui/                      # User interface interfaces
│   ├── ILogViewModel.hpp         # Log display view model
│   ├── IProjectViewModel.hpp     # Project view model
│   └── IWorkspaceViewModel.hpp   # Workspace view model
├── types/                   # Common type definitions
│   └── CommonTypes.hpp      # Shared types and enums
└── IApplicationFactory.hpp  # Factory for creating implementations
```

## Layer Architecture

### Core Layer
The foundation layer containing business logic and data management.

**Interfaces:**
- **IProcessManager**: Manages external process execution using QProcess
- **IProject**: Represents a project with its scripts and metadata
- **IWorkspace**: Groups multiple related projects together
- **IJsonParser**: Parses package.json and configuration files
- **ILogBuffer**: Circular buffer for efficient log storage (5000 lines default)

**Dependencies:** None (bottom layer)

### Platform Layer
Provides OS-specific functionality through abstract interfaces.

**Interfaces:**
- **IPlatformEffect**: Applies native visual effects (Mica on Windows, Vibrancy on macOS)
- **ISystemTray**: System tray/menu bar integration
- **INativeNotifications**: Native system notifications
- **IPlatformUtils**: Platform detection, screen info, notch detection

**Dependencies:** Qt Core only

### Storage Layer
Handles persistent data storage and retrieval.

**Interfaces:**
- **ISettingsManager**: Manages application settings (wrapper around QSettings)
- **IProjectRepository**: Persists project data and metadata
- **IWorkspaceRepository**: Persists workspace configurations

**Dependencies:** Core layer (for data types)

### UI Layer
Provides view models that expose data to QML.

**Interfaces:**
- **ILogViewModel**: Exposes log buffer to QML ListView
- **IProjectViewModel**: Exposes project data and operations to QML
- **IWorkspaceViewModel**: Exposes workspace data and operations to QML

**Dependencies:** Core layer (via view models)

## Usage

### Creating Objects

Use the `IApplicationFactory` to create instances:

```cpp
#include "IApplicationFactory.hpp"

// Get the factory instance
auto& factory = ZenRunner::getApplicationFactory();

// Create core objects
auto processManager = factory.createProcessManager();
auto project = factory.createProject("/path/to/project");
auto workspace = factory.createWorkspace("My Workspace");

// Create platform objects
auto platformEffect = factory.createPlatformEffect();
auto systemTray = factory.createSystemTray();

// Create storage objects
auto settings = factory.createSettingsManager();
auto projectRepo = factory.createProjectRepository();

// Create UI objects
auto logBuffer = factory.createLogBuffer(5000);
auto logViewModel = factory.createLogViewModel(logBuffer);
```

### Implementing an Interface

```cpp
#include "core/IProcessManager.hpp"

class ProcessManager : public Core::IProcessManager {
public:
    bool startProcess(
        const QString& processId,
        const QString& command,
        const QStringList& arguments,
        const QString& workingDirectory
    ) override {
        // Implementation here
        return true;
    }
    
    // Implement other interface methods...
};
```

### Testing with Mocks

```cpp
class MockProcessManager : public Core::IProcessManager {
public:
    MOCK_METHOD(bool, startProcess, 
        (const QString&, const QString&, const QStringList&, const QString&),
        (override));
    MOCK_METHOD(bool, stopProcess, (const QString&, bool), (override));
    // Mock other methods...
};

// Use in tests
MockProcessManager mockPM;
EXPECT_CALL(mockPM, startProcess(_, _, _, _))
    .WillOnce(Return(true));
```

## Design Principles

### 1. Interface Segregation
Each interface has a single, well-defined responsibility.

### 2. Dependency Inversion
High-level modules depend on abstractions (interfaces), not concrete implementations.

### 3. Open/Closed Principle
Interfaces are open for extension (new implementations) but closed for modification.

### 4. Liskov Substitution
Any implementation of an interface can be substituted without affecting correctness.

### 5. Don't Repeat Yourself (DRY)
Common functionality is abstracted into shared interfaces.

## C++20 Features

### Smart Pointers
- `std::unique_ptr<T>`: Exclusive ownership (managers, repositories)
- `std::shared_ptr<T>`: Shared ownership (projects, workspaces)

### [[nodiscard]]
Critical return values are marked with `[[nodiscard]]` to prevent accidental ignoring.

### Concepts (future)
Template constraints for type safety:
```cpp
template<typename T>
concept IsInterface = std::is_abstract_v<T>;
```

### Designated Initializers
Used in result structures:
```cpp
Result<void> result {
    .success = true,
    .errorMessage = ""
};
```

## Qt Integration

### QObject-based Interfaces
Interfaces that need signals/slots inherit from QObject:
```cpp
class ILogViewModel : public QAbstractListModel {
    Q_OBJECT
signals:
    void logsAdded();
};
```

### Q_INVOKABLE Methods
Methods that should be callable from QML are marked `Q_INVOKABLE`:
```cpp
Q_INVOKABLE virtual QString getProjectName() const = 0;
```

## Documentation Standards

All interfaces use Doxygen-style documentation:

```cpp
/**
 * @brief Brief description of the interface
 * 
 * Detailed description of what this interface does,
 * its responsibilities, and usage guidelines.
 */
class IExample {
public:
    /**
     * @brief Brief description of the method
     * @param param1 Description of first parameter
     * @param param2 Description of second parameter
     * @return Description of return value
     */
    virtual bool method(int param1, const QString& param2) = 0;
};
```

## Performance Considerations

### Memory Management
- Use smart pointers to prevent memory leaks
- Circular buffer in `ILogBuffer` prevents unbounded memory growth
- View models provide efficient data access to QML

### Minimal Overhead
- Virtual function calls have minimal overhead (vtable lookup)
- Interfaces are compile-time contracts with runtime efficiency

### Asynchronous Operations
- `IProcessManager` uses Qt's signal/slot mechanism for async process handling
- View models update QML reactively without polling

## Testing Guidelines

### Unit Testing
Test each implementation in isolation:
```cpp
TEST(ProcessManagerTest, StartProcess) {
    auto pm = factory.createProcessManager();
    EXPECT_TRUE(pm->startProcess("test", "echo", {"hello"}, "/tmp"));
}
```

### Integration Testing
Test interactions between layers:
```cpp
TEST(ProjectWorkflow, SaveAndLoad) {
    auto project = factory.createProject("/test");
    auto repo = factory.createProjectRepository();
    
    repo->saveProject(*project);
    auto loaded = repo->loadProject(project->getId());
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getName(), project->getName());
}
```

### Mock Testing
Use mock implementations for dependencies:
```cpp
class MockProjectRepository : public Storage::IProjectRepository {
    // Mock implementation
};
```

## Future Enhancements

### Plugin System
Interfaces support dynamic plugin loading:
```cpp
class IPlugin {
    virtual void initialize(IApplicationFactory& factory) = 0;
};
```

### Remote Interfaces
Expose interfaces over IPC/network for distributed architecture.

### Scripting Integration
Expose interfaces to Python/Lua for automation.

## Contributing

When adding new interfaces:

1. **Define the interface** in the appropriate `include/<layer>/` directory
2. **Document all methods** with Doxygen comments
3. **Add factory method** to `IApplicationFactory`
4. **Implement the interface** in `src/<layer>/`
5. **Update this documentation**
6. **Write unit tests**

## References

- **SOLID Principles**: Applied throughout interface design
- **Qt Best Practices**: Proper use of QObject, signals, and slots
- **C++20 Standard**: Modern C++ features for safety and performance
- **Dependency Injection**: Factory pattern for loose coupling

## License

See the project's main LICENSE file for licensing information.
