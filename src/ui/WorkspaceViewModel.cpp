#include "WorkspaceViewModel.h"
#include "core/Workspace.h"
#include <QDebug>
#include <QColor>

namespace ZenRunner::UI {

WorkspaceViewModel::WorkspaceViewModel(QObject* parent)
    : QAbstractListModel(parent)
    , repository_(nullptr)
    , processManager_(nullptr)
{
}

void WorkspaceViewModel::setRepository(Storage::IWorkspaceRepository* repository) {
    repository_ = repository;
}

void WorkspaceViewModel::setProcessManager(Core::IProcessManager* manager) {
    processManager_ = manager;
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

bool WorkspaceViewModel::createWorkspace(const QString& name, const QString& description) {
    if (!repository_) [[unlikely]] {
        qWarning() << "WorkspaceViewModel: No repository set";
        emit errorOccurred("No workspace repository configured");
        return false;
    }

    if (name.isEmpty()) [[unlikely]] {
        emit errorOccurred("Workspace name cannot be empty");
        return false;
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
        return false;
    }

    // Add to model
    const int newRow = static_cast<int>(workspaces_.size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    workspaces_.push_back(workspace);
    endInsertRows();

    emit countChanged();
    emit workspaceCreated(workspace->getId());

    return true;
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

    // Note: We need to load the actual project from project repository
    // For now, this is a simplified implementation
    // TODO: Integrate with IProjectRepository

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

} // namespace ZenRunner::UI
