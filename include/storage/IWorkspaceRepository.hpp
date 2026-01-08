#pragma once

#include "core/IWorkspace.hpp"
#include <QString>
#include <memory>
#include <vector>

namespace ZenRunner::Storage {

/**
 * @brief Interface for workspace data persistence
 * 
 * This interface defines the contract for storing and retrieving workspace
 * configurations and their associated projects.
 */
class IWorkspaceRepository {
public:
    virtual ~IWorkspaceRepository() = default;

    /**
     * @brief Save a workspace to persistent storage
     * @param workspace Workspace to save
     * @return true if save was successful
     */
    virtual bool saveWorkspace(const Core::IWorkspace& workspace) = 0;

    /**
     * @brief Load a workspace by ID
     * @param workspaceId Workspace identifier
     * @return Shared pointer to workspace, or nullptr if not found
     */
    virtual std::shared_ptr<Core::IWorkspace> loadWorkspace(const QString& workspaceId) const = 0;

    /**
     * @brief Delete a workspace from storage
     * @param workspaceId Workspace identifier
     * @return true if deletion was successful
     */
    virtual bool deleteWorkspace(const QString& workspaceId) = 0;

    /**
     * @brief Get all stored workspace IDs
     * @return Vector of workspace IDs
     */
    virtual std::vector<QString> getAllWorkspaceIds() const = 0;

    /**
     * @brief Get all stored workspaces
     * @return Vector of workspace pointers
     */
    virtual std::vector<std::shared_ptr<Core::IWorkspace>> getAllWorkspaces() = 0;

    /**
     * @brief Check if a workspace exists in storage
     * @param workspaceId Workspace identifier
     * @return true if workspace exists
     */
    virtual bool workspaceExists(const QString& workspaceId) const = 0;

    /**
     * @brief Update workspace metadata without full reload
     * @param workspaceId Workspace identifier
     * @param name New workspace name
     * @param description New description
     * @return true if update was successful
     */
    virtual bool updateWorkspaceMetadata(
        const QString& workspaceId,
        const QString& name,
        const QString& description
    ) = 0;

    /**
     * @brief Add a project to a workspace
     * @param workspaceId Workspace identifier
     * @param projectId Project identifier to add
     * @return true if project was added
     */
    virtual bool addProjectToWorkspace(
        const QString& workspaceId,
        const QString& projectId
    ) = 0;

    /**
     * @brief Remove a project from a workspace
     * @param workspaceId Workspace identifier
     * @param projectId Project identifier to remove
     * @return true if project was removed
     */
    virtual bool removeProjectFromWorkspace(
        const QString& workspaceId,
        const QString& projectId
    ) = 0;

    /**
     * @brief Get all project IDs in a workspace
     * @param workspaceId Workspace identifier
     * @return Vector of project IDs
     */
    virtual std::vector<QString> getWorkspaceProjectIds(const QString& workspaceId) const = 0;

    /**
     * @brief Get the storage directory path
     * @return Path to storage directory
     */
    virtual QString getStorageDirectory() const = 0;

    /**
     * @brief Clear all stored workspaces
     */
    virtual void clear() = 0;
};

} // namespace ZenRunner::Storage
