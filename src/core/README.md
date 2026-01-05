# Core Module

This module contains the core business logic and data management components of ZenRunner.

## Responsibilities

- **Process Management**: Handling QProcess lifecycle, execution, monitoring, and termination
- **Data Models**: Project and Workspace data structures, state management
- **JSON Parsing**: Configuration file parsing (package.json), project detection, and serialization

## Key Components (Planned)

- `ProcessManager`: Manages multiple QProcess instances
- `Project`: Represents a single project with its scripts and metadata
- `Workspace`: Container for multiple related projects
- `JsonParser`: Utilities for parsing package.json and configuration files
- `CircularBuffer`: Memory-efficient log storage implementation

## Design Principles

- Asynchronous operations using Qt's signals/slots
- Minimal memory footprint (< 30MB target)
- Thread-safe where necessary
- Clean separation from UI layer
