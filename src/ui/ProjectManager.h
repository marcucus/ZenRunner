#pragma once

#include "ui/IProjectManager.hpp"
#include "core/Project.h"
#include <QObject>
#include <QString>
#include <vector>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Manages project detection and listing for the UI
 * 
 * This class bridges the C++ ProjectScanner with the QML interface,
 * providing a model for displaying detected projects and their scripts.
 */
class ProjectManager : public IProjectManager {
    Q_OBJECT

public:
    enum ProjectRoles {
        NameRole = Qt::UserRole + 1,
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

    // IProjectManager interface
    int projectCount() const override { return static_cast<int>(projects_.size()); }
    void scanFolder(const QString& folderPath, int maxDepth = 3) override;
    void clearProjects() override;
    const ZenRunner::Project* getProject(int index) const override;

private:
    std::vector<ZenRunner::Project> projects_;
};

} // namespace ZenRunner::UI
