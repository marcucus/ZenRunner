#include "JsonParser.h"
#include "core/IWorkspace.hpp"
#include <QFile>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

namespace ZenRunner::Core {

/**
 * @brief Memory-efficient JSON parser implementation
 * 
 * This implementation minimizes allocations by:
 * - Using Qt's implicit sharing for strings and containers
 * - Parsing on-demand without caching
 * - Avoiding unnecessary copies
 */
class JsonParser : public IJsonParser {
public:
    JsonParser() = default;
    ~JsonParser() override = default;

    ParseResult parseFile(const QString& filePath) override {
        ParseResult result;
        
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) [[unlikely]] {
            result.success = false;
            result.errorMessage = QString("Failed to open file: %1").arg(file.errorString());
            return result;
        }
        
        QJsonParseError parseError;
        const QByteArray fileData = file.readAll();
        file.close();
        
        const QJsonDocument doc = QJsonDocument::fromJson(fileData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
            result.success = false;
            result.errorMessage = QString("JSON parse error at offset %1: %2")
                .arg(parseError.offset)
                .arg(parseError.errorString());
            return result;
        }
        
        if (!doc.isObject()) [[unlikely]] {
            result.success = false;
            result.errorMessage = "JSON root is not an object";
            return result;
        }
        
        result.success = true;
        result.data = doc.object();
        return result;
    }

    ParseResult parseString(const QString& jsonString) override {
        ParseResult result;
        
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
        
        if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
            result.success = false;
            result.errorMessage = QString("JSON parse error at offset %1: %2")
                .arg(parseError.offset)
                .arg(parseError.errorString());
            return result;
        }
        
        if (!doc.isObject()) [[unlikely]] {
            result.success = false;
            result.errorMessage = "JSON root is not an object";
            return result;
        }
        
        result.success = true;
        result.data = doc.object();
        return result;
    }

    QMap<QString, QString> extractScripts(const QJsonObject& jsonObject) override {
        QMap<QString, QString> scripts;
        
        if (!jsonObject.contains("scripts")) [[unlikely]] {
            return scripts;
        }
        
        const QJsonValue scriptsValue = jsonObject["scripts"];
        if (!scriptsValue.isObject()) [[unlikely]] {
            return scripts;
        }
        
        const QJsonObject scriptsObject = scriptsValue.toObject();
        
        // Reserve space if we know the size
        // QMap doesn't have reserve(), but we iterate efficiently
        for (auto it = scriptsObject.constBegin(); it != scriptsObject.constEnd(); ++it) {
            if (it.value().isString()) [[likely]] {
                scripts.insert(it.key(), it.value().toString());
            }
        }
        
        return scripts;
    }

    QString extractProjectName(const QJsonObject& jsonObject) override {
        if (jsonObject.contains("name") && jsonObject["name"].isString()) [[likely]] {
            return jsonObject["name"].toString();
        }
        return QString();
    }

    std::shared_ptr<IProject> createProjectFromPackageJson(
        const QString& packageJsonPath) override {
        
        // Parse the package.json file
        ParseResult parseResult = parseFile(packageJsonPath);
        
        if (!parseResult.success) [[unlikely]] {
            return nullptr;
        }
        
        // TODO: Requires Project factory implementation
        // This method will be fully implemented once the Project factory is available
        // For now, it validates the package.json but returns nullptr
        
        return nullptr;
    }

    QJsonObject serializeProject(const IProject& project) override {
        QJsonObject obj;
        
        obj["id"] = project.getId();
        obj["name"] = project.getName();
        obj["path"] = project.getPath();
        
        // Serialize scripts
        QJsonObject scriptsObj;
        const auto scripts = project.getScripts();
        for (auto it = scripts.constBegin(); it != scripts.constEnd(); ++it) {
            QJsonObject scriptObj;
            scriptObj["name"] = it.value().name;
            scriptObj["command"] = it.value().command;
            scriptObj["isPinned"] = it.value().isPinned;
            scriptsObj[it.key()] = scriptObj;
        }
        obj["scripts"] = scriptsObj;
        
        // Serialize pinned scripts
        QJsonArray pinnedArray;
        const auto pinned = project.getPinnedScripts();
        for (const QString& scriptName : pinned) {
            pinnedArray.append(scriptName);
        }
        obj["pinnedScripts"] = pinnedArray;
        
        return obj;
    }

    QJsonObject serializeWorkspace(const IWorkspace& workspace) override {
        QJsonObject obj;
        
        obj["id"] = workspace.getId();
        obj["name"] = workspace.getName();
        
        // Serialize projects
        QJsonArray projectsArray;
        const auto projects = workspace.getProjects();
        for (const auto& project : projects) {
            if (project) [[likely]] {
                projectsArray.append(serializeProject(*project));
            }
        }
        obj["projects"] = projectsArray;
        
        return obj;
    }

    bool writeToFile(const QString& filePath, const QJsonObject& jsonObject) override {
        QFile file(filePath);
        
        if (!file.open(QIODevice::WriteOnly)) [[unlikely]] {
            return false;
        }
        
        const QJsonDocument doc(jsonObject);
        const qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        
        return written > 0;
    }
};

// Factory function
std::unique_ptr<IJsonParser> createJsonParser() {
    return std::make_unique<JsonParser>();
}

} // namespace ZenRunner::Core
