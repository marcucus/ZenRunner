#pragma once

#include "IProject.hpp"
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>
#include <optional>

namespace ZenRunner::Core {

// Forward declaration
class IWorkspace;

/**
 * @brief Result of a JSON parsing operation
 */
struct ParseResult {
    bool success{false};           ///< Whether parsing was successful
    QString errorMessage;          ///< Error message if parsing failed
    QJsonObject data;              ///< Parsed JSON data
};

/**
 * @brief Interface for JSON parsing operations
 * 
 * This interface defines the contract for parsing configuration files
 * such as package.json and project metadata.
 */
class IJsonParser {
public:
    virtual ~IJsonParser() = default;

    /**
     * @brief Parse a JSON file
     * @param filePath Path to JSON file
     * @return Parse result containing data or error
     */
    virtual ParseResult parseFile(const QString& filePath) = 0;

    /**
     * @brief Parse a JSON string
     * @param jsonString JSON string to parse
     * @return Parse result containing data or error
     */
    virtual ParseResult parseString(const QString& jsonString) = 0;

    /**
     * @brief Extract scripts from a package.json object
     * @param jsonObject Parsed package.json object
     * @return Map of script names to commands
     */
    virtual QMap<QString, QString> extractScripts(const QJsonObject& jsonObject) = 0;

    /**
     * @brief Extract project name from configuration
     * @param jsonObject Parsed configuration object
     * @return Project name, or empty string if not found
     */
    virtual QString extractProjectName(const QJsonObject& jsonObject) = 0;

    /**
     * @brief Create a project from a package.json file
     * @param packageJsonPath Path to package.json
     * @return Shared pointer to created project, or nullptr if failed
     */
    virtual std::shared_ptr<IProject> createProjectFromPackageJson(
        const QString& packageJsonPath
    ) = 0;

    /**
     * @brief Serialize project data to JSON
     * @param project Project to serialize
     * @return JSON object representing the project
     */
    virtual QJsonObject serializeProject(const IProject& project) = 0;

    /**
     * @brief Serialize workspace data to JSON
     * @param workspace Workspace to serialize
     * @return JSON object representing the workspace
     */
    virtual QJsonObject serializeWorkspace(const IWorkspace& workspace) = 0;

    /**
     * @brief Write JSON object to file
     * @param filePath Path to output file
     * @param jsonObject JSON object to write
     * @return true if write was successful
     */
    virtual bool writeToFile(const QString& filePath, const QJsonObject& jsonObject) = 0;
};

} // namespace ZenRunner::Core
