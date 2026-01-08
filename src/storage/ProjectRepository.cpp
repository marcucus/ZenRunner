#include "ProjectRepository.h"
#include <QDebug>
#include <QDirIterator>

namespace ZenRunner::Storage {

ProjectRepository::ProjectRepository(const QString& storageDir)
    : storageDir_(storageDir)
{
    if (storageDir_.isEmpty()) {
        // Use standard application data directory
        storageDir_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        storageDir_ = QDir(storageDir_).filePath("projects");
    }
    
    ensureStorageDirectoryExists();
}

bool ProjectRepository::saveProject(const Project& project) {
    if (!ensureStorageDirectoryExists()) [[unlikely]] {
        qWarning() << "Failed to create storage directory:" << storageDir_;
        return false;
    }
    
    const QString filePath = getProjectFilePath(project.id());
    
    // Use QSaveFile for atomic writes to prevent corruption
    QSaveFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) [[unlikely]] {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    QJsonObject json = projectToJson(project);
    QJsonDocument doc(json);
    
    const qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    
    if (bytesWritten == -1) [[unlikely]] {
        qWarning() << "Failed to write project data to file:" << filePath;
        file.cancelWriting();
        return false;
    }
    
    // Commit the atomic write
    if (!file.commit()) [[unlikely]] {
        qWarning() << "Failed to commit project file:" << filePath;
        return false;
    }
    
    // Invalidate path cache
    pathIndexValid_ = false;
    
    return true;
}

std::shared_ptr<Project> ProjectRepository::loadProject(const QString& projectId) {
    const QString filePath = getProjectFilePath(projectId);
    QFile file(filePath);
    
    if (!file.exists()) [[unlikely]] {
        return nullptr;
    }
    
    if (!file.open(QIODevice::ReadOnly)) [[unlikely]] {
        qWarning() << "Failed to open file for reading:" << filePath;
        return nullptr;
    }
    
    const QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
        qWarning() << "JSON parse error in project" << projectId << ":" << parseError.errorString();
        qWarning() << "Attempting recovery by deleting corrupted file:" << filePath;
        
        // Recovery: Delete corrupted file
        QFile::remove(filePath);
        return nullptr;
    }
    
    if (!doc.isObject()) [[unlikely]] {
        qWarning() << "Invalid project JSON format for" << projectId;
        return nullptr;
    }
    
    const QJsonObject json = doc.object();
    
    // Validate before deserializing
    if (!validateProjectJson(json)) [[unlikely]] {
        qWarning() << "Project JSON validation failed for" << projectId;
        return nullptr;
    }
    
    return projectFromJson(json, projectId);
}

std::shared_ptr<Project> ProjectRepository::loadProjectFromPath(const QString& projectPath) {
    // Check if path is already registered
    if (!pathIndexValid_) {
        rebuildPathIndex();
    }
    
    if (pathToIdCache_.contains(projectPath)) {
        return loadProject(pathToIdCache_[projectPath]);
    }
    
    // Path not registered, return nullptr
    return nullptr;
}

bool ProjectRepository::deleteProject(const QString& projectId) {
    const QString filePath = getProjectFilePath(projectId);
    QFile file(filePath);
    
    if (!file.exists()) [[unlikely]] {
        return false;
    }
    
    // Invalidate path cache
    pathIndexValid_ = false;
    
    return file.remove();
}

std::vector<QString> ProjectRepository::getAllProjectIds() const {
    std::vector<QString> ids;
    
    const QDir dir(storageDir_);
    if (!dir.exists()) [[unlikely]] {
        return ids;
    }
    
    const QStringList filters{"*.json"};
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    ids.reserve(files.size());
    for (const QFileInfo& fileInfo : files) {
        // Project ID is the filename without extension
        ids.push_back(fileInfo.baseName());
    }
    
    return ids;
}

std::vector<std::shared_ptr<Project>> ProjectRepository::getAllProjects() {
    std::vector<std::shared_ptr<Project>> projects;
    
    const auto ids = getAllProjectIds();
    projects.reserve(ids.size());
    
    for (const QString& id : ids) {
        auto project = loadProject(id);
        if (project) [[likely]] {
            projects.push_back(std::move(project));
        }
    }
    
    return projects;
}

bool ProjectRepository::projectExists(const QString& projectId) const {
    const QString filePath = getProjectFilePath(projectId);
    return QFile::exists(filePath);
}

bool ProjectRepository::isPathRegistered(const QString& projectPath) const {
    if (!pathIndexValid_) {
        const_cast<ProjectRepository*>(this)->rebuildPathIndex();
    }
    
    return pathToIdCache_.contains(projectPath);
}

bool ProjectRepository::updateProjectMetadata(
    const QString& projectId,
    const QString& /*name*/,
    const QStringList& pinnedScripts)
{
    auto project = loadProject(projectId);
    if (!project) [[unlikely]] {
        return false;
    }
    
    // Update pinned scripts - get current pinned scripts
    const auto currentPinned = project->getPinnedScripts();
    
    // First unpin all current pinned scripts
    for (const auto& script : currentPinned) {
        project->setScriptPinned(script.name, false);
    }
    
    // Then pin the new ones
    for (const QString& scriptName : pinnedScripts) {
        project->setScriptPinned(scriptName, true);
    }
    
    return saveProject(*project);
}

QString ProjectRepository::getStorageDirectory() const {
    return storageDir_;
}

void ProjectRepository::clear() {
    const QDir dir(storageDir_);
    if (!dir.exists()) [[unlikely]] {
        return;
    }
    
    const QStringList filters{"*.json"};
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QFile::remove(fileInfo.absoluteFilePath());
    }
    
    pathToIdCache_.clear();
    pathIndexValid_ = false;
}

QString ProjectRepository::getProjectFilePath(const QString& projectId) const {
    return QDir(storageDir_).filePath(projectId + ".json");
}

QJsonObject ProjectRepository::projectToJson(const Project& project) const {
    QJsonObject json;
    
    json["id"] = project.id();
    json["name"] = project.name();
    json["path"] = project.path();
    json["version"] = 1;  // Schema version for future compatibility
    
    // Serialize scripts
    QJsonArray scriptsArray;
    const auto& scripts = project.scripts();
    
    for (const auto& script : scripts) {
        QJsonObject scriptObj;
        scriptObj["name"] = script.name;
        scriptObj["command"] = script.command;
        scriptObj["isPinned"] = script.isPinned;
        
        scriptsArray.append(scriptObj);
    }
    
    json["scripts"] = scriptsArray;
    
    // Store pinned scripts list for quick access
    QJsonArray pinnedArray;
    const auto pinnedScripts = project.getPinnedScripts();
    for (const auto& script : pinnedScripts) {
        pinnedArray.append(script.name);
    }
    json["pinnedScripts"] = pinnedArray;
    
    return json;
}

std::shared_ptr<Project> ProjectRepository::projectFromJson(
    const QJsonObject& json,
    const QString& projectId) const
{
    const QString id = json["id"].toString();
    const QString name = json["name"].toString();
    const QString path = json["path"].toString();
    
    // Verify the project directory still exists
    QFileInfo dirInfo(path);
    if (!dirInfo.exists() || !dirInfo.isDir()) [[unlikely]] {
        qWarning() << "Project directory no longer exists:" << path;
        qWarning() << "Skipping project" << projectId;
        return nullptr;
    }
    
    // Try to reload project from disk (this will re-parse package.json)
    auto projectResult = Project::fromDirectory(path);
    
    if (!projectResult.isOk()) [[unlikely]] {
        qWarning() << "Failed to reload project from directory:" << path;
        qWarning() << "Error:" << projectResult.error();
        return nullptr;
    }
    
    auto project = std::make_shared<Project>(projectResult.value());
    
    // Restore pinned scripts from saved state
    const QJsonArray pinnedArray = json["pinnedScripts"].toArray();
    for (const QJsonValue& value : pinnedArray) {
        const QString scriptName = value.toString();
        project->setScriptPinned(scriptName, true);
    }
    
    // Note: We can't restore the original ID as Project generates a new UUID
    // The stored ID is for reference only
    
    return project;
}

bool ProjectRepository::validateProjectJson(const QJsonObject& json) const {
    // Check required fields
    if (!json.contains("id") || !json["id"].isString()) {
        qWarning() << "Missing or invalid 'id' field";
        return false;
    }
    
    if (!json.contains("name") || !json["name"].isString()) {
        qWarning() << "Missing or invalid 'name' field";
        return false;
    }
    
    if (!json.contains("path") || !json["path"].isString()) {
        qWarning() << "Missing or invalid 'path' field";
        return false;
    }
    
    // Check version (for future compatibility)
    if (json.contains("version")) {
        const int version = json["version"].toInt(-1);
        if (version < 0 || version > 1) {
            qWarning() << "Unsupported schema version:" << version;
            return false;
        }
    }
    
    return true;
}

bool ProjectRepository::ensureStorageDirectoryExists() const {
    const QDir dir(storageDir_);
    if (dir.exists()) [[likely]] {
        return true;
    }
    
    return dir.mkpath(".");
}

void ProjectRepository::rebuildPathIndex() {
    pathToIdCache_.clear();
    
    const auto ids = getAllProjectIds();
    for (const QString& id : ids) {
        const QString filePath = getProjectFilePath(id);
        QFile file(filePath);
        
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        
        const QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }
        
        const QJsonObject json = doc.object();
        const QString path = json["path"].toString();
        
        if (!path.isEmpty()) {
            pathToIdCache_[path] = id;
        }
    }
    
    pathIndexValid_ = true;
}

} // namespace ZenRunner::Storage
