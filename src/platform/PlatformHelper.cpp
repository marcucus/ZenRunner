#include "platform/PlatformHelper.h"

#ifdef Q_OS_MACOS
#include "platform/MacOSNativeEffects.h"
#endif

namespace ZenRunner::Platform {

PlatformHelper::PlatformHelper(QObject *parent)
    : QObject(parent)
    , m_hasNotch(false)
    , m_safeAreaTop(0)
    , m_safeAreaBottom(0)
    , m_safeAreaLeft(0)
    , m_safeAreaRight(0)
{
}

QString PlatformHelper::platformName() const
{
#if defined(Q_OS_WIN)
    return "Windows";
#elif defined(Q_OS_MACOS) || defined(Q_OS_MAC)
    return "macOS";
#elif defined(Q_OS_LINUX)
    return "Linux";
#else
    return "Unknown";
#endif
}

bool PlatformHelper::isWindows() const
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

bool PlatformHelper::isMacOS() const
{
#if defined(Q_OS_MACOS) || defined(Q_OS_MAC)
    return true;
#else
    return false;
#endif
}

bool PlatformHelper::isLinux() const
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}

void PlatformHelper::updateSafeAreaInsets(QWindow *window)
{
    if (!window) {
        return;
    }

#ifdef Q_OS_MACOS
    QMargins insets = MacOSNativeEffects::getSafeAreaInsets(window);
    
    m_safeAreaTop = insets.top();
    m_safeAreaBottom = insets.bottom();
    m_safeAreaLeft = insets.left();
    m_safeAreaRight = insets.right();
    
    // Device has a notch if top inset is greater than 0
    m_hasNotch = (m_safeAreaTop > 0);
    
    emit notchChanged();
#else
    // No notch on other platforms
    m_hasNotch = false;
    m_safeAreaTop = 0;
    m_safeAreaBottom = 0;
    m_safeAreaLeft = 0;
    m_safeAreaRight = 0;
#endif
}

} // namespace ZenRunner::Platform
