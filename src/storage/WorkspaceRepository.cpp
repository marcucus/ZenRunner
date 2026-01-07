#include "WorkspaceRepository.h"
#include <QDebug>

namespace ZenRunner::Storage {

WorkspaceRepository::WorkspaceRepository(const QString& storageDir)
    : storageDir_(storageDir)
{
    if (storageDir_.isEmpty()) {
        // Use standard application data directory
        storageDir_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        storageDir_ = QDir(storageDir_).filePath("workspaces");
    }
    
    ensureStorageDirectoryExists();
}

bool WorkspaceRepository::saveWorkspace(const Core::IWorkspace& workspace) {
    if (!ensureStorageDirectoryExists()) [[unlikely]] {
        qWarning() << "Failed to create storage directory:" << storageDir_;
        return false;
    }
    
    const QString filePath = getWorkspaceFilePath(workspace.getId());
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) [[unlikely]] {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    QJsonObject json = workspaceToJson(workspace);
    QJsonDocument doc(json);
    
    const qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    if (bytesWritten == -1) [[unlikely]] {
        qWarning() << "Failed to write workspace data to file:" << filePath;
        return false;
    }
    
    return true;
}

std::shared_ptr<Core::IWorkspace> WorkspaceRepository::loadWorkspace(const QString& workspaceId) const {
    const QString filePath = getWorkspaceFilePath(workspaceId);
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
        qWarning() << "JSON parse error:" << parseError.errorString();
        return nullptr;
    }
    
    if (!doc.isObject()) [[unlikely]] {
        qWarning() << "Invalid workspace JSON format";
        return nullptr;
    }
    
    return workspaceFromJson(doc.object());
}

bool WorkspaceRepository::deleteWorkspace(const QString& workspaceId) {
    const QString filePath = getWorkspaceFilePath(workspaceId);
    QFile file(filePath);
    
    if (!file.exists()) [[unlikely]] {
        return false;
    }
    
    return file.remove();
}

std::vector<QString> WorkspaceRepository::getAllWorkspaceIds() const {
    std::vector<QString> ids;
    
    const QDir dir(storageDir_);
    if (!dir.exists()) [[unlikely]] {
        return ids;
    }
    
    const QStringList filters{"*.json"};
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    ids.reserve(files.size());
    for (const QFileInfo& fileInfo : files) {
        // Workspace ID is the filename without extension
        ids.push_back(fileInfo.baseName());
    }
    
    return ids;
}

std::vector<std::shared_ptr<Core::IWorkspace>> WorkspaceRepository::getAllWorkspaces() {
    std::vector<std::shared_ptr<Core::IWorkspace>> workspaces;
    
    const auto ids = getAllWorkspaceIds();
    workspaces.reserve(ids.size());
    
    for (const QString& id : ids) {
        auto workspace = loadWorkspace(id);
        if (workspace) [[likely]] {
            workspaces.push_back(std::move(workspace));
        }
    }
    
    return workspaces;
}

bool WorkspaceRepository::workspaceExists(const QString& workspaceId) const {
    const QString filePath = getWorkspaceFilePath(workspaceId);
    return QFile::exists(filePath);
}

bool WorkspaceRepository::updateWorkspaceMetadata(
    const QString& workspaceId,
    const QString& name,
    const QString& description)
{
    auto workspace = loadWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        return false;
    }
    
    workspace->setName(name);
    workspace->setDescription(description);
    
    return saveWorkspace(*workspace);
}

bool WorkspaceRepository::addProjectToWorkspace(
    const QString& workspaceId,
    const QString& projectId)
{
    auto workspace = loadWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        return false;
    }
    
    // Note: We're storing project IDs, not full project objects
    // The actual project objects are loaded separately
    // This is a simplified implementation that saves the workspace
    
    return saveWorkspace(*workspace);
}

bool WorkspaceRepository::removeProjectFromWorkspace(
    const QString& workspaceId,
    const QString& projectId)
{
    auto workspace = loadWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        return false;
    }
    
    workspace->removeProject(projectId);
    
    return saveWorkspace(*workspace);
}

std::vector<QString> WorkspaceRepository::getWorkspaceProjectIds(const QString& workspaceId) const {
    auto workspace = loadWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        return {};
    }
    
    std::vector<QString> projectIds;
    const auto projects = workspace->getProjects();
    projectIds.reserve(projects.size());
    
    for (const auto& project : projects) {
        if (project) [[likely]] {
            projectIds.push_back(project->getId());
        }
    }
    
    return projectIds;
}

QString WorkspaceRepository::getStorageDirectory() const {
    return storageDir_;
}

void WorkspaceRepository::clear() {
    const QDir dir(storageDir_);
    if (!dir.exists()) [[unlikely]] {
        return;
    }
    
    const QStringList filters{"*.json"};
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QFile::remove(fileInfo.absoluteFilePath());
    }
}

QString WorkspaceRepository::getWorkspaceFilePath(const QString& workspaceId) const {
    return QDir(storageDir_).filePath(workspaceId + ".json");
}

QJsonObject WorkspaceRepository::workspaceToJson(const Core::IWorkspace& workspace) const {
    QJsonObject json;
    
    json["id"] = workspace.getId();
    json["name"] = workspace.getName();
    json["description"] = workspace.getDescription();
    json["version"] = 1;  // Schema version for future compatibility
    
    // Serialize projects
    QJsonArray projectsArray;
    const auto projects = workspace.getProjects();
    
    for (const auto& project : projects) {
        if (!project) [[unlikely]] {
            continue;
        }
        
        QJsonObject projectObj;
        projectObj["id"] = project->getId();
        projectObj["name"] = project->getName();
        projectObj["path"] = project->getPath();
        
        // Serialize pinned scripts
        QJsonArray pinnedScripts;
        for (const QString& scriptName : project->getPinnedScripts()) {
            pinnedScripts.append(scriptName);
        }
        projectObj["pinnedScripts"] = pinnedScripts;
        
        projectsArray.append(projectObj);
    }
    
    json["projects"] = projectsArray;
    
    return json;
}

std::shared_ptr<Core::IWorkspace> WorkspaceRepository::workspaceFromJson(const QJsonObject& json) const {
    const QString id = json["id"].toString();
    const QString name = json["name"].toString();
    const QString description = json["description"].toString();
    
    if (id.isEmpty() || name.isEmpty()) [[unlikely]] {
        qWarning() << "Invalid workspace JSON: missing id or name";
        return nullptr;
    }
    
    // Create workspace using factory function
    auto workspace = Core::createWorkspace(name, id);
    workspace->setDescription(description);
    
    // Load projects
    const QJsonArray projectsArray = json["projects"].toArray();
    
    // NOTE: Project loading is incomplete in current implementation
    // This is a known limitation that will be addressed when IProjectRepository is fully integrated
    // For now, workspaces will be created but won't contain their projects until
    // the project repository integration is completed
    // TODO: Load actual project objects from IProjectRepository
    
    for (const QJsonValue& projectValue : projectsArray) {
        if (!projectValue.isObject()) [[unlikely]] {
            continue;
        }
        
        const QJsonObject projectObj = projectValue.toObject();
        const QString projectId = projectObj["id"].toString();
        const QString projectPath = projectObj["path"].toString();
        
        // When IProjectRepository is available:
        // 1. Load project from repository by ID
        // 2. If not found, try to reload from path
        // 3. Add loaded project to workspace
        qDebug() << "TODO: Load project" << projectId << "from path" << projectPath;
    }
    
    return workspace;
}

bool WorkspaceRepository::ensureStorageDirectoryExists() const {
    const QDir dir(storageDir_);
    if (dir.exists()) [[likely]] {
        return true;
    }
    
    return dir.mkpath(".");
}

} // namespace ZenRunner::Storage
