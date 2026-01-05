# ZenRunner Interface Usage Examples

This document provides practical examples of how to use the ZenRunner interfaces.

## Basic Setup

```cpp
#include "IApplicationFactory.hpp"

using namespace ZenRunner;

// Get the factory instance
auto& factory = getApplicationFactory();
```

## Example 1: Managing a Process

```cpp
#include "core/IProcessManager.hpp"

// Create a process manager
auto processManager = factory.createProcessManager();

// Start a process
bool started = processManager->startProcess(
    "dev-server",                    // Process ID
    "npm",                           // Command
    QStringList{"run", "dev"},       // Arguments
    "/path/to/project"               // Working directory
);

if (started) {
    // Register callbacks for output
    processManager->onStandardOutput("dev-server", 
        [](const QString& output) {
            qDebug() << "Output:" << output;
        });
    
    processManager->onStandardError("dev-server",
        [](const QString& error) {
            qWarning() << "Error:" << error;
        });
    
    processManager->onStateChanged("dev-server",
        [](Core::ProcessState state) {
            if (state == Core::ProcessState::Crashed) {
                qWarning() << "Process crashed!";
            }
        });
}

// Check if running
if (processManager->isProcessRunning("dev-server")) {
    qDebug() << "Process is running";
}

// Stop the process (graceful)
processManager->stopProcess("dev-server", false);

// Or force kill
processManager->stopProcess("dev-server", true);
```

## Example 2: Working with Projects

```cpp
#include "core/IProject.hpp"
#include "core/IJsonParser.hpp"

// Parse a package.json
auto jsonParser = factory.createJsonParser();
auto project = jsonParser->createProjectFromPackageJson(
    "/path/to/project/package.json"
);

if (project) {
    qDebug() << "Project name:" << project->getName();
    qDebug() << "Project path:" << project->getPath();
    
    // Get all scripts
    auto scripts = project->getScripts();
    for (const auto& [name, script] : scripts.asKeyValueRange()) {
        qDebug() << "Script:" << name << "->" << script.command;
    }
    
    // Pin a script for quick access
    project->pinScript("dev");
    project->pinScript("test");
    
    // Get pinned scripts
    auto pinned = project->getPinnedScripts();
    for (const QString& scriptName : pinned) {
        qDebug() << "Pinned:" << scriptName;
    }
    
    // Get a specific script
    const auto* devScript = project->getScript("dev");
    if (devScript) {
        qDebug() << "Dev command:" << devScript->command;
    }
}
```

## Example 3: Managing Workspaces

```cpp
#include "core/IWorkspace.hpp"

// Create a workspace
auto workspace = factory.createWorkspace(
    "Full Stack Development",
    "Frontend, Backend, and Database"
);

// Create some projects
auto frontend = jsonParser->createProjectFromPackageJson(
    "/projects/frontend/package.json"
);
auto backend = jsonParser->createProjectFromPackageJson(
    "/projects/backend/package.json"
);

// Add projects to workspace
workspace->addProject(frontend);
workspace->addProject(backend);

qDebug() << "Workspace has" << workspace->getProjectCount() << "projects";

// Start all projects with 'dev' script
int started = workspace->startAll("dev");
qDebug() << "Started" << started << "projects";

// Check if any project is running
if (workspace->isAnyProjectRunning()) {
    qDebug() << "Some projects are running";
}

// Stop all projects
workspace->stopAll(false);  // Graceful shutdown
```

## Example 4: Persistent Storage

```cpp
#include "storage/IProjectRepository.hpp"
#include "storage/IWorkspaceRepository.hpp"

// Save a project
auto projectRepo = factory.createProjectRepository();
projectRepo->saveProject(*project);

// Load a project by ID
auto loadedProject = projectRepo->loadProject(project->getId());

// Load a project by path
auto projectByPath = projectRepo->loadProjectFromPath(
    "/path/to/project"
);

// Get all stored projects
auto allProjects = projectRepo->getAllProjects();
for (const auto& proj : allProjects) {
    qDebug() << "Stored project:" << proj->getName();
}

// Save a workspace
auto workspaceRepo = factory.createWorkspaceRepository();
workspaceRepo->saveWorkspace(*workspace);

// Load a workspace
auto loadedWorkspace = workspaceRepo->loadWorkspace(workspace->getId());
```

## Example 5: Application Settings

```cpp
#include "storage/ISettingsManager.hpp"

auto settings = factory.createSettingsManager();

// Store settings
settings->setValue("ui/theme", "dark");
settings->setValue("ui/fontSize", 12);
settings->setBool("notifications/enabled", true);
settings->setStringList("recent/projects", 
    QStringList{"/path1", "/path2", "/path3"}
);

// Retrieve settings
QString theme = settings->getString("ui/theme", "light");
int fontSize = settings->getInt("ui/fontSize", 10);
bool notificationsEnabled = settings->getBool("notifications/enabled", true);
auto recentProjects = settings->getStringList("recent/projects");

// Check if a setting exists
if (settings->contains("ui/theme")) {
    qDebug() << "Theme is configured";
}

// Remove a setting
settings->remove("ui/theme");

// Force sync to disk
settings->sync();
```

## Example 6: Platform Integration

```cpp
#include "platform/IPlatformEffect.hpp"
#include "platform/ISystemTray.hpp"
#include "platform/INativeNotifications.hpp"

// Apply platform effect to a window
auto platformEffect = factory.createPlatformEffect();
auto window = /* get QWindow or QWidget */;
quintptr windowId = reinterpret_cast<quintptr>(window->winId());

// Check what's supported
if (platformEffect->isEffectSupported(Platform::EffectType::Mica)) {
    platformEffect->applyEffect(
        windowId,
        Platform::EffectType::Mica,
        Platform::EffectTheme::Auto
    );
}

// System tray
auto systemTray = factory.createSystemTray();
if (systemTray->isSystemTrayAvailable()) {
    systemTray->setIcon(QIcon(":/icons/app.png"));
    systemTray->setTooltip("ZenRunner - 3 processes running");
    systemTray->setState(Platform::TrayIconState::Active);
    
    systemTray->onActivated([]() {
        qDebug() << "Tray icon clicked";
    });
    
    systemTray->show();
}

// Notifications
auto notifications = factory.createNativeNotifications();
if (notifications->isSupported()) {
    QString notifId = notifications->showNotification(
        "Process Crashed",
        "The dev-server process has stopped unexpectedly",
        Platform::NotificationPriority::High
    );
    
    // With actions
    notifId = notifications->showNotificationWithActions(
        "Build Complete",
        "Your project has been built successfully",
        {
            {"open", "Open Project"},
            {"dismiss", "Dismiss"}
        }
    );
    
    notifications->onActionClicked(
        [](const QString& notifId, const QString& actionId) {
            if (actionId == "open") {
                // Open project
            }
        }
    );
}
```

## Example 7: Log Buffer and Viewing

```cpp
#include "core/ILogBuffer.hpp"

// Create a log buffer
auto logBuffer = factory.createLogBuffer(5000);

// Append log entries
logBuffer->append("Server started on port 3000", false);
logBuffer->append("Error: Connection refused", true);

// Append with full entry
Core::LogEntry entry {
    .text = "Warning: Deprecated API used",
    .timestamp = QDateTime::currentMSecsSinceEpoch(),
    .isError = false
};
logBuffer->append(entry);

// Query logs
auto allLogs = logBuffer->getAll();
auto recent10 = logBuffer->getRecent(10);

// Search logs
auto errorLogs = logBuffer->search("error");
for (const auto& log : errorLogs) {
    qDebug() << "Found error:" << log.text;
}

// Time range query
qint64 startTime = /* some timestamp */;
qint64 endTime = /* another timestamp */;
auto logsInRange = logBuffer->getRange(startTime, endTime);

// Check buffer state
qDebug() << "Buffer size:" << logBuffer->size();
qDebug() << "Buffer capacity:" << logBuffer->capacity();
qDebug() << "Buffer is full:" << logBuffer->isFull();
```

## Example 8: QML Integration with View Models

### C++ Side

```cpp
#include "ui/IProjectViewModel.hpp"
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Create view models
auto projectVM = factory.createProjectViewModel(project);
auto logBuffer = factory.createLogBuffer();
auto logVM = factory.createLogViewModel(logBuffer);

// Connect to process manager
processManager->onStandardOutput("dev-server",
    [&logBuffer](const QString& output) {
        logBuffer->append(output, false);
    });

processManager->onStandardError("dev-server",
    [&logBuffer](const QString& error) {
        logBuffer->append(error, true);
    });

// Expose to QML
QQmlApplicationEngine engine;
engine.rootContext()->setContextProperty("projectModel", projectVM.get());
engine.rootContext()->setContextProperty("logModel", logVM.get());
engine.load(QUrl("qrc:/main.qml"));
```

### QML Side

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    
    Column {
        spacing: 10
        
        // Display project info
        Text {
            text: "Project: " + projectModel.getProjectName()
        }
        
        // Script buttons
        Row {
            spacing: 5
            
            Repeater {
                model: projectModel.getPinnedScripts()
                
                Button {
                    text: modelData
                    onClicked: projectModel.runScript(modelData)
                }
            }
        }
        
        // Log view
        ListView {
            width: parent.width
            height: 400
            model: logModel
            
            delegate: Text {
                text: model.display
                color: model.isError ? "red" : "white"
            }
        }
        
        // Search logs
        TextField {
            placeholderText: "Search logs..."
            onTextChanged: logModel.setFilter(text)
        }
    }
}
```

## Example 9: Complete Application Workflow

```cpp
#include "IApplicationFactory.hpp"

class ZenRunnerApplication {
    ZenRunner::IApplicationFactory& factory_;
    std::unique_ptr<Core::IProcessManager> processManager_;
    std::unique_ptr<Storage::IProjectRepository> projectRepo_;
    std::unique_ptr<Storage::IWorkspaceRepository> workspaceRepo_;
    std::unique_ptr<Storage::ISettingsManager> settings_;
    
public:
    ZenRunnerApplication() 
        : factory_(ZenRunner::getApplicationFactory())
    {
        // Initialize all services
        processManager_ = factory_.createProcessManager();
        projectRepo_ = factory_.createProjectRepository();
        workspaceRepo_ = factory_.createWorkspaceRepository();
        settings_ = factory_.createSettingsManager();
    }
    
    void importProject(const QString& path) {
        auto jsonParser = factory_.createJsonParser();
        auto project = jsonParser->createProjectFromPackageJson(
            path + "/package.json"
        );
        
        if (project && project->isValid()) {
            projectRepo_->saveProject(*project);
            
            // Add to recent projects
            auto recent = settings_->getStringList("recent/projects");
            recent.prepend(path);
            if (recent.size() > 10) {
                recent.removeLast();
            }
            settings_->setStringList("recent/projects", recent);
            settings_->sync();
        }
    }
    
    void runProjectScript(const QString& projectId, const QString& scriptName) {
        auto project = projectRepo_->loadProject(projectId);
        if (!project) return;
        
        const auto* script = project->getScript(scriptName);
        if (!script) return;
        
        // Parse command and run
        QString processId = projectId + "_" + scriptName;
        processManager_->startProcess(
            processId,
            script->command.split(" ").first(),
            script->command.split(" ").mid(1),
            project->getPath()
        );
    }
    
    void loadWorkspace(const QString& workspaceId) {
        auto workspace = workspaceRepo_->loadWorkspace(workspaceId);
        if (!workspace) return;
        
        // Load all projects in workspace
        for (const auto& project : workspace->getProjects()) {
            qDebug() << "Workspace project:" << project->getName();
        }
    }
};
```

## Example 10: Testing with Mock Interfaces

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/IProcessManager.hpp"

class MockProcessManager : public Core::IProcessManager {
public:
    MOCK_METHOD(bool, startProcess, 
        (const QString&, const QString&, const QStringList&, const QString&),
        (override));
    MOCK_METHOD(bool, stopProcess, (const QString&, bool), (override));
    MOCK_METHOD(Core::ProcessState, getProcessState, (const QString&),
        (const, override));
    MOCK_METHOD(bool, isProcessRunning, (const QString&), (const, override));
    MOCK_METHOD(int, getExitCode, (const QString&), (const, override));
    MOCK_METHOD(void, onStandardOutput,
        (const QString&, std::function<void(const QString&)>), (override));
    MOCK_METHOD(void, onStandardError,
        (const QString&, std::function<void(const QString&)>), (override));
    MOCK_METHOD(void, onStateChanged,
        (const QString&, std::function<void(Core::ProcessState)>), (override));
    MOCK_METHOD(void, stopAllProcesses, (bool), (override));
};

TEST(ProcessManagerTest, StartProcess) {
    MockProcessManager mockPM;
    
    EXPECT_CALL(mockPM, startProcess(
        testing::Eq("test"),
        testing::Eq("echo"),
        testing::ElementsAre("hello"),
        testing::Eq("/tmp")
    )).WillOnce(testing::Return(true));
    
    bool result = mockPM.startProcess("test", "echo", {"hello"}, "/tmp");
    EXPECT_TRUE(result);
}

TEST(ProcessManagerTest, ProcessState) {
    MockProcessManager mockPM;
    
    EXPECT_CALL(mockPM, getProcessState("test"))
        .WillOnce(testing::Return(Core::ProcessState::Running));
    
    auto state = mockPM.getProcessState("test");
    EXPECT_EQ(state, Core::ProcessState::Running);
}
```

## Key Takeaways

1. **Factory Pattern**: Always use the factory to create objects
2. **Smart Pointers**: Use unique_ptr for exclusive ownership, shared_ptr for shared
3. **Callbacks**: Use lambdas or std::function for async callbacks
4. **Qt Integration**: Interfaces work seamlessly with Qt signals/slots
5. **QML Integration**: View models expose data to QML via Q_INVOKABLE
6. **Testing**: Easy to mock interfaces for unit testing
7. **Layering**: Respect layer boundaries - don't skip layers

## Best Practices

- **Always check for nullptr** when loading objects from repositories
- **Use const references** when passing parameters to avoid copies
- **Register callbacks early** before starting processes
- **Save state frequently** to prevent data loss
- **Use smart pointers** to prevent memory leaks
- **Follow Qt conventions** for signal/slot naming
- **Test with mocks** before integrating real implementations
