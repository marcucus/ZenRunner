#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <memory>

namespace ZenRunner::Core {

/**
 * @brief Represents a script that can be executed
 */
struct Script {
    QString name;           ///< Script name (e.g., "start", "dev", "test")
    QString command;        ///< Actual command to execute
    bool isPinned{false};   ///< Whether script is pinned for quick access
};

/**
 * @brief Interface for project data model
 * 
 * This interface defines the contract for project representation and management.
 * Projects are detected from package.json or other configuration files.
 */
class IProject {
public:
    virtual ~IProject() = default;

    /**
     * @brief Get the unique identifier for this project
     * @return Project ID
     */
    virtual QString getId() const = 0;

    /**
     * @brief Get the project name
     * @return Project name
     */
    virtual QString getName() const = 0;

    /**
     * @brief Set the project name
     * @param name New project name
     */
    virtual void setName(const QString& name) = 0;

    /**
     * @brief Get the project path (root directory)
     * @return Absolute path to project
     */
    virtual QString getPath() const = 0;

    /**
     * @brief Get all available scripts for this project
     * @return Map of script name to Script object
     */
    virtual QMap<QString, Script> getScripts() const = 0;

    /**
     * @brief Add or update a script
     * @param script Script to add/update
     */
    virtual void addScript(const Script& script) = 0;

    /**
     * @brief Remove a script by name
     * @param scriptName Name of script to remove
     * @return true if script was removed
     */
    virtual bool removeScript(const QString& scriptName) = 0;

    /**
     * @brief Get a specific script by name
     * @param scriptName Name of the script
     * @return Pointer to script, or nullptr if not found
     */
    virtual const Script* getScript(const QString& scriptName) const = 0;

    /**
     * @brief Pin a script for quick access
     * @param scriptName Name of script to pin
     * @return true if script was pinned
     */
    virtual bool pinScript(const QString& scriptName) = 0;

    /**
     * @brief Unpin a script
     * @param scriptName Name of script to unpin
     * @return true if script was unpinned
     */
    virtual bool unpinScript(const QString& scriptName) = 0;

    /**
     * @brief Get list of pinned scripts
     * @return List of pinned script names
     */
    virtual QStringList getPinnedScripts() const = 0;

    /**
     * @brief Check if project is valid (has accessible path and scripts)
     * @return true if project is valid
     */
    virtual bool isValid() const = 0;

    /**
     * @brief Reload project configuration from disk
     * @return true if reload was successful
     */
    virtual bool reload() = 0;
};

} // namespace ZenRunner::Core
