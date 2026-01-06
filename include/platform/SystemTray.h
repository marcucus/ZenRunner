#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include "platform/ISystemTray.hpp"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QObject>
#include <memory>

namespace ZenRunner::Platform {

/**
 * @brief Cross-platform system tray implementation
 * 
 * Provides system tray icon with context menu and notifications.
 * Uses QSystemTrayIcon for cross-platform compatibility.
 */
class SystemTray : public QObject, public ISystemTray
{
    Q_OBJECT

public:
    explicit SystemTray(QObject *parent = nullptr);
    ~SystemTray() override;

    // ISystemTray interface
    bool isSystemTrayAvailable() const override;
    bool show() override;
    void hide() override;
    bool isVisible() const override;
    void setIcon(const QIcon& icon) override;
    void setState(TrayIconState state) override;
    TrayIconState getState() const override;
    void setTooltip(const QString& tooltip) override;
    QString getTooltip() const override;
    void setContextMenu(QMenu* menu) override;
    void showMessage(const QString& title, const QString& message, int durationMs = 0) override;
    void onActivated(std::function<void()> callback) override;
    void onMessageClicked(std::function<void()> callback) override;

signals:
    void activated();
    void messageClicked();

private slots:
    void handleActivated(QSystemTrayIcon::ActivationReason reason);
    void handleMessageClicked();

private:
    QIcon getIconForState(TrayIconState state) const;
    QIcon createColoredIcon(const QColor& color) const;

    std::unique_ptr<QSystemTrayIcon> m_trayIcon;
    QMenu* m_contextMenu;
    TrayIconState m_currentState;
    std::function<void()> m_activatedCallback;
    std::function<void()> m_messageClickedCallback;
};

} // namespace ZenRunner::Platform

#endif // SYSTEMTRAY_H
