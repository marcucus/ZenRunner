#pragma once

#include "core/ILogBuffer.hpp"
#include <memory>

namespace ZenRunner::Core {

/**
 * @brief Factory function to create LogBuffer instances
 * @param capacity Maximum number of log entries (default: 5000)
 * @return Unique pointer to ILogBuffer implementation
 */
std::unique_ptr<ILogBuffer> createLogBuffer(size_t capacity = 5000);

} // namespace ZenRunner::Core
