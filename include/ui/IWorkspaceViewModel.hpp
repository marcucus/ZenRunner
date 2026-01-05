#pragma once

#include "core/IWorkspace.hpp"
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Interface for workspace view model
 * 
 * This interface defines the contract for exposing workspace data to the UI layer.
 * It provides a QML-friendly interface to workspace information and operations.
 */
class IWorkspaceViewModel : public QObject {
    Q_OBJECT

public:
    virtual ~IWorkspaceViewModel() = default;

    /**
     * @brief Set the workspace data source
     * @param workspace Shared pointer to workspace
     */
    virtual void setWorkspace(std::shared_ptr<Core::IWorkspace> workspace) = 0;

    /**
     * @brief Get the current workspace
     * @return Shared pointer to workspace
     */
    virtual std::shared_ptr<Core::IWorkspace> getWorkspace() const = 0;

    /**
     * @brief Get the workspace ID
     * @return Workspace ID
     */
    Q_INVOKABLE virtual QString getWorkspaceId() const = 0;

    /**
     * @brief Get the workspace name
     * @return Workspace name
     */
    Q_INVOKABLE virtual QString getWorkspaceName() const = 0;

    /**
     * @brief Set the workspace name
     * @param name New workspace name
     */
    Q_INVOKABLE virtual void setWorkspaceName(const QString& name) = 0;

    /**
     * @brief Get the workspace description
     * @return Workspace description
     */
    Q_INVOKABLE virtual QString getDescription() const = 0;

    /**
     * @brief Set the workspace description
     * @param description New description
     */
    Q_INVOKABLE virtual void setDescription(const QString& description) = 0;

    /**
     * @brief Get the number of projects in workspace
     * @return Project count
     */
    Q_INVOKABLE virtual int getProjectCount() const = 0;

    /**
     * @brief Get list of project IDs in workspace
     * @return List of project IDs
     */
    Q_INVOKABLE virtual QStringList getProjectIds() const = 0;

    /**
     * @brief Add a project to the workspace
     * @param projectId ID of project to add
     */
    Q_INVOKABLE virtual void addProject(const QString& projectId) = 0;

    /**
     * @brief Remove a project from the workspace
     * @param projectId ID of project to remove
     */
    Q_INVOKABLE virtual void removeProject(const QString& projectId) = 0;

    /**
     * @brief Start all projects with a specific script
     * @param scriptName Name of script to run on all projects
     */
    Q_INVOKABLE virtual void startAll(const QString& scriptName) = 0;

    /**
     * @brief Stop all running processes in workspace
     */
    Q_INVOKABLE virtual void stopAll() = 0;

    /**
     * @brief Check if any project in workspace is running
     * @return true if any project is running
     */
    Q_INVOKABLE virtual bool isAnyProjectRunning() const = 0;

signals:
    /**
     * @brief Emitted when workspace data changes
     */
    void workspaceChanged();

    /**
     * @brief Emitted when projects in workspace change
     */
    void projectsChanged();

    /**
     * @brief Emitted when workspace name changes
     */
    void nameChanged();

    /**
     * @brief Emitted when workspace description changes
     */
    void descriptionChanged();
};

} // namespace ZenRunner::UI
