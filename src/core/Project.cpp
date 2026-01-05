#include "core/Project.h"
#include <QJsonArray>
#include <QFileInfo>
#include <QUuid>
#include <QDirIterator>

namespace ZenRunner {

// ============================================================================
// Script Implementation
// ============================================================================

QJsonObject Script::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["command"] = command;
    json["isPinned"] = isPinned;
    json["priority"] = static_cast<int>(priority);
    return json;
}

Script Script::fromJson(const QJsonObject& json) {
    Script script;
    script.name = json["name"].toString();
    script.command = json["command"].toString();
    script.isPinned = json["isPinned"].toBool();
    script.priority = static_cast<ProcessPriority>(
        json["priority"].toInt(static_cast<int>(ProcessPriority::Normal))
    );
    return script;
}

// ============================================================================
// Project Implementation
// ============================================================================

Project::Project(QString path)
    : path_(std::move(path))
{
    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Result<Project> Project::fromDirectory(const QString& path) {
    QFileInfo dirInfo(path);
    
    if (!dirInfo.exists() || !dirInfo.isDir()) [[unlikely]] {
        return Result<Project>::Err(
            QString("Directory does not exist: %1").arg(path)
        );
    }
    
    // Check for package.json
    const QString packageJsonPath = QDir(path).filePath("package.json");
    QFile packageJsonFile(packageJsonPath);
    
    if (!packageJsonFile.exists()) [[unlikely]] {
        return Result<Project>::Err(
            QString("No package.json found in: %1").arg(path)
        );
    }
    
    if (!packageJsonFile.open(QIODevice::ReadOnly)) [[unlikely]] {
        return Result<Project>::Err(
            QString("Failed to open package.json: %1").arg(packageJsonPath)
        );
    }
    
    // Parse JSON
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(
        packageJsonFile.readAll(), 
        &parseError
    );
    packageJsonFile.close();
    
    if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
        return Result<Project>::Err(
            QString("JSON parse error in %1: %2")
                .arg(packageJsonPath)
                .arg(parseError.errorString())
        );
    }
    
    if (!doc.isObject()) [[unlikely]] {
        return Result<Project>::Err(
            QString("Invalid package.json format in: %1").arg(packageJsonPath)
        );
    }
    
    // Create project
    Project project(path);
    project.parsePackageJson(doc.object());
    project.packageManager_ = project.detectPackageManager();
    project.valid_ = true;
    
    return Result<Project>::Ok(std::move(project));
}

Result<bool> Project::reload() {
    auto result = fromDirectory(path_);
    
    if (result.isOk()) [[likely]] {
        *this = std::move(result.value());
        return Result<bool>::Ok(true);
    }
    
    return Result<bool>::Err(result.error());
}

std::optional<Script> Project::getScript(const QString& name) const {
    auto it = std::find_if(scripts_.begin(), scripts_.end(),
        [&name](const Script& s) { return s.name == name; }
    );
    
    if (it != scripts_.end()) [[likely]] {
        return *it;
    }
    
    return std::nullopt;
}

std::vector<Script> Project::getPinnedScripts() const {
    std::vector<Script> pinned;
    
    for (const auto& script : scripts_) {
        if (script.isPinned) {
            pinned.push_back(script);
        }
    }
    
    return pinned;
}

void Project::setScriptPinned(const QString& scriptName, bool pinned) {
    auto it = std::find_if(scripts_.begin(), scripts_.end(),
        [&scriptName](const Script& s) { return s.name == scriptName; }
    );
    
    if (it != scripts_.end()) [[likely]] {
        it->isPinned = pinned;
    }
}

void Project::setScriptPriority(const QString& scriptName, ProcessPriority priority) {
    auto it = std::find_if(scripts_.begin(), scripts_.end(),
        [&scriptName](const Script& s) { return s.name == scriptName; }
    );
    
    if (it != scripts_.end()) [[likely]] {
        it->priority = priority;
    }
}

std::optional<ProcessConfig> Project::createProcessConfig(
    const QString& scriptName
) const {
    auto scriptOpt = getScript(scriptName);
    
    if (!scriptOpt.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    
    const Script& script = scriptOpt.value();
    
    ProcessConfig config;
    config.workingDirectory = path_;
    
    // Determine command based on package manager
    if (packageManager_ == "yarn") {
        config.command = "yarn";
        config.arguments = QStringList{scriptName};
    } else if (packageManager_ == "pnpm") {
        config.command = "pnpm";
        config.arguments = QStringList{"run", scriptName};
    } else {  // npm or default
        config.command = "npm";
        config.arguments = QStringList{"run", scriptName};
    }
    
    config.priority = script.priority;
    config.captureOutput = true;
    config.mergeChannels = false;
    
    return config;
}

QJsonObject Project::toJson() const {
    QJsonObject json;
    json["id"] = id_;
    json["name"] = name_;
    json["path"] = path_;
    json["description"] = description_;
    json["packageManager"] = packageManager_;
    
    QJsonArray scriptsArray;
    for (const auto& script : scripts_) {
        scriptsArray.append(script.toJson());
    }
    json["scripts"] = scriptsArray;
    
    return json;
}

Project Project::fromJson(const QJsonObject& json) {
    Project project;
    project.id_ = json["id"].toString();
    project.name_ = json["name"].toString();
    project.path_ = json["path"].toString();
    project.description_ = json["description"].toString();
    project.packageManager_ = json["packageManager"].toString("npm");
    
    const QJsonArray scriptsArray = json["scripts"].toArray();
    for (const auto& scriptValue : scriptsArray) {
        if (scriptValue.isObject()) {
            project.scripts_.push_back(Script::fromJson(scriptValue.toObject()));
        }
    }
    
    project.valid_ = !project.id_.isEmpty() && !project.path_.isEmpty();
    
    return project;
}

void Project::parsePackageJson(const QJsonObject& packageJson) {
    // Extract basic info
    name_ = packageJson["name"].toString();
    description_ = packageJson["description"].toString();
    
    // If name is empty, use directory name
    if (name_.isEmpty()) [[unlikely]] {
        name_ = QFileInfo(path_).fileName();
    }
    
    // Extract scripts
    const QJsonObject scriptsObj = packageJson["scripts"].toObject();
    
    for (auto it = scriptsObj.begin(); it != scriptsObj.end(); ++it) {
        Script script;
        script.name = it.key();
        script.command = it.value().toString();
        script.isPinned = false;
        script.priority = ProcessPriority::Normal;
        
        scripts_.push_back(std::move(script));
    }
}

QString Project::detectPackageManager() const {
    const QDir projectDir(path_);
    
    // Check for lock files to determine package manager
    if (projectDir.exists("pnpm-lock.yaml")) [[unlikely]] {
        return "pnpm";
    } else if (projectDir.exists("yarn.lock")) {
        return "yarn";
    } else [[likely]] {
        // Default to npm (package-lock.json or no lock file)
        return "npm";
    }
}

// ============================================================================
// Workspace Implementation
// ============================================================================

Workspace::Workspace(QString name)
    : name_(std::move(name))
{
    id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Workspace::addProject(const QString& projectId) {
    if (!containsProject(projectId)) [[likely]] {
        projectIds_.push_back(projectId);
    }
}

void Workspace::removeProject(const QString& projectId) {
    auto it = std::find(projectIds_.begin(), projectIds_.end(), projectId);
    
    if (it != projectIds_.end()) [[likely]] {
        projectIds_.erase(it);
    }
}

bool Workspace::containsProject(const QString& projectId) const {
    return std::find(projectIds_.begin(), projectIds_.end(), projectId) 
           != projectIds_.end();
}

QJsonObject Workspace::toJson() const {
    QJsonObject json;
    json["id"] = id_;
    json["name"] = name_;
    json["description"] = description_;
    
    QJsonArray projectsArray;
    for (const auto& projectId : projectIds_) {
        projectsArray.append(projectId);
    }
    json["projects"] = projectsArray;
    
    return json;
}

Workspace Workspace::fromJson(const QJsonObject& json) {
    Workspace workspace;
    workspace.id_ = json["id"].toString();
    workspace.name_ = json["name"].toString();
    workspace.description_ = json["description"].toString();
    
    const QJsonArray projectsArray = json["projects"].toArray();
    for (const auto& projectValue : projectsArray) {
        if (projectValue.isString()) {
            workspace.projectIds_.push_back(projectValue.toString());
        }
    }
    
    return workspace;
}

// ============================================================================
// ProjectScanner Implementation
// ============================================================================

std::vector<QString> ProjectScanner::scanDirectory(
    const QString& rootPath,
    int maxDepth
) {
    std::vector<QString> results;
    
    QFileInfo rootInfo(rootPath);
    if (!rootInfo.exists() || !rootInfo.isDir()) [[unlikely]] {
        return results;
    }
    
    scanDirectoryRecursive(rootPath, 0, maxDepth, results);
    
    return results;
}

bool ProjectScanner::isProjectDirectory(const QString& path) {
    const QDir dir(path);
    return dir.exists("package.json");
}

std::vector<Project> ProjectScanner::loadProjects(const QStringList& paths) {
    std::vector<Project> projects;
    projects.reserve(paths.size());
    
    for (const QString& path : paths) {
        auto result = Project::fromDirectory(path);
        
        if (result.isOk()) [[likely]] {
            projects.push_back(std::move(result.value()));
        }
    }
    
    return projects;
}

void ProjectScanner::scanDirectoryRecursive(
    const QString& path,
    int currentDepth,
    int maxDepth,
    std::vector<QString>& results
) {
    if (currentDepth > maxDepth) [[unlikely]] {
        return;
    }
    
    // Check if current directory is a project
    if (isProjectDirectory(path)) [[unlikely]] {
        results.push_back(path);
        // Don't scan subdirectories of projects
        return;
    }
    
    // Scan subdirectories
    QDir dir(path);
    const QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable,
        QDir::Name
    );
    
    for (const QFileInfo& entry : entries) {
        // Skip common directories that shouldn't contain projects
        const QString dirName = entry.fileName();
        if (dirName == "node_modules" || 
            dirName == ".git" || 
            dirName == "dist" ||
            dirName == "build" ||
            dirName.startsWith('.')) [[unlikely]] {
            continue;
        }
        
        scanDirectoryRecursive(
            entry.absoluteFilePath(),
            currentDepth + 1,
            maxDepth,
            results
        );
    }
}

} // namespace ZenRunner
