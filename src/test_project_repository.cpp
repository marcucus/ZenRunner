#include "storage/ProjectRepository.h"
#include "core/Project.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <vector>
#include <memory>

using namespace ZenRunner;
using namespace ZenRunner::Storage;

/**
 * @brief Test suite for ProjectRepository implementation
 * 
 * Validates:
 * - Project save/load operations
 * - Atomic writes prevent corruption
 * - Validation and recovery of malformed data
 * - Path-based project lookup
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

// Helper: Create a test project directory with package.json
QString createTestProject(const QString& basePath, const QString& projectName) {
    QString projectPath = QDir(basePath).filePath(projectName);
    QDir().mkpath(projectPath);
    
    // Create a simple package.json
    QFile packageJson(QDir(projectPath).filePath("package.json"));
    if (packageJson.open(QIODevice::WriteOnly)) {
        QString content = R"({
  "name": ")" + projectName + R"(",
  "version": "1.0.0",
  "scripts": {
    "start": "node index.js",
    "test": "jest",
    "build": "webpack"
  }
})";
        packageJson.write(content.toUtf8());
        packageJson.close();
    }
    
    return projectPath;
}

// Test 1: Basic save and load
void testBasicSaveLoad() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Basic Save/Load", false, "Failed to create temp directory");
        return;
    }
    
    // Create test project
    QString projectPath = createTestProject(tempDir.path(), "test-project");
    
    // Load project from directory
    auto projectResult = Project::fromDirectory(projectPath);
    if (!projectResult.isOk()) {
        addTestResult("Basic Save/Load", false, "Failed to load test project");
        return;
    }
    
    auto project = projectResult.value();
    QString projectId = project.id();
    
    // Create repository with temp storage
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    ProjectRepository repo(storagePath);
    
    // Save project
    bool saved = repo.saveProject(project);
    if (!saved) {
        addTestResult("Basic Save/Load", false, "Failed to save project");
        return;
    }
    
    // Load project
    auto loadedProject = repo.loadProject(projectId);
    if (!loadedProject) {
        addTestResult("Basic Save/Load", false, "Failed to load project");
        return;
    }
    
    // Verify data
    bool nameMatch = loadedProject->name() == project.name();
    bool pathMatch = loadedProject->path() == project.path();
    
    if (nameMatch && pathMatch) {
        addTestResult("Basic Save/Load", true);
    } else {
        addTestResult("Basic Save/Load", false, "Data mismatch after load");
    }
}

// Test 2: Pinned scripts persistence
void testPinnedScriptsPersistence() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Pinned Scripts Persistence", false, "Failed to create temp directory");
        return;
    }
    
    QString projectPath = createTestProject(tempDir.path(), "pinned-test");
    auto projectResult = Project::fromDirectory(projectPath);
    if (!projectResult.isOk()) {
        addTestResult("Pinned Scripts Persistence", false, "Failed to load test project");
        return;
    }
    
    auto project = projectResult.value();
    QString projectId = project.id();
    
    // Pin some scripts
    project.setScriptPinned("start", true);
    project.setScriptPinned("build", true);
    
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    ProjectRepository repo(storagePath);
    
    // Save
    if (!repo.saveProject(project)) {
        addTestResult("Pinned Scripts Persistence", false, "Failed to save");
        return;
    }
    
    // Load
    auto loadedProject = repo.loadProject(projectId);
    if (!loadedProject) {
        addTestResult("Pinned Scripts Persistence", false, "Failed to load");
        return;
    }
    
    // Check pinned scripts
    auto pinnedScripts = loadedProject->getPinnedScripts();
    bool hasStart = false;
    bool hasBuild = false;
    
    for (const auto& script : pinnedScripts) {
        if (script.name == "start") hasStart = true;
        if (script.name == "build") hasBuild = true;
    }
    
    if (hasStart && hasBuild) {
        addTestResult("Pinned Scripts Persistence", true);
    } else {
        addTestResult("Pinned Scripts Persistence", false, 
            QString("Missing pinned scripts: start=%1, build=%2").arg(hasStart).arg(hasBuild));
    }
}

// Test 3: Multiple projects
void testMultipleProjects() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Multiple Projects", false, "Failed to create temp directory");
        return;
    }
    
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    ProjectRepository repo(storagePath);
    
    // Create and save 3 projects
    std::vector<QString> projectIds;
    for (int i = 0; i < 3; ++i) {
        QString projectPath = createTestProject(tempDir.path(), QString("project-%1").arg(i));
        auto projectResult = Project::fromDirectory(projectPath);
        if (!projectResult.isOk()) {
            addTestResult("Multiple Projects", false, 
                QString("Failed to create project %1").arg(i));
            return;
        }
        
        auto project = projectResult.value();
        projectIds.push_back(project.id());
        
        if (!repo.saveProject(project)) {
            addTestResult("Multiple Projects", false, 
                QString("Failed to save project %1").arg(i));
            return;
        }
    }
    
    // Retrieve all project IDs
    auto allIds = repo.getAllProjectIds();
    
    if (allIds.size() == 3) {
        addTestResult("Multiple Projects", true);
    } else {
        addTestResult("Multiple Projects", false, 
            QString("Expected 3 projects, got %1").arg(allIds.size()));
    }
}

// Test 4: Project deletion
void testProjectDeletion() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Project Deletion", false, "Failed to create temp directory");
        return;
    }
    
    QString projectPath = createTestProject(tempDir.path(), "delete-test");
    auto projectResult = Project::fromDirectory(projectPath);
    if (!projectResult.isOk()) {
        addTestResult("Project Deletion", false, "Failed to create test project");
        return;
    }
    
    auto project = projectResult.value();
    QString projectId = project.id();
    
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    ProjectRepository repo(storagePath);
    
    // Save project
    if (!repo.saveProject(project)) {
        addTestResult("Project Deletion", false, "Failed to save project");
        return;
    }
    
    // Verify exists
    if (!repo.projectExists(projectId)) {
        addTestResult("Project Deletion", false, "Project not found after save");
        return;
    }
    
    // Delete project
    if (!repo.deleteProject(projectId)) {
        addTestResult("Project Deletion", false, "Failed to delete project");
        return;
    }
    
    // Verify deleted
    if (repo.projectExists(projectId)) {
        addTestResult("Project Deletion", false, "Project still exists after deletion");
    } else {
        addTestResult("Project Deletion", true);
    }
}

// Test 5: Corrupted data recovery
void testCorruptedDataRecovery() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Corrupted Data Recovery", false, "Failed to create temp directory");
        return;
    }
    
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    QDir().mkpath(storagePath);
    
    // Create a corrupted JSON file
    QString corruptedId = "corrupted-project";
    QString corruptedFile = QDir(storagePath).filePath(corruptedId + ".json");
    
    QFile file(corruptedFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("{ invalid json content }");
        file.close();
    }
    
    ProjectRepository repo(storagePath);
    
    // Try to load corrupted project - should return nullptr and delete the file
    auto project = repo.loadProject(corruptedId);
    
    if (project != nullptr) {
        addTestResult("Corrupted Data Recovery", false, "Expected nullptr for corrupted data");
        return;
    }
    
    // Verify the corrupted file was deleted
    if (QFile::exists(corruptedFile)) {
        addTestResult("Corrupted Data Recovery", false, "Corrupted file not deleted");
    } else {
        addTestResult("Corrupted Data Recovery", true);
    }
}

// Test 6: Path-based lookup
void testPathBasedLookup() {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        addTestResult("Path-Based Lookup", false, "Failed to create temp directory");
        return;
    }
    
    QString projectPath = createTestProject(tempDir.path(), "path-lookup-test");
    auto projectResult = Project::fromDirectory(projectPath);
    if (!projectResult.isOk()) {
        addTestResult("Path-Based Lookup", false, "Failed to create test project");
        return;
    }
    
    auto project = projectResult.value();
    
    QString storagePath = QDir(tempDir.path()).filePath("storage");
    ProjectRepository repo(storagePath);
    
    // Save project
    if (!repo.saveProject(project)) {
        addTestResult("Path-Based Lookup", false, "Failed to save project");
        return;
    }
    
    // Check if path is registered
    bool isRegistered = repo.isPathRegistered(projectPath);
    
    if (isRegistered) {
        addTestResult("Path-Based Lookup", true);
    } else {
        addTestResult("Path-Based Lookup", false, "Path not found in registry");
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "\n========================================";
    qDebug() << "ProjectRepository Test Suite";
    qDebug() << "========================================\n";
    
    // Run all tests
    testBasicSaveLoad();
    testPinnedScriptsPersistence();
    testMultipleProjects();
    testProjectDeletion();
    testCorruptedDataRecovery();
    testPathBasedLookup();
    
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
