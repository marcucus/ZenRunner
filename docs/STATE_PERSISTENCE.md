# State Persistence System

## Overview

ZenRunner implements a comprehensive state persistence system that ensures user preferences, workspace configurations, and project data are safely stored and restored across application sessions.

## Architecture

The state persistence system is organized into three main layers:

### 1. Settings Manager (`ISettingsManager`)
- **Purpose**: Stores global application preferences
- **Implementation**: Uses Qt's `QSettings` for platform-native storage
- **Storage Location**: Platform-specific (Registry on Windows, plist on macOS, config files on Linux)
- **Data Stored**:
  - Window geometry (position and size)
  - Auto-save preferences
  - Recent projects list
  - Last active workspace
  - First run flag

### 2. Workspace Repository (`IWorkspaceRepository`)
- **Purpose**: Persists workspace configurations
- **Implementation**: JSON files in application data directory
- **Storage Location**: `<AppData>/ZenRunner/workspaces/`
- **Features**:
  - Each workspace stored as separate JSON file
  - Atomic writes using `QSaveFile`
  - Automatic validation on load
  - Recovery from corrupted data

### 3. Project Repository (`ProjectRepository`)
- **Purpose**: Stores project metadata and state
- **Implementation**: JSON files in application data directory
- **Storage Location**: `<AppData>/ZenRunner/projects/`
- **Features**:
  - Project-specific metadata (name, path, pinned scripts)
  - Path-based lookup index for fast retrieval
  - Atomic writes to prevent corruption
  - Validation and recovery mechanisms

### 4. Application State Manager (`IApplicationStateManager`)
- **Purpose**: High-level orchestration of all persistence components
- **Features**:
  - Coordinates state save/restore operations
  - Periodic auto-save with configurable intervals
  - State validation and recovery
  - Modular design for easy testing

## Key Features

### Atomic Writes
All storage operations use atomic writes to prevent data corruption:
- `QSettings` handles atomic writes internally
- JSON files use `QSaveFile` which writes to temporary file first, then atomically renames

### Data Validation
Each component validates data on load:
- JSON schema validation
- Required field checks
- Corrupted data is automatically removed
- Graceful degradation when data is incomplete

### Auto-Save
Periodic state persistence prevents data loss:
- Configurable interval (default: 60 seconds)
- Triggered by timer in `ApplicationStateManager`
- Can be disabled if needed
- Runs on background thread to avoid UI blocking

### Recovery Mechanisms
Robust error handling ensures application stability:
- Corrupted JSON files are deleted and logged
- Missing directories are automatically created
- Invalid projects are skipped during load
- State validation runs on startup

## Usage

### Basic Integration

```cpp
#include "storage/SettingsManager.h"
#include "storage/WorkspaceRepository.h"
#include "storage/ProjectRepository.h"
#include "storage/ApplicationStateManager.h"

// Create components
auto settings = Storage::createSettingsManager();
auto workspaceRepo = std::make_shared<WorkspaceRepository>();
auto projectRepo = std::make_shared<ProjectRepository>();

// Create state manager
auto stateManager = Storage::createApplicationStateManager(
    settings, workspaceRepo, projectRepo);

// Initialize and restore state
stateManager->initialize();

// Enable auto-save every 60 seconds
stateManager->setAutoSaveEnabled(true, 60);

// Use the state manager
stateManager->setLastWorkspaceId("workspace-123");
stateManager->addRecentProject("project-456");
stateManager->setWindowGeometry(100, 100, 1200, 800);
```

### Saving Project State

```cpp
// Create or load a project
auto projectResult = Project::fromDirectory("/path/to/project");
if (projectResult.isOk()) {
    auto project = projectResult.unwrap();
    
    // Modify project state
    project.setScriptPinned("start", true);
    project.setScriptPinned("build", true);
    
    // Save to repository
    projectRepo->saveProject(project);
}
```

### Loading Workspace

```cpp
// Load workspace from repository
auto workspace = workspaceRepo->loadWorkspace("workspace-id");
if (workspace) {
    qDebug() << "Loaded workspace:" << workspace->getName();
    qDebug() << "Projects:" << workspace->getProjects().size();
}
```

## Storage Locations

### Windows
- Settings: `HKEY_CURRENT_USER\Software\ZenRunner`
- Workspaces: `%APPDATA%\ZenRunner\workspaces\`
- Projects: `%APPDATA%\ZenRunner\projects\`

### macOS
- Settings: `~/Library/Preferences/com.zenrunner.ZenRunner.plist`
- Workspaces: `~/Library/Application Support/ZenRunner/workspaces/`
- Projects: `~/Library/Application Support/ZenRunner/projects/`

### Linux
- Settings: `~/.config/ZenRunner/ZenRunner.conf`
- Workspaces: `~/.local/share/ZenRunner/workspaces/`
- Projects: `~/.local/share/ZenRunner/projects/`

## JSON Schema

### Project JSON Format
```json
{
  "id": "unique-project-id",
  "name": "My Project",
  "path": "/path/to/project",
  "version": 1,
  "scripts": [
    {
      "name": "start",
      "command": "npm start",
      "isPinned": true
    },
    {
      "name": "build",
      "command": "npm run build",
      "isPinned": false
    }
  ],
  "pinnedScripts": ["start"]
}
```

### Workspace JSON Format
```json
{
  "id": "unique-workspace-id",
  "name": "Development Workspace",
  "description": "Main development environment",
  "version": 1,
  "projects": [
    {
      "id": "project-1-id",
      "name": "Frontend",
      "path": "/path/to/frontend",
      "pinnedScripts": ["start", "test"]
    },
    {
      "id": "project-2-id",
      "name": "Backend",
      "path": "/path/to/backend",
      "pinnedScripts": ["dev"]
    }
  ]
}
```

## Error Handling

The persistence system handles various error scenarios:

1. **Missing Directories**: Automatically created on first use
2. **Corrupted JSON**: Deleted and logged, application continues
3. **Invalid Project Paths**: Project skipped, workspace loads without it
4. **Permission Errors**: Logged with clear error messages
5. **Disk Full**: Atomic writes ensure no partial data written

## Testing

Test suites are provided for all components:
- `test_project_repository`: Tests project persistence and recovery
- `test_application_state`: Tests high-level state management

Run tests:
```bash
./bin/test_project_repository
./bin/test_application_state
```

## Performance Considerations

- **Lazy Loading**: Data loaded only when needed
- **Caching**: Path-to-ID mapping cached for fast lookups
- **Atomic Operations**: No locks needed, file system handles atomicity
- **Background Saves**: Auto-save runs asynchronously
- **Minimal Memory**: No in-memory duplication of persisted data

## Future Enhancements

Potential improvements for future versions:
- [ ] Encrypted storage for sensitive data
- [ ] Cloud sync support
- [ ] Version migration for schema upgrades
- [ ] Compression for large workspace files
- [ ] Backup and restore functionality
- [ ] Export/import for sharing configurations
