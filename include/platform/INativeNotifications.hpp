#pragma once

#include <QString>
#include <QIcon>
#include <functional>

namespace ZenRunner::Platform {

/**
 * @brief Notification priority level
 */
enum class NotificationPriority {
    Low,        ///< Low priority notification
    Normal,     ///< Normal priority notification
    High,       ///< High priority notification
    Critical    ///< Critical notification
};

/**
 * @brief Notification action button
 */
struct NotificationAction {
    QString id;         ///< Unique action identifier
    QString label;      ///< Button label
};

/**
 * @brief Interface for native system notifications
 * 
 * This interface defines the contract for sending native system notifications
 * across different platforms.
 */
class INativeNotifications {
public:
    virtual ~INativeNotifications() = default;

    /**
     * @brief Check if native notifications are supported
     * @return true if notifications are supported
     */
    virtual bool isSupported() const = 0;

    /**
     * @brief Request notification permissions (if required by platform)
     * @return true if permissions granted or not required
     */
    virtual bool requestPermission() = 0;

    /**
     * @brief Check if notifications are currently enabled
     * @return true if notifications are enabled
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Show a simple notification
     * @param title Notification title
     * @param message Notification message
     * @param priority Notification priority
     * @return Notification ID for tracking
     */
    virtual QString showNotification(
        const QString& title,
        const QString& message,
        NotificationPriority priority = NotificationPriority::Normal
    ) = 0;

    /**
     * @brief Show a notification with an icon
     * @param title Notification title
     * @param message Notification message
     * @param icon Notification icon
     * @param priority Notification priority
     * @return Notification ID for tracking
     */
    virtual QString showNotification(
        const QString& title,
        const QString& message,
        const QIcon& icon,
        NotificationPriority priority = NotificationPriority::Normal
    ) = 0;

    /**
     * @brief Show a notification with action buttons
     * @param title Notification title
     * @param message Notification message
     * @param actions List of action buttons
     * @param priority Notification priority
     * @return Notification ID for tracking
     */
    virtual QString showNotificationWithActions(
        const QString& title,
        const QString& message,
        const std::vector<NotificationAction>& actions,
        NotificationPriority priority = NotificationPriority::Normal
    ) = 0;

    /**
     * @brief Close a notification
     * @param notificationId ID of notification to close
     */
    virtual void closeNotification(const QString& notificationId) = 0;

    /**
     * @brief Register a callback for notification clicked
     * @param callback Function to call with notification ID
     */
    virtual void onNotificationClicked(
        std::function<void(const QString& notificationId)> callback
    ) = 0;

    /**
     * @brief Register a callback for notification action clicked
     * @param callback Function to call with notification ID and action ID
     */
    virtual void onActionClicked(
        std::function<void(const QString& notificationId, const QString& actionId)> callback
    ) = 0;

    /**
     * @brief Register a callback for notification closed
     * @param callback Function to call with notification ID
     */
    virtual void onNotificationClosed(
        std::function<void(const QString& notificationId)> callback
    ) = 0;
};

} // namespace ZenRunner::Platform
