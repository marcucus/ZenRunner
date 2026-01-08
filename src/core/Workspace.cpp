#include "Workspace.h"
#include "core/IProcessManager.hpp"
#include <QUuid>
#include <algorithm>

namespace ZenRunner::Core {

/**
 * @brief Memory-efficient Workspace implementation
 * 
 * Uses std::vector for projects (better cache locality than QList)
 * and minimizes allocations through reserve() and move semantics.
 * Integrates with ProcessManager for batch process operations.
 */
class Workspace : public IWorkspace {
public:
    explicit Workspace(const QString& name, const QString& id = QString())
        : id_(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id)
        , name_(name)
        , processManager_(nullptr)
    {
        // Pre-allocate space for typical workspace size (5-10 projects)
        projects_.reserve(10);
    }

    ~Workspace() override = default;

    /**
     * @brief Set the process manager for this workspace
     * @param manager Pointer to process manager
     */
    void setProcessManager(IProcessManager* manager) override {
        processManager_ = manager;
    }

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

    int startAll(const QString& scriptName, IWorkspace::ExecutionMode mode) override {
        if (!processManager_) [[unlikely]] {
            return 0;
        }

        int count = 0;

        if (mode == IWorkspace::ExecutionMode::Parallel) {
            // Parallel execution: start all processes simultaneously
            for (const auto& project : projects_) {
                if (!project) [[unlikely]] {
                    continue;
                }

                const auto* script = project->getScript(scriptName);
                if (!script) {
                    continue;
                }

                // Create a unique process ID for this project+script combination
                const QString processId = QString("%1_%2_%3")
                    .arg(id_)
                    .arg(project->getId())
                    .arg(scriptName);

                // Determine command based on project type (npm/yarn/pnpm)
                QString command;
                QStringList arguments;
                
                // For now, assume npm projects (can be enhanced later)
                command = "npm";
                arguments << "run" << scriptName;

                // Start the process
                bool started = processManager_->startProcess(
                    processId,
                    command,
                    arguments,
                    project->getPath()
                );

                if (started) [[likely]] {
                    ++count;
                }
            }
        } else {
            // Sequential execution: start processes one after another
            // In a real implementation, we would wait for each process to start
            // before starting the next one. For now, we'll use a simple delay approach.
            // TODO: Implement proper sequential startup with state tracking
            
            for (const auto& project : projects_) {
                if (!project) [[unlikely]] {
                    continue;
                }

                const auto* script = project->getScript(scriptName);
                if (!script) {
                    continue;
                }

                // Create a unique process ID for this project+script combination
                const QString processId = QString("%1_%2_%3")
                    .arg(id_)
                    .arg(project->getId())
                    .arg(scriptName);

                // Determine command based on project type
                QString command = "npm";
                QStringList arguments;
                arguments << "run" << scriptName;

                // Start the process
                bool started = processManager_->startProcess(
                    processId,
                    command,
                    arguments,
                    project->getPath()
                );

                if (started) [[likely]] {
                    ++count;
                }
                
                // In sequential mode, we would typically wait here for the process
                // to reach a running state before starting the next one
            }
        }

        return count;
    }

    int stopAll(bool forceKill) override {
        if (!processManager_) [[unlikely]] {
            return 0;
        }

        int count = 0;
        
        // Stop all processes related to this workspace
        // Process IDs follow the pattern: workspaceId_projectId_scriptName
        for (const auto& project : projects_) {
            if (!project) [[unlikely]] {
                continue;
            }

            // For each project, we need to stop all its running processes
            // Since we don't track active process IDs yet, this is a simplified version
            // TODO: Maintain a list of active process IDs per workspace
            
            // Try to stop processes that might exist for this project
            const auto scripts = project->getScripts();
            for (const auto& script : scripts) {
                const QString processId = QString("%1_%2_%3")
                    .arg(id_)
                    .arg(project->getId())
                    .arg(script.name);
                
                // Check if process exists and stop it
                if (processManager_->isProcessRunning(processId)) [[unlikely]] {
                    bool stopped = processManager_->stopProcess(processId, forceKill);
                    if (stopped) [[likely]] {
                        ++count;
                    }
                }
            }
        }

        return count;
    }

    bool isAnyProjectRunning() const override {
        if (!processManager_) [[unlikely]] {
            return false;
        }

        // Check if any process related to this workspace is running
        // Process IDs follow the pattern: workspaceId_projectId_scriptName
        for (const auto& project : projects_) {
            if (!project) [[unlikely]] {
                continue;
            }

            // Check all possible process IDs for this project
            const auto scripts = project->getScripts();
            for (const auto& script : scripts) {
                const QString processId = QString("%1_%2_%3")
                    .arg(id_)
                    .arg(project->getId())
                    .arg(script.name);
                
                if (processManager_->isProcessRunning(processId)) [[unlikely]] {
                    return true;
                }
            }
        }

        return false;
    }

private:
    QString id_;
    QString name_;
    QString description_;
    std::vector<std::shared_ptr<IProject>> projects_;
    IProcessManager* processManager_;  // Non-owning pointer to process manager
};

// Factory function
std::shared_ptr<IWorkspace> createWorkspace(const QString& name, const QString& id) {
    return std::make_shared<Workspace>(name, id);
}

} // namespace ZenRunner::Core
