#pragma once

#include <QString>
#include <QStringList>
#include <memory>

namespace ZenRunner::Storage {

/**
 * @brief Interface for application state management
 * 
 * This interface defines the contract for managing the complete application state,
 * including global preferences, workspace configuration, and project state.
 * 
 * Features:
 * - Periodic auto-save with configurable intervals
 * - Atomic state persistence to prevent corruption
 * - State restoration with validation and recovery
 * - Modular storage mechanism for portability
 */
class IApplicationStateManager {
public:
    virtual ~IApplicationStateManager() = default;

    /**
     * @brief Initialize state manager and restore previous state
     * @return true if initialization was successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Save current application state to disk
     * @return true if save was successful
     */
    virtual bool saveState() = 0;

    /**
     * @brief Restore application state from disk
     * @return true if restoration was successful
     */
    virtual bool restoreState() = 0;

    /**
     * @brief Enable/disable periodic auto-save
     * @param enabled Whether to enable auto-save
     * @param intervalSeconds Interval between auto-saves (default: 60 seconds)
     */
    virtual void setAutoSaveEnabled(bool enabled, int intervalSeconds = 60) = 0;

    /**
     * @brief Check if auto-save is enabled
     * @return true if auto-save is enabled
     */
    virtual bool isAutoSaveEnabled() const = 0;

    /**
     * @brief Get the current auto-save interval
     * @return Interval in seconds
     */
    virtual int getAutoSaveInterval() const = 0;

    /**
     * @brief Get the last saved workspace ID
     * @return Workspace ID or empty string if none
     */
    virtual QString getLastWorkspaceId() const = 0;

    /**
     * @brief Set the last active workspace ID
     * @param workspaceId Workspace ID
     */
    virtual void setLastWorkspaceId(const QString& workspaceId) = 0;

    /**
     * @brief Get list of recently opened projects
     * @param maxCount Maximum number of recent projects to return
     * @return List of project IDs
     */
    virtual QStringList getRecentProjects(int maxCount = 10) const = 0;

    /**
     * @brief Add a project to the recent projects list
     * @param projectId Project ID
     */
    virtual void addRecentProject(const QString& projectId) = 0;

    /**
     * @brief Clear recent projects list
     */
    virtual void clearRecentProjects() = 0;

    /**
     * @brief Get window geometry state
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @return true if geometry was restored
     */
    virtual bool getWindowGeometry(int& x, int& y, int& width, int& height) const = 0;

    /**
     * @brief Set window geometry state
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     */
    virtual void setWindowGeometry(int x, int y, int width, int height) = 0;

    /**
     * @brief Check if this is the first run of the application
     * @return true if first run
     */
    virtual bool isFirstRun() const = 0;

    /**
     * @brief Mark first run as complete
     */
    virtual void setFirstRunComplete() = 0;

    /**
     * @brief Validate stored state and recover if necessary
     * @return true if state is valid or was successfully recovered
     */
    virtual bool validateAndRecover() = 0;

    /**
     * @brief Clear all application state
     * WARNING: This will delete all stored preferences, workspaces, and projects
     */
    virtual void clearAll() = 0;
};

} // namespace ZenRunner::Storage
