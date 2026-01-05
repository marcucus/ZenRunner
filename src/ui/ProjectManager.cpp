#include "ProjectManager.h"
#include <QDebug>

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
    qDebug() << "Scanning folder:" << folderPath << "with max depth:" << maxDepth;

    // Clear existing projects
    beginResetModel();
    projects_.clear();
    endResetModel();
    emit projectCountChanged();

    // Scan for project paths
    auto projectPaths = ProjectScanner::scanDirectory(folderPath, maxDepth);
    
    if (projectPaths.empty()) {
        qDebug() << "No projects found in" << folderPath;
        emit scanComplete(true, QString("No projects found in %1").arg(folderPath));
        return;
    }

    qDebug() << "Found" << projectPaths.size() << "potential projects";

    // Load each project
    beginInsertRows(QModelIndex(), 0, static_cast<int>(projectPaths.size()) - 1);
    
    for (const auto& path : projectPaths) {
        auto result = Project::fromDirectory(path);
        if (result.isOk()) {
            projects_.push_back(std::move(result.value()));
            qDebug() << "Loaded project:" << projects_.back().name() 
                     << "with" << projects_.back().scripts().size() << "scripts";
        } else {
            qDebug() << "Failed to load project from" << path << ":" << result.error();
        }
    }
    
    endInsertRows();
    
    emit projectCountChanged();
    emit projectsDetected(static_cast<int>(projects_.size()));
    emit scanComplete(true, QString("Found %1 projects").arg(projects_.size()));
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

const ZenRunner::Project* ProjectManager::getProject(int index) const {
    if (index < 0 || index >= static_cast<int>(projects_.size())) {
        return nullptr;
    }
    return &projects_[index];
}

} // namespace ZenRunner::UI
