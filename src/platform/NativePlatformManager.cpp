#include "platform/NativePlatformManager.h"
#include "platform/NativeEffects.h"
#include "platform/SystemTray.h"
#include <QMenu>
#include <QAction>

namespace ZenRunner::Platform {

NativePlatformManager::NativePlatformManager(QObject *parent)
    : QObject(parent)
    , m_nativeEffects(nullptr)
    , m_systemTray(nullptr)
    , m_trayMenu(nullptr)
    , m_showAction(nullptr)
    , m_quitAction(nullptr)
{
}

NativePlatformManager::~NativePlatformManager()
{
    // Cleanup is handled by unique_ptr
}

bool NativePlatformManager::initializeNativeEffects(QWindow *window)
{
    if (!window) {
        return false;
    }

    // Create platform-specific native effects
    m_nativeEffects.reset(NativeEffects::create(this));
    
    if (!m_nativeEffects) {
        return false;
    }

    // Try to apply the native effect based on platform
    NativeEffects::Effect effect = NativeEffects::Effect::None;
    
#ifdef Q_OS_WIN
    // Windows: Try Mica first, fall back to MicaAlt
    if (m_nativeEffects->isEffectSupported(NativeEffects::Effect::Mica)) {
        effect = NativeEffects::Effect::Mica;
    } else if (m_nativeEffects->isEffectSupported(NativeEffects::Effect::MicaAlt)) {
        effect = NativeEffects::Effect::MicaAlt;
    }
#elif defined(Q_OS_MACOS) || defined(Q_OS_MAC)
    // macOS: Use Vibrancy
    if (m_nativeEffects->isEffectSupported(NativeEffects::Effect::Vibrancy)) {
        effect = NativeEffects::Effect::Vibrancy;
    }
#else
    // Linux/Other: Use generic blur if available
    if (m_nativeEffects->isEffectSupported(NativeEffects::Effect::Blur)) {
        effect = NativeEffects::Effect::Blur;
    }
#endif

    if (effect != NativeEffects::Effect::None) {
        return m_nativeEffects->applyEffect(window, effect);
    }

    return false;
}

bool NativePlatformManager::initializeSystemTray()
{
    m_systemTray = std::make_unique<SystemTray>(this);
    
    if (!m_systemTray->isSystemTrayAvailable()) {
        return false;
    }

    // Create tray menu
    createTrayMenu();

    // Connect tray icon signals
    m_systemTray->onActivated([this]() {
        handleTrayActivated();
    });

    // Set initial state and show
    m_systemTray->setState(TrayIconState::Idle);
    return m_systemTray->show();
}

void NativePlatformManager::setTrayState(TrayIconState state)
{
    if (m_systemTray) {
        m_systemTray->setState(state);
    }
}

void NativePlatformManager::showNotification(const QString& title, const QString& message)
{
    if (m_systemTray && m_systemTray->isVisible()) {
        m_systemTray->showMessage(title, message);
    }
}

void NativePlatformManager::handleTrayActivated()
{
    emit trayIconActivated();
    emit showRequested();
}

void NativePlatformManager::handleShowAction()
{
    emit showRequested();
}

void NativePlatformManager::handleQuitAction()
{
    emit quitRequested();
}

void NativePlatformManager::createTrayMenu()
{
    m_trayMenu = new QMenu();

    // Show/Hide action
    m_showAction = new QAction("Show ZenRunner", this);
    connect(m_showAction, &QAction::triggered, this, &NativePlatformManager::handleShowAction);
    m_trayMenu->addAction(m_showAction);

    m_trayMenu->addSeparator();

    // Quit action
    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, &QAction::triggered, this, &NativePlatformManager::handleQuitAction);
    m_trayMenu->addAction(m_quitAction);

    // Set the context menu
    if (m_systemTray) {
        m_systemTray->setContextMenu(m_trayMenu);
    }
}

} // namespace ZenRunner::Platform
