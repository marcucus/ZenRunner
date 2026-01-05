#pragma once

#include "core/IProject.hpp"
#include <QString>
#include <memory>
#include <vector>
#include <optional>

namespace ZenRunner::Storage {

/**
 * @brief Interface for project data persistence
 * 
 * This interface defines the contract for storing and retrieving project
 * configurations and metadata.
 */
class IProjectRepository {
public:
    virtual ~IProjectRepository() = default;

    /**
     * @brief Save a project to persistent storage
     * @param project Project to save
     * @return true if save was successful
     */
    virtual bool saveProject(const Core::IProject& project) = 0;

    /**
     * @brief Load a project by ID
     * @param projectId Project identifier
     * @return Shared pointer to project, or nullptr if not found
     */
    virtual std::shared_ptr<Core::IProject> loadProject(const QString& projectId) = 0;

    /**
     * @brief Load a project from its path
     * @param projectPath Path to project directory
     * @return Shared pointer to project, or nullptr if not found
     */
    virtual std::shared_ptr<Core::IProject> loadProjectFromPath(const QString& projectPath) = 0;

    /**
     * @brief Delete a project from storage
     * @param projectId Project identifier
     * @return true if deletion was successful
     */
    virtual bool deleteProject(const QString& projectId) = 0;

    /**
     * @brief Get all stored project IDs
     * @return Vector of project IDs
     */
    virtual std::vector<QString> getAllProjectIds() const = 0;

    /**
     * @brief Get all stored projects
     * @return Vector of project pointers
     */
    virtual std::vector<std::shared_ptr<Core::IProject>> getAllProjects() = 0;

    /**
     * @brief Check if a project exists in storage
     * @param projectId Project identifier
     * @return true if project exists
     */
    virtual bool projectExists(const QString& projectId) const = 0;

    /**
     * @brief Check if a project path is already registered
     * @param projectPath Project path
     * @return true if path is registered
     */
    virtual bool isPathRegistered(const QString& projectPath) const = 0;

    /**
     * @brief Update project metadata without reloading
     * @param projectId Project identifier
     * @param name New project name
     * @param pinnedScripts List of pinned scripts
     * @return true if update was successful
     */
    virtual bool updateProjectMetadata(
        const QString& projectId,
        const QString& name,
        const QStringList& pinnedScripts
    ) = 0;

    /**
     * @brief Get the storage directory path
     * @return Path to storage directory
     */
    virtual QString getStorageDirectory() const = 0;

    /**
     * @brief Clear all stored projects
     */
    virtual void clear() = 0;
};

} // namespace ZenRunner::Storage
