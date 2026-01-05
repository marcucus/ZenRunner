#include "Workspace.h"
#include <QUuid>
#include <algorithm>

namespace ZenRunner::Core {

/**
 * @brief Memory-efficient Workspace implementation
 * 
 * Uses std::vector for projects (better cache locality than QList)
 * and minimizes allocations through reserve() and move semantics.
 */
class Workspace : public IWorkspace {
public:
    explicit Workspace(const QString& name, const QString& id = QString())
        : name_(name)
        , id_(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id)
    {
        // Pre-allocate space for typical workspace size (5-10 projects)
        projects_.reserve(10);
    }

    ~Workspace() override = default;

    QString getId() const override {
        return id_;
    }

    QString getName() const override {
        return name_;
    }

    void setName(const QString& name) override {
        name_ = name;
    }

    QString getDescription() const override {
        return description_;
    }

    void setDescription(const QString& description) override {
        description_ = description;
    }

    bool addProject(std::shared_ptr<IProject> project) override {
        if (!project) [[unlikely]] {
            return false;
        }

        // Check if project already exists
        const QString projectId = project->getId();
        if (hasProject(projectId)) [[unlikely]] {
            return false;
        }

        projects_.push_back(std::move(project));
        return true;
    }

    bool removeProject(const QString& projectId) override {
        auto it = std::find_if(projects_.begin(), projects_.end(),
            [&projectId](const std::shared_ptr<IProject>& p) {
                return p && p->getId() == projectId;
            });

        if (it != projects_.end()) [[likely]] {
            projects_.erase(it);
            return true;
        }

        return false;
    }

    std::vector<std::shared_ptr<IProject>> getProjects() const override {
        return projects_;
    }

    std::shared_ptr<IProject> getProject(const QString& projectId) const override {
        auto it = std::find_if(projects_.begin(), projects_.end(),
            [&projectId](const std::shared_ptr<IProject>& p) {
                return p && p->getId() == projectId;
            });

        if (it != projects_.end()) [[likely]] {
            return *it;
        }

        return nullptr;
    }

    bool hasProject(const QString& projectId) const override {
        return std::any_of(projects_.begin(), projects_.end(),
            [&projectId](const std::shared_ptr<IProject>& p) {
                return p && p->getId() == projectId;
            });
    }

    size_t getProjectCount() const override {
        return projects_.size();
    }

    int startAll(const QString& scriptName) override {
        // TODO: Requires ProcessManager reference for actual implementation
        // This is a placeholder that counts potential starts
        int count = 0;
        for (const auto& project : projects_) {
            if (project && project->getScript(scriptName)) {
                ++count;
            }
        }
        return count;
    }

    int stopAll(bool forceKill) override {
        // TODO: Requires ProcessManager reference for actual implementation
        return 0;
    }

    bool isAnyProjectRunning() const override {
        // TODO: Requires ProcessManager reference for actual implementation
        return false;
    }

private:
    QString id_;
    QString name_;
    QString description_;
    std::vector<std::shared_ptr<IProject>> projects_;
};

// Factory function
std::shared_ptr<IWorkspace> createWorkspace(const QString& name, const QString& id) {
    return std::make_shared<Workspace>(name, id);
}

} // namespace ZenRunner::Core
