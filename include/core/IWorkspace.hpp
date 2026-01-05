#pragma once

#include "IProject.hpp"
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

namespace ZenRunner::Core {

/**
 * @brief Interface for workspace management
 * 
 * A workspace is a collection of related projects that can be managed together.
 * Workspaces allow batch operations on multiple projects.
 */
class IWorkspace {
public:
    virtual ~IWorkspace() = default;

    /**
     * @brief Get the unique identifier for this workspace
     * @return Workspace ID
     */
    virtual QString getId() const = 0;

    /**
     * @brief Get the workspace name
     * @return Workspace name
     */
    virtual QString getName() const = 0;

    /**
     * @brief Set the workspace name
     * @param name New workspace name
     */
    virtual void setName(const QString& name) = 0;

    /**
     * @brief Get the workspace description
     * @return Workspace description
     */
    virtual QString getDescription() const = 0;

    /**
     * @brief Set the workspace description
     * @param description New description
     */
    virtual void setDescription(const QString& description) = 0;

    /**
     * @brief Add a project to this workspace
     * @param project Shared pointer to project
     * @return true if project was added
     */
    virtual bool addProject(std::shared_ptr<IProject> project) = 0;

    /**
     * @brief Remove a project from this workspace
     * @param projectId ID of project to remove
     * @return true if project was removed
     */
    virtual bool removeProject(const QString& projectId) = 0;

    /**
     * @brief Get all projects in this workspace
     * @return Vector of project pointers
     */
    virtual std::vector<std::shared_ptr<IProject>> getProjects() const = 0;

    /**
     * @brief Get a specific project by ID
     * @param projectId ID of the project
     * @return Shared pointer to project, or nullptr if not found
     */
    virtual std::shared_ptr<IProject> getProject(const QString& projectId) const = 0;

    /**
     * @brief Check if workspace contains a project
     * @param projectId ID of the project
     * @return true if workspace contains the project
     */
    virtual bool hasProject(const QString& projectId) const = 0;

    /**
     * @brief Get the number of projects in this workspace
     * @return Project count
     */
    virtual size_t getProjectCount() const = 0;

    /**
     * @brief Execution mode for batch operations
     */
    enum class ExecutionMode {
        Parallel,    ///< Start all processes simultaneously
        Sequential   ///< Start processes one after another
    };

    /**
     * @brief Start all projects in the workspace
     * @param scriptName Name of script to run on all projects
     * @param mode Execution mode (parallel or sequential)
     * @return Number of projects successfully started
     */
    virtual int startAll(const QString& scriptName, ExecutionMode mode = ExecutionMode::Parallel) = 0;

    /**
     * @brief Stop all running processes in this workspace
     * @param forceKill If true, force kill all processes
     * @return Number of processes stopped
     */
    virtual int stopAll(bool forceKill = false) = 0;

    /**
     * @brief Check if any project in the workspace is running
     * @return true if at least one project is running
     */
    virtual bool isAnyProjectRunning() const = 0;
};

} // namespace ZenRunner::Core
