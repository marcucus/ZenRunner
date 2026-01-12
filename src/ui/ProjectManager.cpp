#include "ui/ProjectManager.h"
#include "core/Project.h"
#include <QDebug>
#include <QUrl>

namespace ZenRunner::UI {

ProjectManager::ProjectManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ProjectManager::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(projects_.size());
}

QVariant ProjectManager::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(projects_.size())) {
        return QVariant();
    }

    const auto& project = projects_[index.row()];

    switch (role) {
    case NameRole:
        return project.name();
    case PathRole:
        return project.path();
    case ScriptCountRole:
        return static_cast<int>(project.scripts().size());
    case ScriptsRole: {
        QVariantList scriptList;
        for (const auto& script : project.scripts()) {
            QVariantMap scriptMap;
            scriptMap["name"] = script.name;
            scriptMap["command"] = script.command;
            scriptMap["isPinned"] = script.isPinned;
            scriptList.append(scriptMap);
        }
        return scriptList;
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ProjectManager::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[ScriptCountRole] = "scriptCount";
    roles[ScriptsRole] = "scripts";
    return roles;
}

void ProjectManager::scanFolder(const QString& folderPath, int maxDepth) {
    // Convert URL to local path if needed (QML FolderDialog returns file:// URLs)
    QString localPath = folderPath;
    if (folderPath.startsWith("file://")) {
        localPath = QUrl(folderPath).toLocalFile();
    }
    
    qDebug() << "Scanning folder:" << localPath << "with max depth:" << maxDepth;

    // Clear existing projects
    beginResetModel();
    projects_.clear();
    endResetModel();
    emit projectCountChanged();

    // Scan for project paths
    auto projectPaths = ProjectScanner::scanDirectory(localPath, maxDepth);
    
    if (projectPaths.empty()) {
        qDebug() << "No projects found in" << localPath;
        emit scanComplete(true, QString("No projects found in %1").arg(localPath));
        return;
    }

    qDebug() << "Found" << projectPaths.size() << "potential projects";

    // Load each project first to know how many succeeded
    std::vector<Project> loadedProjects;
    for (const auto& path : projectPaths) {
        auto result = Project::fromDirectory(path);
        if (result.isOk()) {
            loadedProjects.push_back(std::move(result.value()));
            qDebug() << "Loaded project:" << loadedProjects.back().name() 
                     << "with" << loadedProjects.back().scripts().size() << "scripts";
        } else {
            qDebug() << "Failed to load project from" << path << ":" << result.error();
        }
    }
    
    // Now insert the successfully loaded projects
    if (!loadedProjects.empty()) {
        beginInsertRows(QModelIndex(), 0, static_cast<int>(loadedProjects.size()) - 1);
        for (auto& project : loadedProjects) {
            projects_.push_back(std::move(project));
        }
        endInsertRows();
    }
    
    emit projectCountChanged();
    emit projectsDetected(static_cast<int>(projects_.size()));
    emit scanComplete(true, QString("Found %1 projects").arg(projects_.size()));
}

void ProjectManager::addProject(const QString& folderPath) {
    // Convert URL to local path if needed (QML FolderDialog returns file:// URLs)
    QString localPath = folderPath;
    if (folderPath.startsWith("file://")) {
        localPath = QUrl(folderPath).toLocalFile();
    }
    
    qDebug() << "Adding project from folder:" << localPath;
    
    // Load the project
    auto result = Project::fromDirectory(localPath);
    if (!result.isOk()) {
        qWarning() << "Failed to load project from" << localPath << ":" << result.error();
        emit scanComplete(false, QString("Failed to load project: %1").arg(result.error()));
        return;
    }
    
    auto project = std::move(result.value());
    qDebug() << "Loaded project:" << project.name() 
             << "with" << project.scripts().size() << "scripts";
    
    // Check if project already exists
    for (const auto& existingProject : projects_) {
        if (existingProject.path() == project.path()) {
            qWarning() << "Project already exists:" << project.path();
            emit scanComplete(false, "Project already exists");
            return;
        }
    }
    
    // Add the project
    const int newRow = static_cast<int>(projects_.size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    projects_.push_back(std::move(project));
    endInsertRows();
    
    emit projectCountChanged();
    emit projectsDetected(static_cast<int>(projects_.size()));
    emit scanComplete(true, "Project added successfully");
}

void ProjectManager::clearProjects() {
    if (projects_.empty()) {
        return;
    }

    beginResetModel();
    projects_.clear();
    endResetModel();
    
    emit projectCountChanged();
}

QVariantMap ProjectManager::getProject(int index) const {
    QVariantMap result;
    
    if (index < 0 || index >= static_cast<int>(projects_.size())) {
        return result;
    }
    
    const auto& project = projects_[index];
    result["id"] = project.id();
    result["name"] = project.name();
    result["path"] = project.path();
    result["scriptCount"] = static_cast<int>(project.scripts().size());
    
    QVariantList scripts;
    for (const auto& script : project.scripts()) {
        QVariantMap scriptMap;
        scriptMap["name"] = script.name;
        scriptMap["command"] = script.command;
        scriptMap["isPinned"] = script.isPinned;
        scripts.append(scriptMap);
    }
    result["scripts"] = scripts;
    
    return result;
}

} // namespace ZenRunner::UI
