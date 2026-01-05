# Workspace System - User Guide

## Overview

The Workspace system in ZenRunner allows you to group related projects together and manage them as a unit. This is particularly useful for complex development scenarios such as:

- **Full-stack applications** (Frontend + Backend + Database)
- **Microservices architectures** (Multiple interdependent services)
- **Monorepos** (Multiple packages in a single repository)
- **Development environments** (API + UI + Tools)

## Key Features

### 1. Workspace Management

#### Creating a Workspace
1. Click the **"+ Create Workspace"** button in the workspace panel
2. Enter a name for your workspace (e.g., "Full Stack Development")
3. Optionally add a description
4. Click **OK** to create

#### Editing a Workspace
1. Click the **⚙️ Settings** button on any workspace card
2. Modify the name or description
3. Click **OK** to save changes

#### Deleting a Workspace
1. Click the **🗑️ Delete** button on the workspace card
2. Confirm the deletion in the dialog
3. **Note**: This only deletes the workspace configuration, not the projects themselves

### 2. Project Organization

#### Adding Projects to a Workspace
- Projects can be added to multiple workspaces
- Each workspace maintains its own list of projects
- Projects retain their individual settings

#### Removing Projects from a Workspace
- Use the project management interface to remove projects
- Removing a project from a workspace doesn't delete the project

### 3. Batch Process Management

The workspace system provides powerful batch operations to control all projects simultaneously.

#### Start All Projects (Parallel)
**Use Case**: Start all services at once for maximum speed

- Click **"▶ Start All"** button
- Or click **"⚡ Parallel"** button when hovering
- All project dev scripts start simultaneously
- Fastest startup time
- Best for independent services

**Example Scenario**:
```
Frontend  ━━━▶ Running
Backend   ━━━▶ Running  
Database  ━━━▶ Running
(All started at the same time)
```

#### Start All Projects (Sequential)
**Use Case**: Start services in order when there are dependencies

- Hover over workspace card to reveal execution mode buttons
- Click **"➡️ Sequential"** button
- Projects start one after another
- Each waits for the previous to stabilize
- Best for dependent services

**Example Scenario**:
```
Database  ━━━▶ Running ━━━▶ Backend ━━━▶ Running ━━━▶ Frontend ━━━▶ Running
(Database must start before Backend, Backend before Frontend)
```

#### Stop All Projects
- Click **"⏸ Stop All"** when workspace is running
- All running processes in the workspace are stopped gracefully
- Force kill option available if needed

### 4. Visual Status Indicators

#### Workspace Status Badge
- **Green "Active"**: At least one project is running
- **Gray "Idle"**: No projects are running

#### Project Count
- Shows the number of projects in the workspace
- Updated automatically when projects are added/removed

#### Color Coding
- Each workspace gets a unique color based on its name
- Colors remain consistent across sessions
- Helps quickly identify workspaces

## Best Practices

### 1. Workspace Organization

**Recommended Structure**:
```
📁 Workspaces
├── 🎨 Frontend Development
│   └── UI Projects
├── ⚙️ Backend Services  
│   └── API and microservices
├── 🗄️ Full Stack
│   └── Complete application stack
└── 🧪 Testing Environment
    └── Test runners and tools
```

### 2. Execution Mode Selection

**Use Parallel Mode When**:
- Services are independent
- You want fastest startup
- Services don't share resources
- Examples: Multiple frontend apps, isolated microservices

**Use Sequential Mode When**:
- Services have dependencies
- Database must start before API
- API must be ready before frontend
- Examples: Traditional full-stack apps, dependent microservices

### 3. Script Naming

Ensure consistent script names across projects for batch operations:
```json
{
  "scripts": {
    "dev": "Start development server",
    "start": "Start production server",
    "test": "Run tests"
  }
}
```

When you run **Start All** on a workspace, it runs the same script (e.g., "dev") on all projects.

## Advanced Features

### Workspace Persistence

Workspaces are automatically saved to:
- **Windows**: `%APPDATA%/ZenRunner/workspaces/`
- **macOS**: `~/Library/Application Support/ZenRunner/workspaces/`
- **Linux**: `~/.local/share/ZenRunner/workspaces/`

Each workspace is stored as a JSON file:
```json
{
  "id": "unique-workspace-id",
  "name": "Full Stack Development",
  "description": "Frontend, Backend, and Database",
  "version": 1,
  "projects": [
    {
      "id": "project-id-1",
      "name": "Frontend App",
      "path": "/path/to/frontend",
      "pinnedScripts": ["dev", "build"]
    },
    {
      "id": "project-id-2",
      "name": "Backend API",
      "path": "/path/to/backend",
      "pinnedScripts": ["dev"]
    }
  ]
}
```

### Process Management Integration

The workspace system integrates with ZenRunner's process manager:
- Each project process gets a unique ID: `workspaceId_projectId_scriptName`
- Processes can be monitored individually or as a group
- Log output is captured per process
- Process states are tracked in real-time

## Keyboard Shortcuts (Planned)

- `Ctrl/Cmd + N`: Create new workspace
- `Ctrl/Cmd + S`: Start selected workspace
- `Ctrl/Cmd + Q`: Stop selected workspace
- `Ctrl/Cmd + E`: Edit selected workspace
- `Delete`: Delete selected workspace (with confirmation)

## Troubleshooting

### Workspace doesn't start all projects
**Possible causes**:
1. Some projects don't have the specified script (e.g., "dev")
2. Project path is invalid
3. Process manager is busy

**Solution**: Check that all projects have the same script name in their package.json

### Sequential mode doesn't wait between starts
**Note**: In the current implementation, sequential mode starts processes one after another but doesn't wait for each to be fully ready. This is a known limitation and will be enhanced in future versions to include proper startup detection.

### Workspace shows "0 projects"
**Possible causes**:
1. Projects haven't been added to the workspace yet
2. Project paths are no longer valid

**Solution**: Add projects to the workspace through the project management interface

## API Reference (for developers)

### WorkspaceViewModel

```cpp
// Create a workspace
workspaceViewModel.createWorkspace("Workspace Name", "Description")

// Start all projects (parallel)
workspaceViewModel.startAllProjects(workspaceId, "dev", true)

// Start all projects (sequential)
workspaceViewModel.startAllProjects(workspaceId, "dev", false)

// Stop all projects
workspaceViewModel.stopAllProjects(workspaceId, false)

// Delete workspace
workspaceViewModel.deleteWorkspace(workspaceId)
```

### QML Integration

```qml
// Access from QML
WorkspaceManager {
    workspaceViewModel: workspaceViewModel  // Set from C++
}

// Create workspace from QML
workspaceViewModel.createWorkspace(name, description)

// Start all parallel
workspaceViewModel.startAllProjects(workspaceId, "dev", true)

// Start all sequential
workspaceViewModel.startAllProjects(workspaceId, "dev", false)
```

## Future Enhancements

- [ ] Custom startup delays for sequential mode
- [ ] Dependency graphs between projects
- [ ] Health checks before marking services as "ready"
- [ ] Workspace templates for common setups
- [ ] Import/export workspace configurations
- [ ] Workspace-level environment variables
- [ ] Script selection (choose which script to run per project)
- [ ] Startup orchestration with wait conditions

## Feedback

The workspace system is designed to be flexible and extensible. If you have suggestions for improvements or encounter issues, please open an issue on the GitHub repository.
