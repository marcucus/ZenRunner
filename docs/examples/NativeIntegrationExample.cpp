/**
 * @file NativeIntegrationExample.cpp
 * @brief Quick reference examples for native OS integration
 */

#include "platform/NativePlatformManager.h"
#include "platform/NativeEffects.h"
#include "platform/SystemTray.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

using namespace ZenRunner::Platform;

// ============================================================================
// EXAMPLE 1: Basic System Tray Setup
// ============================================================================

void example_BasicSystemTray()
{
    SystemTray tray;
    
    // Show the tray icon
    if (tray.isSystemTrayAvailable()) {
        tray.show();
        tray.setTooltip("ZenRunner - Process Manager");
    }
    
    // Handle activation (click)
    tray.onActivated([]() {
        // Show main window
    });
}

// ============================================================================
// EXAMPLE 2: System Tray with Context Menu
// ============================================================================

void example_TrayWithMenu(QObject* parent)
{
    SystemTray tray(parent);
    
    // Create context menu
    QMenu* menu = new QMenu();
    menu->addAction("Show", []() { /* show window */ });
    menu->addSeparator();
    menu->addAction("Quit", []() { /* quit app */ });
    
    tray.setContextMenu(menu);
    tray.show();
}

// ============================================================================
// EXAMPLE 3: Tray State Management
// ============================================================================

void example_TrayStateManagement()
{
    SystemTray tray;
    tray.show();
    
    // Idle state (gray icon)
    tray.setState(TrayIconState::Idle);
    
    // Active state (green icon)
    tray.setState(TrayIconState::Active);
    tray.showMessage("Process Started", "Server is now running");
    
    // Error state (red icon)
    tray.setState(TrayIconState::Error);
    tray.showMessage("Process Error", "Server crashed unexpectedly");
    
    // Warning state (orange icon)
    tray.setState(TrayIconState::Warning);
}

// ============================================================================
// EXAMPLE 4: Windows Mica Effect
// ============================================================================

void example_WindowsMica(QWindow* window)
{
#ifdef Q_OS_WIN
    NativeEffects* effects = NativeEffects::create();
    
    if (effects->isEffectSupported(NativeEffects::Effect::Mica)) {
        // Apply standard Mica
        effects->applyEffect(window, NativeEffects::Effect::Mica);
    } else if (effects->isEffectSupported(NativeEffects::Effect::MicaAlt)) {
        // Fall back to Mica Alt
        effects->applyEffect(window, NativeEffects::Effect::MicaAlt);
    }
    
    delete effects;
#endif
}

// ============================================================================
// EXAMPLE 5: macOS Vibrancy
// ============================================================================

void example_MacOSVibrancy(QWindow* window)
{
#ifdef Q_OS_MACOS
    NativeEffects* effects = NativeEffects::create();
    
    if (effects->isEffectSupported(NativeEffects::Effect::Vibrancy)) {
        effects->applyEffect(window, NativeEffects::Effect::Vibrancy);
    }
    
    delete effects;
#endif
}

// ============================================================================
// EXAMPLE 6: macOS Notch Detection
// ============================================================================

void example_MacOSNotchDetection(QWindow* window)
{
#ifdef Q_OS_MACOS
    #include "platform/MacOSNativeEffects.h"
    
    QMargins safeArea = MacOSNativeEffects::getSafeAreaInsets(window);
    
    if (safeArea.top() > 0) {
        // Device has a notch
        qDebug() << "Notch detected, top safe area:" << safeArea.top();
        
        // Position UI elements to avoid the notch
        // Example: Add top padding to QML layout
    } else {
        // No notch
        qDebug() << "No notch detected";
    }
#endif
}

// ============================================================================
// EXAMPLE 7: Unified Platform Manager (RECOMMENDED)
// ============================================================================

void example_UnifiedPlatformManager(QGuiApplication& app)
{
    NativePlatformManager manager;
    
    // Initialize system tray
    if (manager.initializeSystemTray()) {
        qDebug() << "System tray initialized";
    }
    
    // Connect signals
    QObject::connect(&manager, &NativePlatformManager::showRequested, []() {
        // Show main window
    });
    
    QObject::connect(&manager, &NativePlatformManager::quitRequested, 
                     &app, &QGuiApplication::quit);
    
    // Update tray state
    manager.setTrayState(TrayIconState::Active);
    manager.showNotification("Started", "Application is running");
}

// ============================================================================
// EXAMPLE 8: Complete Integration
// ============================================================================

int example_CompleteIntegration(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    
    // Set app metadata
    app.setApplicationName("ZenRunner");
    app.setOrganizationName("ZenRunner");
    
    // Create platform manager
    NativePlatformManager platformManager;
    
    // Initialize tray
    platformManager.initializeSystemTray();
    
    // Load QML
    QQmlApplicationEngine engine;
    engine.load(QUrl("qrc:/ui/Main.qml"));
    
    // Get root window and apply native effects
    auto rootObjects = engine.rootObjects();
    if (!rootObjects.isEmpty()) {
        QQuickWindow* window = qobject_cast<QQuickWindow*>(rootObjects.first());
        if (window) {
            platformManager.initializeNativeEffects(window);
        }
    }
    
    // Connect platform signals
    QObject::connect(&platformManager, &NativePlatformManager::showRequested,
                     [&engine]() {
        auto rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QQuickWindow* window = qobject_cast<QQuickWindow*>(rootObjects.first());
            if (window) {
                window->show();
                window->raise();
                window->requestActivate();
            }
        }
    });
    
    QObject::connect(&platformManager, &NativePlatformManager::quitRequested,
                     &app, &QGuiApplication::quit);
    
    return app.exec();
}

// ============================================================================
// EXAMPLE 9: Dynamic State Updates
// ============================================================================

class ProcessMonitor : public QObject
{
    Q_OBJECT
    
public:
    ProcessMonitor(NativePlatformManager* platformManager)
        : m_platformManager(platformManager)
    {}
    
public slots:
    void onProcessStarted() {
        m_platformManager->setTrayState(TrayIconState::Active);
        m_platformManager->showNotification(
            "Process Started",
            "Development server is now running"
        );
    }
    
    void onProcessStopped() {
        m_platformManager->setTrayState(TrayIconState::Idle);
    }
    
    void onProcessCrashed(const QString& processName) {
        m_platformManager->setTrayState(TrayIconState::Error);
        m_platformManager->showNotification(
            "Process Crashed",
            processName + " has stopped unexpectedly"
        );
    }
    
private:
    NativePlatformManager* m_platformManager;
};

// ============================================================================
// EXAMPLE 10: Custom Tray Icon
// ============================================================================

void example_CustomTrayIcon()
{
    SystemTray tray;
    
    // Create custom icon
    QIcon customIcon(":/icons/custom_icon.png");
    tray.setIcon(customIcon);
    
    // Or use state-based icons (automatic color-coded icons)
    tray.setState(TrayIconState::Active); // Green icon
    
    tray.show();
}

#include "NativeIntegrationExample.moc"
