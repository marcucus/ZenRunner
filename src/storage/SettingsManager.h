#pragma once

#include "storage/ISettingsManager.hpp"
#include <memory>

namespace ZenRunner::Storage {

/**
 * @brief Factory function to create SettingsManager instances
 * @return Unique pointer to ISettingsManager implementation
 */
std::unique_ptr<ISettingsManager> createSettingsManager();

/**
 * @brief Factory function to create SettingsManager with custom organization/app
 * @param organization Organization name
 * @param application Application name
 * @return Unique pointer to ISettingsManager implementation
 */
std::unique_ptr<ISettingsManager> createSettingsManager(
    const QString& organization,
    const QString& application);

} // namespace ZenRunner::Storage
