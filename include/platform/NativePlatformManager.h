#ifndef NATIVEPLATFORMMANAGER_H
#define NATIVEPLATFORMMANAGER_H

#include <QObject>
#include <QWindow>
#include <memory>

// Forward declarations
class QMenu;
class QAction;

namespace ZenRunner {
    class NativeEffects;
}

namespace ZenRunner::Platform {
    class SystemTray;
    enum class TrayIconState;
}

namespace ZenRunner::Platform {

/**
 * @brief Centralized manager for native platform features
 * 
 * Manages native effects (Mica/Vibrancy) and system tray integration.
 * Provides a unified interface for the main application.
 */
class NativePlatformManager : public QObject
{
    Q_OBJECT

public:
    explicit NativePlatformManager(QObject *parent = nullptr);
    ~NativePlatformManager() override;

    /**
     * @brief Initialize native effects for the given window
     * @param window Main application window
     * @return true if effects were applied successfully
     */
    bool initializeNativeEffects(QWindow *window);

    /**
     * @brief Initialize system tray icon
     * @return true if tray icon was shown successfully
     */
    bool initializeSystemTray();

    /**
     * @brief Update system tray state
     * @param state New tray icon state
     */
    void setTrayState(TrayIconState state);

    /**
     * @brief Show a notification from the system tray
     * @param title Notification title
     * @param message Notification message
     */
    void showNotification(const QString& title, const QString& message);

    /**
     * @brief Get the system tray instance
     * @return System tray pointer (may be nullptr if not initialized)
     */
    SystemTray* getSystemTray() const { return m_systemTray.get(); }

    /**
     * @brief Get the native effects instance
     * @return Native effects pointer (may be nullptr if not initialized)
     */
    NativeEffects* getNativeEffects() const { return m_nativeEffects.get(); }

signals:
    /**
     * @brief Emitted when the tray icon is clicked
     */
    void trayIconActivated();

    /**
     * @brief Emitted when show action is triggered
     */
    void showRequested();

    /**
     * @brief Emitted when quit action is triggered
     */
    void quitRequested();

private slots:
    void handleTrayActivated();
    void handleShowAction();
    void handleQuitAction();

private:
    void createTrayMenu();

    std::unique_ptr<NativeEffects> m_nativeEffects;
    std::unique_ptr<SystemTray> m_systemTray;
    QMenu* m_trayMenu;
    QAction* m_showAction;
    QAction* m_quitAction;
};

} // namespace ZenRunner::Platform

#endif // NATIVEPLATFORMMANAGER_H
