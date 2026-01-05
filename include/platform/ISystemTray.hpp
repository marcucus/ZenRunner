#pragma once

#include <QObject>
#include <QString>
#include <QIcon>
#include <QMenu>
#include <functional>
#include <memory>

namespace ZenRunner::Platform {

/**
 * @brief System tray icon state
 */
enum class TrayIconState {
    Idle,       ///< Application is idle
    Active,     ///< Processes are running
    Error,      ///< One or more processes have errors
    Warning     ///< Warning state
};

/**
 * @brief Interface for system tray integration
 * 
 * This interface defines the contract for system tray / menu bar integration
 * across different platforms (Windows, macOS, Linux).
 */
class ISystemTray {
public:
    virtual ~ISystemTray() = default;

    /**
     * @brief Check if system tray is available on current platform
     * @return true if system tray is available
     */
    virtual bool isSystemTrayAvailable() const = 0;

    /**
     * @brief Show the system tray icon
     * @return true if icon was shown successfully
     */
    virtual bool show() = 0;

    /**
     * @brief Hide the system tray icon
     */
    virtual void hide() = 0;

    /**
     * @brief Check if the tray icon is currently visible
     * @return true if visible
     */
    virtual bool isVisible() const = 0;

    /**
     * @brief Set the tray icon
     * @param icon Icon to display
     */
    virtual void setIcon(const QIcon& icon) = 0;

    /**
     * @brief Set the tray icon state (updates icon automatically)
     * @param state New icon state
     */
    virtual void setState(TrayIconState state) = 0;

    /**
     * @brief Get the current tray icon state
     * @return Current state
     */
    virtual TrayIconState getState() const = 0;

    /**
     * @brief Set the tooltip text
     * @param tooltip Tooltip text to display
     */
    virtual void setTooltip(const QString& tooltip) = 0;

    /**
     * @brief Get the current tooltip text
     * @return Tooltip text
     */
    virtual QString getTooltip() const = 0;

    /**
     * @brief Set the context menu
     * @param menu Menu to display on right-click
     */
    virtual void setContextMenu(QMenu* menu) = 0;

    /**
     * @brief Show a message notification from the tray icon
     * @param title Notification title
     * @param message Notification message
     * @param durationMs Duration in milliseconds (0 for default)
     */
    virtual void showMessage(
        const QString& title,
        const QString& message,
        int durationMs = 0
    ) = 0;

    /**
     * @brief Register a callback for icon activation (click)
     * @param callback Function to call when icon is clicked
     */
    virtual void onActivated(std::function<void()> callback) = 0;

    /**
     * @brief Register a callback for message clicked
     * @param callback Function to call when notification is clicked
     */
    virtual void onMessageClicked(std::function<void()> callback) = 0;
};

} // namespace ZenRunner::Platform
