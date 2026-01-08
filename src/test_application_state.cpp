#include "storage/ApplicationStateManager.h"
#include "storage/SettingsManager.h"
#include "storage/WorkspaceRepository.h"
#include "storage/ProjectRepository.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <QThread>
#include <vector>

using namespace ZenRunner::Storage;

/**
 * @brief Test suite for ApplicationStateManager
 * 
 * Validates:
 * - Initialization and state restoration
 * - Auto-save functionality
 * - Recent projects tracking
 * - Window geometry persistence
 * - First run detection
 */

class TestResult {
public:
    bool passed{false};
    QString name;
    QString message;
    
    void print() const {
        if (passed) {
            qDebug() << "✓" << name;
        } else {
            qDebug() << "✗" << name << ":" << message;
        }
    }
};

std::vector<TestResult> testResults;

void addTestResult(const QString& name, bool passed, const QString& message = "") {
    TestResult result;
    result.name = name;
    result.passed = passed;
    result.message = message;
    testResults.push_back(result);
    result.print();
}

// Test 1: Initialization
void testInitialization() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Initialization", false, "Failed to create temp directory");
        return;
    }
    
    // Create components
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    // Create state manager
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager) {
        addTestResult("Initialization", false, "Failed to create state manager");
        return;
    }
    
    bool initialized = stateManager->initialize();
    
    if (initialized) {
        addTestResult("Initialization", true);
    } else {
        addTestResult("Initialization", false, "Initialization failed");
    }
}

// Test 2: Recent projects tracking
void testRecentProjects() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Recent Projects", false, "Failed to create temp directory");
        return;
    }
    
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        addTestResult("Recent Projects", false, "Failed to initialize");
        return;
    }
    
    // Add some recent projects
    stateManager->addRecentProject("project1");
    stateManager->addRecentProject("project2");
    stateManager->addRecentProject("project3");
    
    // Get recent projects
    auto recent = stateManager->getRecentProjects(10);
    
    if (recent.size() != 3) {
        addTestResult("Recent Projects", false, 
            QString("Expected 3 projects, got %1").arg(recent.size()));
        return;
    }
    
    // Verify order (most recent first)
    if (recent[0] == "project3" && recent[1] == "project2" && recent[2] == "project1") {
        addTestResult("Recent Projects", true);
    } else {
        addTestResult("Recent Projects", false, "Incorrect order");
    }
}

// Test 3: Window geometry persistence
void testWindowGeometry() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Window Geometry", false, "Failed to create temp directory");
        return;
    }
    
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        addTestResult("Window Geometry", false, "Failed to initialize");
        return;
    }
    
    // Set geometry
    stateManager->setWindowGeometry(100, 200, 1280, 720);
    stateManager->saveState();
    
    // Read back geometry
    int x, y, width, height;
    bool hasGeometry = stateManager->getWindowGeometry(x, y, width, height);
    
    if (!hasGeometry) {
        addTestResult("Window Geometry", false, "No geometry data");
        return;
    }
    
    if (x == 100 && y == 200 && width == 1280 && height == 720) {
        addTestResult("Window Geometry", true);
    } else {
        addTestResult("Window Geometry", false, 
            QString("Mismatch: (%1,%2,%3,%4)").arg(x).arg(y).arg(width).arg(height));
    }
}

// Test 4: First run detection
void testFirstRun() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("First Run Detection", false, "Failed to create temp directory");
        return;
    }
    
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        addTestResult("First Run Detection", false, "Failed to initialize");
        return;
    }
    
    // Should be first run
    bool isFirst = stateManager->isFirstRun();
    
    if (!isFirst) {
        addTestResult("First Run Detection", false, "Expected first run");
        return;
    }
    
    // Mark as complete
    stateManager->setFirstRunComplete();
    
    // Should not be first run anymore
    bool isStillFirst = stateManager->isFirstRun();
    
    if (isStillFirst) {
        addTestResult("First Run Detection", false, "Still marked as first run");
    } else {
        addTestResult("First Run Detection", true);
    }
}

// Test 5: Auto-save configuration
void testAutoSaveConfig() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Auto-Save Configuration", false, "Failed to create temp directory");
        return;
    }
    
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        addTestResult("Auto-Save Configuration", false, "Failed to initialize");
        return;
    }
    
    // Auto-save should be enabled by default
    if (!stateManager->isAutoSaveEnabled()) {
        addTestResult("Auto-Save Configuration", false, "Auto-save not enabled by default");
        return;
    }
    
    // Change interval
    stateManager->setAutoSaveEnabled(true, 120);
    
    int interval = stateManager->getAutoSaveInterval();
    if (interval != 120) {
        addTestResult("Auto-Save Configuration", false, 
            QString("Expected interval 120, got %1").arg(interval));
        return;
    }
    
    // Disable auto-save
    stateManager->setAutoSaveEnabled(false);
    
    if (stateManager->isAutoSaveEnabled()) {
        addTestResult("Auto-Save Configuration", false, "Auto-save still enabled");
    } else {
        addTestResult("Auto-Save Configuration", true);
    }
}

// Test 6: Last workspace tracking
void testLastWorkspace() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Last Workspace", false, "Failed to create temp directory");
        return;
    }
    
    auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
    auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
    auto workspaceRepo = std::make_shared<WorkspaceRepository>(
        QDir(tempDir.path()).filePath("workspaces"));
    auto projectRepo = std::make_shared<ProjectRepository>(
        QDir(tempDir.path()).filePath("projects"));
    
    auto stateManager = createApplicationStateManager(
        settings, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        addTestResult("Last Workspace", false, "Failed to initialize");
        return;
    }
    
    // Set last workspace
    QString workspaceId = "workspace-123";
    stateManager->setLastWorkspaceId(workspaceId);
    stateManager->saveState();
    
    // Get last workspace
    QString retrievedId = stateManager->getLastWorkspaceId();
    
    if (retrievedId == workspaceId) {
        addTestResult("Last Workspace", true);
    } else {
        addTestResult("Last Workspace", false, 
            QString("Expected '%1', got '%2'").arg(workspaceId).arg(retrievedId));
    }
}

// Test 7: State save and restore
void testSaveAndRestore() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Save and Restore", false, "Failed to create temp directory");
        return;
    }
    
    QString settingsPath = QDir(tempDir.path()).filePath("settings");
    QString workspacesPath = QDir(tempDir.path()).filePath("workspaces");
    QString projectsPath = QDir(tempDir.path()).filePath("projects");
    
    // First session
    {
        auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
        auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
        auto workspaceRepo = std::make_shared<WorkspaceRepository>(workspacesPath);
        auto projectRepo = std::make_shared<ProjectRepository>(projectsPath);
        
        auto stateManager = createApplicationStateManager(
            settings, workspaceRepo, projectRepo);
        
        stateManager->initialize();
        
        // Set some state
        stateManager->addRecentProject("project-abc");
        stateManager->setLastWorkspaceId("workspace-xyz");
        stateManager->setWindowGeometry(50, 100, 1920, 1080);
        
        // Save
        stateManager->saveState();
    }
    
    // Second session - restore state
    {
        auto settingsUnique = createSettingsManager("TestOrg", "TestApp");
        auto settings = std::shared_ptr<ISettingsManager>(std::move(settingsUnique));
        auto workspaceRepo = std::make_shared<WorkspaceRepository>(workspacesPath);
        auto projectRepo = std::make_shared<ProjectRepository>(projectsPath);
        
        auto stateManager = createApplicationStateManager(
            settings, workspaceRepo, projectRepo);
        
        stateManager->initialize();
        
        // Verify restored state
        auto recent = stateManager->getRecentProjects(1);
        QString lastWorkspace = stateManager->getLastWorkspaceId();
        
        int x, y, width, height;
        stateManager->getWindowGeometry(x, y, width, height);
        
        bool recentOk = recent.size() == 1 && recent[0] == "project-abc";
        bool workspaceOk = lastWorkspace == "workspace-xyz";
        bool geometryOk = x == 50 && y == 100 && width == 1920 && height == 1080;
        
        if (recentOk && workspaceOk && geometryOk) {
            addTestResult("Save and Restore", true);
        } else {
            addTestResult("Save and Restore", false, 
                QString("State mismatch: recent=%1, workspace=%2, geometry=%3")
                    .arg(recentOk).arg(workspaceOk).arg(geometryOk));
        }
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "\n========================================";
    qDebug() << "ApplicationStateManager Test Suite";
    qDebug() << "========================================\n";
    
    // Run all tests
    testInitialization();
    testRecentProjects();
    testWindowGeometry();
    testFirstRun();
    testAutoSaveConfig();
    testLastWorkspace();
    testSaveAndRestore();
    
    // Print summary
    int passed = 0;
    int failed = 0;
    
    for (const auto& result : testResults) {
        if (result.passed) {
            passed++;
        } else {
            failed++;
        }
    }
    
    qDebug() << "\n========================================";
    qDebug() << "Test Summary:";
    qDebug() << "  Passed:" << passed;
    qDebug() << "  Failed:" << failed;
    qDebug() << "  Total: " << testResults.size();
    qDebug() << "========================================\n";
    
    return failed > 0 ? 1 : 0;
}
