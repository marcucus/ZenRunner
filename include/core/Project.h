#pragma once

#include "../types/CommonTypes.h"
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <memory>
#include <vector>

namespace ZenRunner {

/**
 * @brief Represents a script that can be executed
 */
struct Script {
    QString name;
    QString command;
    bool isPinned = false;
    ProcessPriority priority = ProcessPriority::Normal;

    Script() = default;
    Script(QString name_, QString command_)
        : name(std::move(name_)), command(std::move(command_)) {}

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] static Script fromJson(const QJsonObject& json);
};

/**
 * @brief Represents a project with its scripts and metadata
 */
class Project {
public:
    Project() = default;
    explicit Project(QString path);

    /**
     * @brief Load project from a directory containing package.json
     * 
     * This function scans the directory for package.json, parses it,
     * and extracts available scripts.
     * 
     * @param path Path to the project directory
     * @return Result with Project if successful
     */
    [[nodiscard]] static Result<Project> fromDirectory(const QString& path);

    /**
     * @brief Reload project configuration from disk
     */
    [[nodiscard]] Result<bool> reload();

    // Getters
    [[nodiscard]] const QString& id() const noexcept { return id_; }
    [[nodiscard]] const QString& name() const noexcept { return name_; }
    [[nodiscard]] const QString& path() const noexcept { return path_; }
    [[nodiscard]] const QString& description() const noexcept { return description_; }
    [[nodiscard]] const std::vector<Script>& scripts() const noexcept { return scripts_; }
    [[nodiscard]] bool isValid() const noexcept { return valid_; }

    /**
     * @brief Get a specific script by name
     */
    [[nodiscard]] std::optional<Script> getScript(const QString& name) const;

    /**
     * @brief Get all pinned scripts
     */
    [[nodiscard]] std::vector<Script> getPinnedScripts() const;

    /**
     * @brief Pin or unpin a script
     */
    void setScriptPinned(const QString& scriptName, bool pinned);

    /**
     * @brief Set script priority
     */
    void setScriptPriority(const QString& scriptName, ProcessPriority priority);

    /**
     * @brief Create ProcessConfig for a script
     * 
     * This generates the appropriate configuration for running
     * the script with npm/yarn/pnpm based on project setup.
     */
    [[nodiscard]] std::optional<ProcessConfig> createProcessConfig(
        const QString& scriptName
    ) const;

    /**
     * @brief Serialize to JSON
     */
    [[nodiscard]] QJsonObject toJson() const;

    /**
     * @brief Deserialize from JSON
     */
    [[nodiscard]] static Project fromJson(const QJsonObject& json);

private:
    void parsePackageJson(const QJsonObject& packageJson);
    QString detectPackageManager() const;

    QString id_;
    QString name_;
    QString path_;
    QString description_;
    QString packageManager_ = "npm";  // npm, yarn, pnpm
    std::vector<Script> scripts_;
    bool valid_ = false;
};

/**
 * @brief Represents a workspace containing multiple projects
 */
class Workspace {
public:
    Workspace() = default;
    explicit Workspace(QString name);

    // Getters
    [[nodiscard]] const QString& id() const noexcept { return id_; }
    [[nodiscard]] const QString& name() const noexcept { return name_; }
    [[nodiscard]] const QString& description() const noexcept { return description_; }
    [[nodiscard]] const std::vector<QString>& projectIds() const noexcept { 
        return projectIds_; 
    }

    // Setters
    void setName(const QString& name) { name_ = name; }
    void setDescription(const QString& description) { description_ = description; }

    /**
     * @brief Add a project to the workspace
     */
    void addProject(const QString& projectId);

    /**
     * @brief Remove a project from the workspace
     */
    void removeProject(const QString& projectId);

    /**
     * @brief Check if workspace contains a project
     */
    [[nodiscard]] bool containsProject(const QString& projectId) const;

    /**
     * @brief Get count of projects in workspace
     */
    [[nodiscard]] std::size_t projectCount() const noexcept { 
        return projectIds_.size(); 
    }

    /**
     * @brief Serialize to JSON
     */
    [[nodiscard]] QJsonObject toJson() const;

    /**
     * @brief Deserialize from JSON
     */
    [[nodiscard]] static Workspace fromJson(const QJsonObject& json);

private:
    QString id_;
    QString name_;
    QString description_;
    std::vector<QString> projectIds_;
};

/**
 * @brief Utilities for project detection and management
 */
class ProjectScanner {
public:
    /**
     * @brief Scan a directory for projects (package.json files)
     * 
     * @param rootPath Root directory to scan
     * @param maxDepth Maximum depth to recurse (default: 3)
     * @return Vector of discovered project paths
     */
    [[nodiscard]] static std::vector<QString> scanDirectory(
        const QString& rootPath,
        int maxDepth = 3
    );

    /**
     * @brief Check if a directory contains a valid project
     */
    [[nodiscard]] static bool isProjectDirectory(const QString& path);

    /**
     * @brief Load multiple projects from a list of paths
     */
    [[nodiscard]] static std::vector<Project> loadProjects(
        const QStringList& paths
    );

private:
    static void scanDirectoryRecursive(
        const QString& path,
        int currentDepth,
        int maxDepth,
        std::vector<QString>& results
    );
};

} // namespace ZenRunner
