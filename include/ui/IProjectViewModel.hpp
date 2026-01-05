#pragma once

#include "core/IProject.hpp"
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Interface for project view model
 * 
 * This interface defines the contract for exposing project data to the UI layer.
 * It provides a QML-friendly interface to project information and operations.
 */
class IProjectViewModel : public QObject {
    Q_OBJECT

public:
    virtual ~IProjectViewModel() = default;

    /**
     * @brief Set the project data source
     * @param project Shared pointer to project
     */
    virtual void setProject(std::shared_ptr<Core::IProject> project) = 0;

    /**
     * @brief Get the current project
     * @return Shared pointer to project
     */
    virtual std::shared_ptr<Core::IProject> getProject() const = 0;

    /**
     * @brief Get the project ID
     * @return Project ID
     */
    Q_INVOKABLE virtual QString getProjectId() const = 0;

    /**
     * @brief Get the project name
     * @return Project name
     */
    Q_INVOKABLE virtual QString getProjectName() const = 0;

    /**
     * @brief Get the project path
     * @return Project path
     */
    Q_INVOKABLE virtual QString getProjectPath() const = 0;

    /**
     * @brief Get list of all script names
     * @return List of script names
     */
    Q_INVOKABLE virtual QStringList getScriptNames() const = 0;

    /**
     * @brief Get list of pinned script names
     * @return List of pinned script names
     */
    Q_INVOKABLE virtual QStringList getPinnedScripts() const = 0;

    /**
     * @brief Check if a script is pinned
     * @param scriptName Name of the script
     * @return true if script is pinned
     */
    Q_INVOKABLE virtual bool isScriptPinned(const QString& scriptName) const = 0;

    /**
     * @brief Pin a script
     * @param scriptName Name of script to pin
     */
    Q_INVOKABLE virtual void pinScript(const QString& scriptName) = 0;

    /**
     * @brief Unpin a script
     * @param scriptName Name of script to unpin
     */
    Q_INVOKABLE virtual void unpinScript(const QString& scriptName) = 0;

    /**
     * @brief Run a script
     * @param scriptName Name of script to run
     */
    Q_INVOKABLE virtual void runScript(const QString& scriptName) = 0;

    /**
     * @brief Stop all running processes for this project
     */
    Q_INVOKABLE virtual void stopAllProcesses() = 0;

    /**
     * @brief Check if project has any running processes
     * @return true if processes are running
     */
    Q_INVOKABLE virtual bool isRunning() const = 0;

    /**
     * @brief Reload project from disk
     */
    Q_INVOKABLE virtual void reload() = 0;

signals:
    /**
     * @brief Emitted when project data changes
     */
    void projectChanged();

    /**
     * @brief Emitted when a script starts
     * @param scriptName Name of the script
     */
    void scriptStarted(const QString& scriptName);

    /**
     * @brief Emitted when a script stops
     * @param scriptName Name of the script
     * @param exitCode Exit code of the process
     */
    void scriptStopped(const QString& scriptName, int exitCode);

    /**
     * @brief Emitted when pinned scripts change
     */
    void pinnedScriptsChanged();
};

} // namespace ZenRunner::UI
