#pragma once

#include "core/Project.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QSaveFile>
#include <memory>
#include <vector>

namespace ZenRunner::Storage {

/**
 * @brief JSON-based project repository implementation
 * 
 * Stores projects as JSON files in the application data directory.
 * Each project is stored in a separate file named <projectId>.json
 * 
 * Features:
 * - Atomic writes using QSaveFile to prevent corruption
 * - Validation and recovery for malformed data
 * - Modular storage for portability
 */
class ProjectRepository {
public:
    /**
     * @brief Construct a project repository
     * @param storageDir Optional custom storage directory (uses app data dir if empty)
     */
    explicit ProjectRepository(const QString& storageDir = QString());

    /**
     * @brief Save a project to persistent storage
     * 
     * NOTE: The project ID is stored but will change on reload since Project
     * generates new UUIDs. Use the project path as the stable identifier.
     * 
     * @param project Project to save
     * @return true if save was successful
     */
    bool saveProject(const Project& project);

    /**
     * @brief Load a project by ID
     * 
     * NOTE: Due to the Project class generating new UUIDs on each load from disk,
     * the returned project will have a different ID than the stored ID. The project
     * is instead identified by its path. The stored ID is kept for reference but
     * should not be used as a stable identifier across sessions.
     * 
     * For stable project identification, use the project's path instead.
     * 
     * @param projectId Project identifier (stored for reference only)
     * @return Shared pointer to project, or nullptr if not found
     */
    std::shared_ptr<Project> loadProject(const QString& projectId);
    std::shared_ptr<Project> loadProjectFromPath(const QString& projectPath);
    bool deleteProject(const QString& projectId);
    std::vector<QString> getAllProjectIds() const;
    std::vector<std::shared_ptr<Project>> getAllProjects();
    bool projectExists(const QString& projectId) const;
    bool isPathRegistered(const QString& projectPath) const;
    
    bool updateProjectMetadata(
        const QString& projectId,
        const QString& name,
        const QStringList& pinnedScripts
    );
    
    QString getStorageDirectory() const;
    void clear();

private:
    QString storageDir_;
    
    /**
     * @brief Get the file path for a project
     * @param projectId Project identifier
     * @return Absolute path to project file
     */
    QString getProjectFilePath(const QString& projectId) const;
    
    /**
     * @brief Serialize project to JSON
     * @param project Project to serialize
     * @return JSON object
     */
    QJsonObject projectToJson(const Project& project) const;
    
    /**
     * @brief Deserialize project from JSON with validation
     * @param json JSON object
     * @param projectId Project ID for error reporting
     * @return Shared pointer to project, nullptr if validation fails
     */
    std::shared_ptr<Project> projectFromJson(
        const QJsonObject& json,
        const QString& projectId) const;
    
    /**
     * @brief Validate project JSON structure
     * @param json JSON object to validate
     * @return true if JSON is valid
     */
    bool validateProjectJson(const QJsonObject& json) const;
    
    /**
     * @brief Ensure storage directory exists
     * @return true if directory exists or was created
     */
    bool ensureStorageDirectoryExists() const;
    
    /**
     * @brief Create path to ID mapping index
     * This helps with efficient lookup when checking if a path is registered
     */
    void rebuildPathIndex();
    
    // Cache for path -> projectId mapping
    mutable QMap<QString, QString> pathToIdCache_;
    mutable bool pathIndexValid_{false};
};

} // namespace ZenRunner::Storage
