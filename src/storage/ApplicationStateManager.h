#pragma once

#include "storage/IApplicationStateManager.hpp"
#include "storage/ISettingsManager.hpp"
#include "storage/IWorkspaceRepository.hpp"
#include "storage/ProjectRepository.h"
#include <QTimer>
#include <memory>

namespace ZenRunner::Storage {

/**
 * @brief High-level application state manager
 * 
 * Orchestrates state persistence across all storage components:
 * - Global preferences via ISettingsManager
 * - Workspace data via IWorkspaceRepository
 * - Project data via ProjectRepository
 * 
 * Features:
 * - Periodic auto-save with timer
 * - Atomic state persistence
 * - Validation and recovery for corrupted data
 * - Modular storage architecture
 */
class ApplicationStateManager : public IApplicationStateManager {
public:
    /**
     * @brief Construct application state manager
     * @param settingsManager Settings manager instance
     * @param workspaceRepo Workspace repository instance
     * @param projectRepo Project repository instance
     */
    ApplicationStateManager(
        std::shared_ptr<ISettingsManager> settingsManager,
        std::shared_ptr<IWorkspaceRepository> workspaceRepo,
        std::shared_ptr<ProjectRepository> projectRepo
    );

    ~ApplicationStateManager() override;

    bool initialize() override;
    bool saveState() override;
    bool restoreState() override;
    
    void setAutoSaveEnabled(bool enabled, int intervalSeconds = 60) override;
    bool isAutoSaveEnabled() const override;
    int getAutoSaveInterval() const override;
    
    QString getLastWorkspaceId() const override;
    void setLastWorkspaceId(const QString& workspaceId) override;
    
    QStringList getRecentProjects(int maxCount = 10) const override;
    void addRecentProject(const QString& projectId) override;
    void clearRecentProjects() override;
    
    bool getWindowGeometry(int& x, int& y, int& width, int& height) const override;
    void setWindowGeometry(int x, int y, int width, int height) override;
    
    bool isFirstRun() const override;
    void setFirstRunComplete() override;
    
    bool validateAndRecover() override;
    void clearAll() override;

private:
    std::shared_ptr<ISettingsManager> settings_;
    std::shared_ptr<IWorkspaceRepository> workspaceRepo_;
    std::shared_ptr<ProjectRepository> projectRepo_;
    
    std::unique_ptr<QTimer> autoSaveTimer_;
    bool autoSaveEnabled_{false};
    int autoSaveInterval_{60};
    
    /**
     * @brief Save global preferences
     */
    void savePreferences();
    
    /**
     * @brief Restore global preferences
     */
    void restorePreferences();
    
    /**
     * @brief Validate workspace repository data
     * @return true if validation passed
     */
    bool validateWorkspaces();
    
    /**
     * @brief Validate project repository data
     * @return true if validation passed
     */
    bool validateProjects();
    
    /**
     * @brief Attempt to recover corrupted workspaces
     * @return Number of workspaces recovered
     */
    int recoverWorkspaces();
    
    /**
     * @brief Attempt to recover corrupted projects
     * @return Number of projects recovered
     */
    int recoverProjects();
    
    /**
     * @brief Initialize auto-save timer
     */
    void setupAutoSaveTimer();
    
    /**
     * @brief Auto-save timer callback
     */
    void onAutoSaveTimer();
};

/**
 * @brief Factory function to create ApplicationStateManager
 * @param settingsManager Settings manager instance
 * @param workspaceRepo Workspace repository instance
 * @param projectRepo Project repository instance
 * @return Unique pointer to ApplicationStateManager
 */
std::unique_ptr<IApplicationStateManager> createApplicationStateManager(
    std::shared_ptr<ISettingsManager> settingsManager,
    std::shared_ptr<IWorkspaceRepository> workspaceRepo,
    std::shared_ptr<ProjectRepository> projectRepo
);

} // namespace ZenRunner::Storage
