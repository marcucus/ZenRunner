#pragma once

#include "core/IProcessManager.hpp"
#include "core/IProject.hpp"
#include "core/IWorkspace.hpp"
#include "core/IJsonParser.hpp"
#include "core/ILogBuffer.hpp"
#include "platform/IPlatformEffect.hpp"
#include "platform/ISystemTray.hpp"
#include "platform/INativeNotifications.hpp"
#include "platform/IPlatformUtils.hpp"
#include "storage/ISettingsManager.hpp"
#include "storage/IProjectRepository.hpp"
#include "storage/IWorkspaceRepository.hpp"
#include "ui/ILogViewModel.hpp"
#include "ui/IProjectViewModel.hpp"
#include "ui/IWorkspaceViewModel.hpp"

#include <memory>
#include <QString>

namespace ZenRunner {

/**
 * @brief Central factory for creating interface implementations
 * 
 * This factory provides a single point of creation for all layer objects,
 * ensuring proper dependency injection and layer separation.
 */
class IApplicationFactory {
public:
    virtual ~IApplicationFactory() = default;

    // Core Layer Factory Methods
    
    /**
     * @brief Create a process manager instance
     * @return Unique pointer to process manager
     */
    virtual std::unique_ptr<Core::IProcessManager> createProcessManager() = 0;

    /**
     * @brief Create a project instance
     * @param projectPath Path to the project directory
     * @return Shared pointer to project
     */
    virtual std::shared_ptr<Core::IProject> createProject(const QString& projectPath) = 0;

    /**
     * @brief Create a workspace instance
     * @param name Workspace name
     * @param description Workspace description
     * @return Shared pointer to workspace
     */
    virtual std::shared_ptr<Core::IWorkspace> createWorkspace(
        const QString& name,
        const QString& description = QString()
    ) = 0;

    /**
     * @brief Create a JSON parser instance
     * @return Unique pointer to JSON parser
     */
    virtual std::unique_ptr<Core::IJsonParser> createJsonParser() = 0;

    /**
     * @brief Create a log buffer instance
     * @param capacity Maximum number of log entries
     * @return Shared pointer to log buffer
     */
    virtual std::shared_ptr<Core::ILogBuffer> createLogBuffer(size_t capacity = 5000) = 0;

    // Platform Layer Factory Methods

    /**
     * @brief Create a platform effect manager for the current OS
     * @return Unique pointer to platform effect manager
     */
    virtual std::unique_ptr<Platform::IPlatformEffect> createPlatformEffect() = 0;

    /**
     * @brief Create a system tray instance
     * @return Unique pointer to system tray
     */
    virtual std::unique_ptr<Platform::ISystemTray> createSystemTray() = 0;

    /**
     * @brief Create a native notifications instance
     * @return Unique pointer to native notifications
     */
    virtual std::unique_ptr<Platform::INativeNotifications> createNativeNotifications() = 0;

    /**
     * @brief Create a platform utils instance
     * @return Unique pointer to platform utils
     */
    virtual std::unique_ptr<Platform::IPlatformUtils> createPlatformUtils() = 0;

    // Storage Layer Factory Methods

    /**
     * @brief Create a settings manager instance
     * @return Unique pointer to settings manager
     */
    virtual std::unique_ptr<Storage::ISettingsManager> createSettingsManager() = 0;

    /**
     * @brief Create a project repository instance
     * @return Unique pointer to project repository
     */
    virtual std::unique_ptr<Storage::IProjectRepository> createProjectRepository() = 0;

    /**
     * @brief Create a workspace repository instance
     * @return Unique pointer to workspace repository
     */
    virtual std::unique_ptr<Storage::IWorkspaceRepository> createWorkspaceRepository() = 0;

    // UI Layer Factory Methods

    /**
     * @brief Create a log view model instance
     * @param logBuffer Log buffer data source
     * @return Unique pointer to log view model
     */
    virtual std::unique_ptr<UI::ILogViewModel> createLogViewModel(
        std::shared_ptr<Core::ILogBuffer> logBuffer
    ) = 0;

    /**
     * @brief Create a project view model instance
     * @param project Project data source
     * @return Unique pointer to project view model
     */
    virtual std::unique_ptr<UI::IProjectViewModel> createProjectViewModel(
        std::shared_ptr<Core::IProject> project
    ) = 0;

    /**
     * @brief Create a workspace view model instance
     * @param workspace Workspace data source
     * @return Unique pointer to workspace view model
     */
    virtual std::unique_ptr<UI::IWorkspaceViewModel> createWorkspaceViewModel(
        std::shared_ptr<Core::IWorkspace> workspace
    ) = 0;
};

/**
 * @brief Get the global application factory instance
 * @return Reference to the application factory
 */
IApplicationFactory& getApplicationFactory();

/**
 * @brief Set the global application factory instance
 * @param factory Unique pointer to factory implementation
 */
void setApplicationFactory(std::unique_ptr<IApplicationFactory> factory);

} // namespace ZenRunner
