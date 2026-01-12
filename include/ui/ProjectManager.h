#pragma once

#include "core/Project.h"
#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <vector>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Manages project detection and listing for the UI
 * 
 * This class bridges the C++ ProjectScanner with the QML interface,
 * providing a model for displaying detected projects and their scripts.
 */
class ProjectManager : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int projectCount READ projectCount NOTIFY projectCountChanged)

public:
    enum ProjectRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        PathRole,
        ScriptCountRole,
        ScriptsRole
    };

    explicit ProjectManager(QObject* parent = nullptr);
    ~ProjectManager() override = default;

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ProjectManager interface
    int projectCount() const { return static_cast<int>(projects_.size()); }
    
    /**
     * @brief Scan a folder for projects
     * @param folderPath Path to the folder to scan
     * @param maxDepth Maximum depth for recursive scanning
     */
    Q_INVOKABLE void scanFolder(const QString& folderPath, int maxDepth = 3);
    
    /**
     * @brief Add a single project from a folder
     * @param folderPath Path to the project folder
     */
    Q_INVOKABLE void addProject(const QString& folderPath);
    
    /**
     * @brief Clear all detected projects
     */
    Q_INVOKABLE void clearProjects();
    
    /**
     * @brief Get project at index
     * @param index Project index
     * @return Project data as QVariantMap
     */
    Q_INVOKABLE QVariantMap getProject(int index) const;

signals:
    /**
     * @brief Emitted when projects are detected
     * @param count Number of projects found
     */
    void projectsDetected(int count);

    /**
     * @brief Emitted when project count changes
     */
    void projectCountChanged();

    /**
     * @brief Emitted when scanning is complete
     * @param success Whether scan was successful
     * @param message Status message
     */
    void scanComplete(bool success, const QString& message);

private:
    std::vector<ZenRunner::Project> projects_;
};

} // namespace ZenRunner::UI
