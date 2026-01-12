#include "WorkspaceViewModel.h"
#include "core/Workspace.h"
#include "core/Project.h"
#include "storage/ProjectRepository.h"
#include <QDebug>
#include <QColor>

namespace ZenRunner::UI {

WorkspaceViewModel::WorkspaceViewModel(QObject* parent)
    : QAbstractListModel(parent)
    , repository_(nullptr)
    , projectRepository_(nullptr)
    , processManager_(nullptr)
{
}

void WorkspaceViewModel::setRepository(Storage::IWorkspaceRepository* repository) {
    repository_ = repository;
}

void WorkspaceViewModel::setProcessManager(Core::IProcessManager* manager) {
    processManager_ = manager;
}

void WorkspaceViewModel::setProjectRepository(Storage::ProjectRepository* repository) {
    projectRepository_ = repository;
}

int WorkspaceViewModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) [[unlikely]] {
        return 0;
    }
    return static_cast<int>(workspaces_.size());
}

QVariant WorkspaceViewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(workspaces_.size())) [[unlikely]] {
        return QVariant();
    }

    const auto& workspace = workspaces_[index.row()];
    if (!workspace) [[unlikely]] {
        return QVariant();
    }

    switch (role) {
        case IdRole:
            return workspace->getId();
        case NameRole:
            return workspace->getName();
        case DescriptionRole:
            return workspace->getDescription();
        case ProjectCountRole:
            return static_cast<int>(workspace->getProjectCount());
        case IsRunningRole:
            return workspace->isAnyProjectRunning();
        case ColorRole:
            return generateWorkspaceColor(workspace->getName());
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> WorkspaceViewModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "workspaceId";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[ProjectCountRole] = "projectCount";
    roles[IsRunningRole] = "isRunning";
    roles[ColorRole] = "color";
    return roles;
}

void WorkspaceViewModel::loadWorkspaces() {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return;
    }

    beginResetModel();
    workspaces_ = repository_->getAllWorkspaces();
    
    // Set process manager for each workspace
    if (processManager_) [[likely]] {
        for (auto& workspace : workspaces_) {
            if (workspace) [[likely]] {
                workspace->setProcessManager(processManager_);
            }
        }
    }
    
    endResetModel();
    emit countChanged();
}

QString WorkspaceViewModel::createWorkspace(const QString& name, const QString& description) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return QString();
    }

    if (name.isEmpty()) [[unlikely]] {
        emit errorOccurred("Workspace name cannot be empty");
        return QString();
    }

    // Create new workspace
    auto workspace = Core::createWorkspace(name);
    workspace->setDescription(description);
    
    // Set process manager if available
    if (processManager_) [[likely]] {
        workspace->setProcessManager(processManager_);
    }

    // Save to repository
    if (!repository_->saveWorkspace(*workspace)) [[unlikely]] {
        emit errorOccurred("Failed to save workspace");
        return QString();
    }

    QString workspaceId = workspace->getId();

    // Add to model
    const int newRow = static_cast<int>(workspaces_.size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    workspaces_.push_back(workspace);
    endInsertRows();

    emit countChanged();
    emit workspaceCreated(workspaceId);

    return workspaceId;
}

bool WorkspaceViewModel::deleteWorkspace(const QString& workspaceId) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    // Find workspace in model
    const int index = getWorkspaceIndex(workspaceId);
    if (index == -1) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return false;
    }

    // Stop all running processes in the workspace first
    if (workspaces_[index]->isAnyProjectRunning()) [[unlikely]] {
        workspaces_[index]->stopAll(true);
    }

    // Delete from repository
    if (!repository_->deleteWorkspace(workspaceId)) [[unlikely]] {
        emit errorOccurred("Failed to delete workspace from storage");
        return false;
    }

    // Remove from model
    beginRemoveRows(QModelIndex(), index, index);
    workspaces_.erase(workspaces_.begin() + index);
    endRemoveRows();

    emit countChanged();
    emit workspaceDeleted(workspaceId);

    return true;
}

bool WorkspaceViewModel::updateWorkspace(const QString& workspaceId, const QString& name, const QString& description) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    auto workspace = findWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return false;
    }

    workspace->setName(name);
    workspace->setDescription(description);

    if (!repository_->saveWorkspace(*workspace)) [[unlikely]] {
        emit errorOccurred("Failed to save workspace");
        return false;
    }

    // Notify model of changes
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1) [[likely]] {
        const QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
    }

    emit workspaceUpdated(workspaceId);

    return true;
}

bool WorkspaceViewModel::addProjectToWorkspace(const QString& workspaceId, const QString& projectId) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    auto workspace = findWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return false;
    }

    // Add project ID to workspace using the interface method
    workspace->addProjectId(projectId);

    // Save the updated workspace (pass the interface reference)
    if (!repository_->saveWorkspace(*workspace)) [[unlikely]] {
        emit errorOccurred("Failed to save workspace");
        return false;
    }

    // Notify model of changes
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1) [[likely]] {
        const QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
    }

    emit workspaceUpdated(workspaceId);

    return true;
}

bool WorkspaceViewModel::addScannedProjectToWorkspace(const QString& workspaceId, const QVariantMap& projectData) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    if (!projectRepository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No project repository set";
        emit errorOccurred("No project repository configured");
        return false;
    }

    // Extract project data from QVariantMap
    QString projectId = projectData.value("id").toString();
    QString projectName = projectData.value("name").toString();
    QString projectPath = projectData.value("path").toString();
    
    if (projectId.isEmpty() || projectPath.isEmpty()) [[unlikely]] {
        emit errorOccurred("Invalid project data: missing id or path");
        return false;
    }
    
    qDebug() << "Adding scanned project to workspace:" << projectName << "ID:" << projectId;
    
    // Create a Project object from the data
    auto projectResult = Project::fromDirectory(projectPath);
    if (!projectResult.isOk()) [[unlikely]] {
        qWarning() << "Failed to load project from directory:" << projectPath;
        emit errorOccurred(QString("Failed to load project: %1").arg(projectResult.error()));
        return false;
    }
    
    auto project = std::make_shared<Project>(std::move(projectResult.value()));
    
    // Save the project to the repository
    if (!projectRepository_->saveProject(*project)) [[unlikely]] {
        qWarning() << "Failed to save project to repository";
        emit errorOccurred("Failed to save project to repository");
        return false;
    }
    
    qDebug() << "Project saved to repository, now adding to workspace";
    
    // Now add it to the workspace using the existing method
    return addProjectToWorkspace(workspaceId, project->id());
}

bool WorkspaceViewModel::removeProjectFromWorkspace(const QString& workspaceId, const QString& projectId) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    auto workspace = findWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return false;
    }

    if (!workspace->removeProject(projectId)) [[unlikely]] {
        emit errorOccurred("Project not found in workspace");
        return false;
    }

    if (!repository_->saveWorkspace(*workspace)) [[unlikely]] {
        emit errorOccurred("Failed to save workspace");
        return false;
    }

    // Notify model of changes
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1) [[likely]] {
        const QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
    }

    emit workspaceUpdated(workspaceId);

    return true;
}

int WorkspaceViewModel::startAllProjects(const QString& workspaceId, const QString& scriptName, bool parallel) {
    auto workspace = findWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return 0;
    }

    const auto mode = parallel ? 
        Core::IWorkspace::ExecutionMode::Parallel : 
        Core::IWorkspace::ExecutionMode::Sequential;

    const int count = workspace->startAll(scriptName, mode);

    // Notify model of state change
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1) [[likely]] {
        const QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, {IsRunningRole});
    }

    return count;
}

int WorkspaceViewModel::stopAllProjects(const QString& workspaceId, bool forceKill) {
    auto workspace = findWorkspace(workspaceId);
    if (!workspace) [[unlikely]] {
        emit errorOccurred("Workspace not found");
        return 0;
    }

    const int count = workspace->stopAll(forceKill);

    // Notify model of state change
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1) [[likely]] {
        const QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, {IsRunningRole});
    }

    return count;
}

int WorkspaceViewModel::getWorkspaceIndex(const QString& workspaceId) const {
    for (size_t i = 0; i < workspaces_.size(); ++i) {
        if (workspaces_[i] && workspaces_[i]->getId() == workspaceId) [[likely]] {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::shared_ptr<Core::IWorkspace> WorkspaceViewModel::findWorkspace(const QString& workspaceId) const {
    const int index = getWorkspaceIndex(workspaceId);
    if (index != -1 && index < static_cast<int>(workspaces_.size())) [[likely]] {
        return workspaces_[index];
    }
    return nullptr;
}

QString WorkspaceViewModel::generateWorkspaceColor(const QString& name) const {
    // Generate a consistent color based on workspace name hash
    // This ensures the same name always gets the same color
    
    if (name.isEmpty()) [[unlikely]] {
        return "#6b7280";  // Gray for empty names
    }

    // Predefined color palette for better aesthetics
    const QStringList colors = {
        "#4a90e2",  // Blue
        "#9b59b6",  // Purple
        "#16a085",  // Teal
        "#e74c3c",  // Red
        "#f39c12",  // Orange
        "#2ecc71",  // Green
        "#3498db",  // Light Blue
        "#e67e22",  // Dark Orange
        "#1abc9c",  // Turquoise
        "#8e44ad"   // Deep Purple
    };

    // Use hash to select color
    const uint hash = qHash(name);
    const int colorIndex = hash % colors.size();

    return colors[colorIndex];
}

QVariantList WorkspaceViewModel::getWorkspaceProjects(int index) const {
    QVariantList result;
    
    if (index < 0 || index >= static_cast<int>(workspaces_.size())) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: Invalid workspace index:" << index;
        return result;
    }
    
    const auto& workspace = workspaces_[index];
    if (!workspace) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: Null workspace at index:" << index;
        return result;
    }
    
    if (!projectRepository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No project repository set";
        return result;
    }
    
    // Get project IDs from workspace
    const auto& projectIds = workspace->getProjectIds();
    qDebug() << "Loading" << projectIds.size() << "projects for workspace" << workspace->getName();
    
    // Load each project from repository
    for (const auto& projectId : projectIds) {
        auto project = projectRepository_->loadProject(projectId);
        if (!project) {
            qWarning() << "Failed to load project with ID:" << projectId;
            continue;
        }
        
        QVariantMap projectInfo;
        projectInfo["id"] = project->id();
        projectInfo["name"] = project->name();
        projectInfo["path"] = project->path();
        
        // Get scripts from project
        QVariantList scripts;
        const auto& projectScripts = project->scripts();
        for (const auto& script : projectScripts) {
            QVariantMap scriptInfo;
            scriptInfo["name"] = script.name;
            scriptInfo["command"] = script.command;
            scripts.append(scriptInfo);
        }
        projectInfo["scripts"] = scripts;
        
        result.append(projectInfo);
    }
    
    qDebug() << "Loaded" << result.size() << "projects from repository";
    
    return result;
}

} // namespace ZenRunner::UI
