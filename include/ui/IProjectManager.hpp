#pragma once

#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <vector>
#include <memory>

namespace ZenRunner {
class Project;
}

namespace ZenRunner::UI {

/**
 * @brief Interface for ProjectManager exposed to QML
 * 
 * This interface provides functionality to scan folders, detect projects,
 * and manage the list of detected projects.
 */
class IProjectManager : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int projectCount READ projectCount NOTIFY projectCountChanged)

public:
    IProjectManager(QObject* parent = nullptr);
    virtual ~IProjectManager() = default;

    /**
     * @brief Get the number of projects
     */
    virtual int projectCount() const = 0;

    /**
     * @brief Scan a folder for projects
     * @param folderPath Path to the folder to scan
     * @param maxDepth Maximum depth for recursive scanning
     */
    Q_INVOKABLE virtual void scanFolder(const QString& folderPath, int maxDepth = 3) = 0;

    /**
     * @brief Clear all detected projects
     */
    Q_INVOKABLE virtual void clearProjects() = 0;

    /**
     * @brief Get project at index
     * @param index Project index
     * @return Pointer to project or nullptr
     */
    virtual const ZenRunner::Project* getProject(int index) const = 0;

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
};

} // namespace ZenRunner::UI
