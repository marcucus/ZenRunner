#include "ui/IProjectManager.hpp"

namespace ZenRunner::UI {

IProjectManager::IProjectManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

} // namespace ZenRunner::UI
