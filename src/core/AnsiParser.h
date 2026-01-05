#pragma once

#include "core/IAnsiParser.hpp"
#include <memory>

namespace ZenRunner::Core {

// Factory function to create AnsiParser instances
std::unique_ptr<IAnsiParser> createAnsiParser();

} // namespace ZenRunner::Core
