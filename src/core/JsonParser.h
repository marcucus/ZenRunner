#pragma once

#include "core/IJsonParser.hpp"
#include <memory>

namespace ZenRunner::Core {

/**
 * @brief Factory function to create JsonParser instances
 * @return Unique pointer to IJsonParser implementation
 */
std::unique_ptr<IJsonParser> createJsonParser();

} // namespace ZenRunner::Core
