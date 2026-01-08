#pragma once

#include "core/IWorkspace.hpp"
#include "core/IProcessManager.hpp"
#include "storage/IWorkspaceRepository.hpp"
#include "storage/IProjectRepository.hpp"
#include <QAbstractListModel>
#include <QObject>
#include <memory>
#include <vector>

namespace ZenRunner::UI {

/**
 * @brief View model for workspace management in QML
 * 
 * Exposes workspace data and operations to the QML UI layer.
 * Implements QAbstractListModel for efficient list rendering.
 */
class WorkspaceViewModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum WorkspaceRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        ProjectCountRole,
        IsRunningRole,
        ColorRole  // For UI display
    };

    explicit WorkspaceViewModel(QObject* parent = nullptr);
    ~WorkspaceViewModel() override = default;

    /**
     * @brief Set the workspace repository
     * @param repository Pointer to workspace repository
     */
    void setRepository(Storage::IWorkspaceRepository* repository);

    /**
     * @brief Set the process manager
     * @param manager Pointer to process manager
     */
    void setProcessManager(Core::IProcessManager* manager);

    /**
     * @brief Set the project repository
     * @param repository Pointer to project repository
     */
    void setProjectRepository(Storage::IProjectRepository* repository);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Get the number of workspaces
     * @return Workspace count
     */
    int count() const { return static_cast<int>(workspaces_.size()); }

public slots:
    /**
     * @brief Load all workspaces from storage
     */
    void loadWorkspaces();

    /**
     * @brief Create a new workspace
     * @param name Workspace name
     * @param description Workspace description
     * @return true if workspace was created
     */
    bool createWorkspace(const QString& name, const QString& description = QString());

    /**
     * @brief Delete a workspace
     * @param workspaceId Workspace identifier
     * @return true if workspace was deleted
     */
    bool deleteWorkspace(const QString& workspaceId);

    /**
     * @brief Update workspace metadata
     * @param workspaceId Workspace identifier
     * @param name New name
     * @param description New description
     * @return true if update was successful
     */
    bool updateWorkspace(const QString& workspaceId, const QString& name, const QString& description);

    /**
     * @brief Add a project to a workspace
     * @param workspaceId Workspace identifier
     * @param projectId Project identifier
     * @return true if project was added
     */
    bool addProjectToWorkspace(const QString& workspaceId, const QString& projectId);

    /**
     * @brief Remove a project from a workspace
     * @param workspaceId Workspace identifier
     * @param projectId Project identifier
     * @return true if project was removed
     */
    bool removeProjectFromWorkspace(const QString& workspaceId, const QString& projectId);

    /**
     * @brief Start all projects in a workspace
     * @param workspaceId Workspace identifier
     * @param scriptName Script name to run
     * @param parallel true for parallel execution, false for sequential
     * @return Number of projects started
     */
    int startAllProjects(const QString& workspaceId, const QString& scriptName, bool parallel = true);

    /**
     * @brief Stop all projects in a workspace
     * @param workspaceId Workspace identifier
     * @param forceKill true to force kill processes
     * @return Number of processes stopped
     */
    int stopAllProjects(const QString& workspaceId, bool forceKill = false);

    /**
     * @brief Get workspace by ID
     * @param workspaceId Workspace identifier
     * @return Index of workspace in model, or -1 if not found
     */
    int getWorkspaceIndex(const QString& workspaceId) const;

signals:
    /**
     * @brief Emitted when workspace count changes
     */
    void countChanged();

    /**
     * @brief Emitted when a workspace is created
     * @param workspaceId Workspace identifier
     */
    void workspaceCreated(const QString& workspaceId);

    /**
     * @brief Emitted when a workspace is deleted
     * @param workspaceId Workspace identifier
     */
    void workspaceDeleted(const QString& workspaceId);

    /**
     * @brief Emitted when a workspace is updated
     * @param workspaceId Workspace identifier
     */
    void workspaceUpdated(const QString& workspaceId);

    /**
     * @brief Emitted when an error occurs
     * @param message Error message
     */
    void errorOccurred(const QString& message);

private:
    std::vector<std::shared_ptr<Core::IWorkspace>> workspaces_;
    Storage::IWorkspaceRepository* repository_;
    Storage::IProjectRepository* projectRepository_;
    Core::IProcessManager* processManager_;

    /**
     * @brief Find workspace by ID
     * @param workspaceId Workspace identifier
     * @return Pointer to workspace, or nullptr if not found
     */
    std::shared_ptr<Core::IWorkspace> findWorkspace(const QString& workspaceId) const;

    /**
     * @brief Generate a color for a workspace based on its name
     * @param name Workspace name
     * @return Color as hex string
     */
    QString generateWorkspaceColor(const QString& name) const;
};

} // namespace ZenRunner::UI
