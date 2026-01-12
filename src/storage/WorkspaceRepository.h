#pragma once

#include "storage/IWorkspaceRepository.hpp"
#include "storage/IProjectRepository.hpp"
#include "core/Workspace.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

namespace ZenRunner::Storage {

/**
 * @brief JSON-based workspace repository implementation
 * 
 * Stores workspaces as JSON files in the application data directory.
 * Each workspace is stored in a separate file named <workspaceId>.json
 */
class WorkspaceRepository : public IWorkspaceRepository {
public:
    /**
     * @brief Construct a workspace repository
     * @param storageDir Optional custom storage directory (uses app data dir if empty)
     */
    explicit WorkspaceRepository(const QString& storageDir = QString());

    bool saveWorkspace(const Core::IWorkspace& workspace) override;
    std::shared_ptr<Core::IWorkspace> loadWorkspace(const QString& workspaceId) const override;
    bool deleteWorkspace(const QString& workspaceId) override;
    std::vector<QString> getAllWorkspaceIds() const override;
    std::vector<std::shared_ptr<Core::IWorkspace>> getAllWorkspaces() override;
    bool workspaceExists(const QString& workspaceId) const override;
    
    bool updateWorkspaceMetadata(
        const QString& workspaceId,
        const QString& name,
        const QString& description
    ) override;
    
    bool addProjectToWorkspace(
        const QString& workspaceId,
        const QString& projectId
    ) override;
    
    bool removeProjectFromWorkspace(
        const QString& workspaceId,
        const QString& projectId
    ) override;
    
    std::vector<QString> getWorkspaceProjectIds(const QString& workspaceId) const override;
    QString getStorageDirectory() const override;
    void clear() override;

private:
    QString storageDir_;
    
    /**
     * @brief Get the file path for a workspace
     * @param workspaceId Workspace identifier
     * @return Absolute path to workspace file
     */
    QString getWorkspaceFilePath(const QString& workspaceId) const;
    
    /**
     * @brief Serialize workspace to JSON
     * @param workspace Workspace to serialize
     * @return JSON object
     */
    QJsonObject workspaceToJson(const Core::IWorkspace& workspace) const;
    
    /**
     * @brief Deserialize workspace from JSON
     * @param json JSON object
     * @return Shared pointer to workspace
     */
    std::shared_ptr<Core::IWorkspace> workspaceFromJson(const QJsonObject& json) const;
    
    /**
     * @brief Ensure storage directory exists
     * @return true if directory exists or was created
     */
    bool ensureStorageDirectoryExists() const;
};

} // namespace ZenRunner::Storage
