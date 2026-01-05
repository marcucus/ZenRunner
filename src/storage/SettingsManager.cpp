#include "storage/SettingsManager.h"
#include <QSettings>
#include <QCoreApplication>

namespace ZenRunner::Storage {

/**
 * @brief Memory-efficient SettingsManager implementation using QSettings
 * 
 * This implementation:
 * - Uses QSettings which stores data on disk, not in memory
 * - Lazy loads values only when requested
 * - Leverages Qt's implicit sharing for returned strings
 * - No caching layer to minimize memory usage
 */
class SettingsManager : public ISettingsManager {
public:
    SettingsManager()
        : settings_(std::make_unique<QSettings>(
            QCoreApplication::organizationName(),
            QCoreApplication::applicationName()))
    {
    }

    explicit SettingsManager(const QString& organization, const QString& application)
        : settings_(std::make_unique<QSettings>(organization, application))
    {
    }

    ~SettingsManager() override = default;

    QVariant getValue(const QString& key, const QVariant& defaultValue) const override {
        return settings_->value(key, defaultValue);
    }

    void setValue(const QString& key, const QVariant& value) override {
        settings_->setValue(key, value);
    }

    bool contains(const QString& key) const override {
        return settings_->contains(key);
    }

    void remove(const QString& key) override {
        settings_->remove(key);
    }

    QStringList getAllKeys(const QString& group) const override {
        if (group.isEmpty()) [[likely]] {
            return settings_->allKeys();
        } else {
            settings_->beginGroup(group);
            QStringList keys = settings_->allKeys();
            settings_->endGroup();
            return keys;
        }
    }

    QStringList getChildGroups(const QString& group) const override {
        if (group.isEmpty()) [[likely]] {
            return settings_->childGroups();
        } else {
            settings_->beginGroup(group);
            QStringList groups = settings_->childGroups();
            settings_->endGroup();
            return groups;
        }
    }

    void clear() override {
        settings_->clear();
    }

    void sync() override {
        settings_->sync();
    }

    // Convenience methods
    QString getString(const QString& key, const QString& defaultValue) const override {
        return settings_->value(key, defaultValue).toString();
    }

    int getInt(const QString& key, int defaultValue) const override {
        return settings_->value(key, defaultValue).toInt();
    }

    bool getBool(const QString& key, bool defaultValue) const override {
        return settings_->value(key, defaultValue).toBool();
    }

    double getDouble(const QString& key, double defaultValue) const override {
        return settings_->value(key, defaultValue).toDouble();
    }

    QStringList getStringList(const QString& key) const override {
        return settings_->value(key).toStringList();
    }

    void setStringList(const QString& key, const QStringList& value) override {
        settings_->setValue(key, value);
    }

private:
    std::unique_ptr<QSettings> settings_;
};

// Factory function
std::unique_ptr<ISettingsManager> createSettingsManager() {
    return std::make_unique<SettingsManager>();
}

std::unique_ptr<ISettingsManager> createSettingsManager(
    const QString& organization,
    const QString& application)
{
    return std::make_unique<SettingsManager>(organization, application);
}

} // namespace ZenRunner::Storage
