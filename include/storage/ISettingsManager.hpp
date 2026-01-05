#pragma once

#include <QString>
#include <QVariant>
#include <QStringList>
#include <optional>

namespace ZenRunner::Storage {

/**
 * @brief Interface for application settings management
 * 
 * This interface defines the contract for persistent storage of application
 * settings and preferences using QSettings or similar mechanisms.
 */
class ISettingsManager {
public:
    virtual ~ISettingsManager() = default;

    /**
     * @brief Get a setting value
     * @param key Setting key (supports hierarchical keys with '/')
     * @param defaultValue Default value if key doesn't exist
     * @return Setting value or default
     */
    virtual QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;

    /**
     * @brief Set a setting value
     * @param key Setting key
     * @param value Value to store
     */
    virtual void setValue(const QString& key, const QVariant& value) = 0;

    /**
     * @brief Check if a setting exists
     * @param key Setting key
     * @return true if setting exists
     */
    virtual bool contains(const QString& key) const = 0;

    /**
     * @brief Remove a setting
     * @param key Setting key
     */
    virtual void remove(const QString& key) = 0;

    /**
     * @brief Get all keys in a group
     * @param group Group name (empty for root)
     * @return List of keys
     */
    virtual QStringList getAllKeys(const QString& group = QString()) const = 0;

    /**
     * @brief Get all child groups
     * @param group Parent group name (empty for root)
     * @return List of child group names
     */
    virtual QStringList getChildGroups(const QString& group = QString()) const = 0;

    /**
     * @brief Clear all settings
     */
    virtual void clear() = 0;

    /**
     * @brief Force synchronization to disk
     */
    virtual void sync() = 0;

    // Convenience methods for common types

    /**
     * @brief Get a string value
     * @param key Setting key
     * @param defaultValue Default value
     * @return String value
     */
    virtual QString getString(const QString& key, const QString& defaultValue = QString()) const = 0;

    /**
     * @brief Get an integer value
     * @param key Setting key
     * @param defaultValue Default value
     * @return Integer value
     */
    virtual int getInt(const QString& key, int defaultValue = 0) const = 0;

    /**
     * @brief Get a boolean value
     * @param key Setting key
     * @param defaultValue Default value
     * @return Boolean value
     */
    virtual bool getBool(const QString& key, bool defaultValue = false) const = 0;

    /**
     * @brief Get a double value
     * @param key Setting key
     * @param defaultValue Default value
     * @return Double value
     */
    virtual double getDouble(const QString& key, double defaultValue = 0.0) const = 0;

    /**
     * @brief Get a string list value
     * @param key Setting key
     * @return String list
     */
    virtual QStringList getStringList(const QString& key) const = 0;

    /**
     * @brief Set a string list value
     * @param key Setting key
     * @param value String list to store
     */
    virtual void setStringList(const QString& key, const QStringList& value) = 0;
};

} // namespace ZenRunner::Storage
