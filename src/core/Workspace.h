#pragma once

#include "core/IWorkspace.hpp"
#include <memory>

namespace ZenRunner::Core {

/**
 * @brief Factory function to create Workspace instances
 * @param name Workspace name
 * @param id Optional workspace ID (generated if not provided)
 * @return Shared pointer to IWorkspace implementation
 */
std::shared_ptr<IWorkspace> createWorkspace(const QString& name, const QString& id = QString());

} // namespace ZenRunner::Core
