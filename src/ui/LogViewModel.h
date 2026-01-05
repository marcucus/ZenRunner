#pragma once

#include "ui/ILogViewModel.hpp"
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Factory function to create LogViewModel instances
 * @param parent Optional parent QObject
 * @return Unique pointer to ILogViewModel implementation
 */
std::unique_ptr<ILogViewModel> createLogViewModel(QObject* parent = nullptr);

} // namespace ZenRunner::UI
