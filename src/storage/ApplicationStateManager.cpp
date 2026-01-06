#include "ApplicationStateManager.h"
#include <QDebug>

namespace ZenRunner::Storage {

// Settings keys
static const QString KEY_FIRST_RUN = "app/firstRun";
static const QString KEY_LAST_WORKSPACE = "app/lastWorkspaceId";
static const QString KEY_RECENT_PROJECTS = "app/recentProjects";
static const QString KEY_AUTO_SAVE_ENABLED = "app/autoSaveEnabled";
static const QString KEY_AUTO_SAVE_INTERVAL = "app/autoSaveInterval";
static const QString KEY_WINDOW_X = "window/x";
static const QString KEY_WINDOW_Y = "window/y";
static const QString KEY_WINDOW_WIDTH = "window/width";
static const QString KEY_WINDOW_HEIGHT = "window/height";

ApplicationStateManager::ApplicationStateManager(
    std::shared_ptr<ISettingsManager> settingsManager,
    std::shared_ptr<IWorkspaceRepository> workspaceRepo,
    std::shared_ptr<ProjectRepository> projectRepo)
    : settings_(std::move(settingsManager))
    , workspaceRepo_(std::move(workspaceRepo))
    , projectRepo_(std::move(projectRepo))
    , autoSaveTimer_(std::make_unique<QTimer>())
{
    setupAutoSaveTimer();
}

ApplicationStateManager::~ApplicationStateManager() {
    // Save state on destruction
    saveState();
}

bool ApplicationStateManager::initialize() {
    qDebug() << "[ApplicationStateManager] Initializing...";
    
    // Validate and recover if necessary
    if (!validateAndRecover()) [[unlikely]] {
        qWarning() << "[ApplicationStateManager] Validation failed";
        // Continue anyway, but log the issue
    }
    
    // Restore previous state
    if (!restoreState()) [[unlikely]] {
        qWarning() << "[ApplicationStateManager] Failed to restore state";
        return false;
    }
    
    // Load auto-save preferences
    autoSaveEnabled_ = settings_->getBool(KEY_AUTO_SAVE_ENABLED, true);
    autoSaveInterval_ = settings_->getInt(KEY_AUTO_SAVE_INTERVAL, 60);
    
    // Start auto-save timer if enabled
    if (autoSaveEnabled_) {
        autoSaveTimer_->start(autoSaveInterval_ * 1000);
        qDebug() << "[ApplicationStateManager] Auto-save enabled with interval:" 
                 << autoSaveInterval_ << "seconds";
    }
    
    qDebug() << "[ApplicationStateManager] Initialization complete";
    return true;
}

bool ApplicationStateManager::saveState() {
    qDebug() << "[ApplicationStateManager] Saving state...";
    
    // Save preferences
    savePreferences();
    
    // Sync settings to disk
    settings_->sync();
    
    // Note: Workspaces and projects are saved individually when modified
    // This method primarily handles global preferences
    
    qDebug() << "[ApplicationStateManager] State saved successfully";
    return true;
}

bool ApplicationStateManager::restoreState() {
    qDebug() << "[ApplicationStateManager] Restoring state...";
    
    // Restore preferences
    restorePreferences();
    
    // Workspace and project restoration is handled by their respective managers
    // This method primarily handles global state
    
    qDebug() << "[ApplicationStateManager] State restored successfully";
    return true;
}

void ApplicationStateManager::setAutoSaveEnabled(bool enabled, int intervalSeconds) {
    autoSaveEnabled_ = enabled;
    autoSaveInterval_ = intervalSeconds;
    
    // Save preferences
    settings_->setValue(KEY_AUTO_SAVE_ENABLED, enabled);
    settings_->setValue(KEY_AUTO_SAVE_INTERVAL, intervalSeconds);
    settings_->sync();
    
    // Update timer
    if (enabled) {
        autoSaveTimer_->start(intervalSeconds * 1000);
        qDebug() << "[ApplicationStateManager] Auto-save enabled:" << intervalSeconds << "seconds";
    } else {
        autoSaveTimer_->stop();
        qDebug() << "[ApplicationStateManager] Auto-save disabled";
    }
}

bool ApplicationStateManager::isAutoSaveEnabled() const {
    return autoSaveEnabled_;
}

int ApplicationStateManager::getAutoSaveInterval() const {
    return autoSaveInterval_;
}

QString ApplicationStateManager::getLastWorkspaceId() const {
    return settings_->getString(KEY_LAST_WORKSPACE);
}

void ApplicationStateManager::setLastWorkspaceId(const QString& workspaceId) {
    settings_->setValue(KEY_LAST_WORKSPACE, workspaceId);
}

QStringList ApplicationStateManager::getRecentProjects(int maxCount) const {
    QStringList recent = settings_->getStringList(KEY_RECENT_PROJECTS);
    
    // Limit to maxCount
    if (recent.size() > maxCount) {
        recent = recent.mid(0, maxCount);
    }
    
    return recent;
}

void ApplicationStateManager::addRecentProject(const QString& projectId) {
    QStringList recent = settings_->getStringList(KEY_RECENT_PROJECTS);
    
    // Remove if already exists (to move to front)
    recent.removeAll(projectId);
    
    // Add to front
    recent.prepend(projectId);
    
    // Limit to 20 recent projects
    while (recent.size() > 20) {
        recent.removeLast();
    }
    
    settings_->setStringList(KEY_RECENT_PROJECTS, recent);
}

void ApplicationStateManager::clearRecentProjects() {
    settings_->remove(KEY_RECENT_PROJECTS);
}

bool ApplicationStateManager::getWindowGeometry(int& x, int& y, int& width, int& height) const {
    if (!settings_->contains(KEY_WINDOW_X) || !settings_->contains(KEY_WINDOW_Y) ||
        !settings_->contains(KEY_WINDOW_WIDTH) || !settings_->contains(KEY_WINDOW_HEIGHT)) {
        return false;
    }
    
    x = settings_->getInt(KEY_WINDOW_X, 100);
    y = settings_->getInt(KEY_WINDOW_Y, 100);
    width = settings_->getInt(KEY_WINDOW_WIDTH, 1200);
    height = settings_->getInt(KEY_WINDOW_HEIGHT, 800);
    
    return true;
}

void ApplicationStateManager::setWindowGeometry(int x, int y, int width, int height) {
    settings_->setValue(KEY_WINDOW_X, x);
    settings_->setValue(KEY_WINDOW_Y, y);
    settings_->setValue(KEY_WINDOW_WIDTH, width);
    settings_->setValue(KEY_WINDOW_HEIGHT, height);
}

bool ApplicationStateManager::isFirstRun() const {
    return settings_->getBool(KEY_FIRST_RUN, true);
}

void ApplicationStateManager::setFirstRunComplete() {
    settings_->setValue(KEY_FIRST_RUN, false);
    settings_->sync();
}

bool ApplicationStateManager::validateAndRecover() {
    qDebug() << "[ApplicationStateManager] Validating state...";
    
    bool workspacesValid = validateWorkspaces();
    bool projectsValid = validateProjects();
    
    if (!workspacesValid || !projectsValid) {
        qWarning() << "[ApplicationStateManager] State validation issues detected";
        qWarning() << "  Workspaces valid:" << workspacesValid;
        qWarning() << "  Projects valid:" << projectsValid;
        
        // Attempt recovery
        int workspacesRecovered = recoverWorkspaces();
        int projectsRecovered = recoverProjects();
        
        qDebug() << "[ApplicationStateManager] Recovery complete:";
        qDebug() << "  Workspaces recovered:" << workspacesRecovered;
        qDebug() << "  Projects recovered:" << projectsRecovered;
    } else {
        qDebug() << "[ApplicationStateManager] State validation passed";
    }
    
    return true;
}

void ApplicationStateManager::clearAll() {
    qWarning() << "[ApplicationStateManager] Clearing all application state...";
    
    // Clear repositories
    workspaceRepo_->clear();
    projectRepo_->clear();
    
    // Clear settings
    settings_->clear();
    settings_->sync();
    
    // Reset auto-save to defaults
    autoSaveEnabled_ = true;
    autoSaveInterval_ = 60;
    autoSaveTimer_->stop();
    
    qDebug() << "[ApplicationStateManager] All state cleared";
}

void ApplicationStateManager::savePreferences() {
    // Preferences are saved via settings_ as they are set
    // This method is primarily for explicit sync operations
    settings_->sync();
}

void ApplicationStateManager::restorePreferences() {
    // Preferences are restored via settings_ as they are accessed
    // This method logs the restoration
    qDebug() << "[ApplicationStateManager] Preferences restored:";
    qDebug() << "  Last workspace:" << getLastWorkspaceId();
    qDebug() << "  Recent projects:" << getRecentProjects();
    qDebug() << "  Auto-save enabled:" << isAutoSaveEnabled();
    qDebug() << "  Auto-save interval:" << getAutoSaveInterval();
}

bool ApplicationStateManager::validateWorkspaces() {
    // Try to load all workspaces
    const auto workspaceIds = workspaceRepo_->getAllWorkspaceIds();
    
    int validCount = 0;
    int invalidCount = 0;
    
    for (const QString& id : workspaceIds) {
        auto workspace = workspaceRepo_->loadWorkspace(id);
        if (workspace) {
            validCount++;
        } else {
            invalidCount++;
            qWarning() << "[ApplicationStateManager] Invalid workspace:" << id;
        }
    }
    
    qDebug() << "[ApplicationStateManager] Workspace validation:" 
             << validCount << "valid," << invalidCount << "invalid";
    
    return invalidCount == 0;
}

bool ApplicationStateManager::validateProjects() {
    // Try to load all projects
    const auto projectIds = projectRepo_->getAllProjectIds();
    
    int validCount = 0;
    int invalidCount = 0;
    
    for (const QString& id : projectIds) {
        auto project = projectRepo_->loadProject(id);
        if (project) {
            validCount++;
        } else {
            invalidCount++;
            qWarning() << "[ApplicationStateManager] Invalid project:" << id;
        }
    }
    
    qDebug() << "[ApplicationStateManager] Project validation:" 
             << validCount << "valid," << invalidCount << "invalid";
    
    return invalidCount == 0;
}

int ApplicationStateManager::recoverWorkspaces() {
    // Corrupted workspaces are automatically deleted during loadWorkspace
    // This method counts how many were recovered (deleted)
    
    const auto workspaceIds = workspaceRepo_->getAllWorkspaceIds();
    int recoveredCount = 0;
    
    for (const QString& id : workspaceIds) {
        auto workspace = workspaceRepo_->loadWorkspace(id);
        if (!workspace) {
            // Workspace was corrupted and deleted
            recoveredCount++;
        }
    }
    
    return recoveredCount;
}

int ApplicationStateManager::recoverProjects() {
    // Corrupted projects are automatically deleted during loadProject
    // This method counts how many were recovered (deleted)
    
    const auto projectIds = projectRepo_->getAllProjectIds();
    int recoveredCount = 0;
    
    for (const QString& id : projectIds) {
        auto project = projectRepo_->loadProject(id);
        if (!project) {
            // Project was corrupted and deleted
            recoveredCount++;
        }
    }
    
    return recoveredCount;
}

void ApplicationStateManager::setupAutoSaveTimer() {
    // Connect timer to auto-save callback
    QObject::connect(autoSaveTimer_.get(), &QTimer::timeout, [this]() {
        onAutoSaveTimer();
    });
}

void ApplicationStateManager::onAutoSaveTimer() {
    qDebug() << "[ApplicationStateManager] Auto-save triggered";
    saveState();
}

// Factory function
std::unique_ptr<IApplicationStateManager> createApplicationStateManager(
    std::shared_ptr<ISettingsManager> settingsManager,
    std::shared_ptr<IWorkspaceRepository> workspaceRepo,
    std::shared_ptr<ProjectRepository> projectRepo)
{
    return std::make_unique<ApplicationStateManager>(
        std::move(settingsManager),
        std::move(workspaceRepo),
        std::move(projectRepo)
    );
}

} // namespace ZenRunner::Storage
