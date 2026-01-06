#include "platform/SystemTray.h"
#include <QPainter>
#include <QPixmap>

namespace ZenRunner::Platform {

SystemTray::SystemTray(QObject *parent)
    : QObject(parent)
    , m_trayIcon(std::make_unique<QSystemTrayIcon>(this))
    , m_contextMenu(nullptr)
    , m_currentState(TrayIconState::Idle)
    , m_activatedCallback(nullptr)
    , m_messageClickedCallback(nullptr)
{
    // Connect signals
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated,
            this, &SystemTray::handleActivated);
    connect(m_trayIcon.get(), &QSystemTrayIcon::messageClicked,
            this, &SystemTray::handleMessageClicked);

    // Set initial icon
    setState(TrayIconState::Idle);
}

SystemTray::~SystemTray()
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->hide();
    }
}

bool SystemTray::isSystemTrayAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool SystemTray::show()
{
    if (!isSystemTrayAvailable()) {
        return false;
    }

    m_trayIcon->show();
    return m_trayIcon->isVisible();
}

void SystemTray::hide()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool SystemTray::isVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

void SystemTray::setIcon(const QIcon& icon)
{
    if (m_trayIcon) {
        m_trayIcon->setIcon(icon);
    }
}

void SystemTray::setState(TrayIconState state)
{
    m_currentState = state;
    setIcon(getIconForState(state));
    
    // Update tooltip based on state
    QString tooltip = "ZenRunner";
    switch (state) {
        case TrayIconState::Idle:
            tooltip += " - Idle";
            break;
        case TrayIconState::Active:
            tooltip += " - Active";
            break;
        case TrayIconState::Error:
            tooltip += " - Error";
            break;
        case TrayIconState::Warning:
            tooltip += " - Warning";
            break;
    }
    setTooltip(tooltip);
}

TrayIconState SystemTray::getState() const
{
    return m_currentState;
}

void SystemTray::setTooltip(const QString& tooltip)
{
    if (m_trayIcon) {
        m_trayIcon->setToolTip(tooltip);
    }
}

QString SystemTray::getTooltip() const
{
    return m_trayIcon ? m_trayIcon->toolTip() : QString();
}

void SystemTray::setContextMenu(QMenu* menu)
{
    m_contextMenu = menu;
    if (m_trayIcon) {
        m_trayIcon->setContextMenu(menu);
    }
}

void SystemTray::showMessage(const QString& title, const QString& message, int durationMs)
{
    if (m_trayIcon && isVisible()) {
        QSystemTrayIcon::MessageIcon icon;
        switch (m_currentState) {
            case TrayIconState::Error:
                icon = QSystemTrayIcon::Critical;
                break;
            case TrayIconState::Warning:
                icon = QSystemTrayIcon::Warning;
                break;
            default:
                icon = QSystemTrayIcon::Information;
                break;
        }
        
        m_trayIcon->showMessage(title, message, icon, durationMs);
    }
}

void SystemTray::onActivated(std::function<void()> callback)
{
    m_activatedCallback = callback;
}

void SystemTray::onMessageClicked(std::function<void()> callback)
{
    m_messageClickedCallback = callback;
}

void SystemTray::handleActivated(QSystemTrayIcon::ActivationReason reason)
{
    // Only trigger on left click or double click (not on context menu)
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        emit activated();
        if (m_activatedCallback) {
            m_activatedCallback();
        }
    }
}

void SystemTray::handleMessageClicked()
{
    emit messageClicked();
    if (m_messageClickedCallback) {
        m_messageClickedCallback();
    }
}

QIcon SystemTray::getIconForState(TrayIconState state) const
{
    QColor color;
    switch (state) {
        case TrayIconState::Idle:
            color = QColor(128, 128, 128); // Gray
            break;
        case TrayIconState::Active:
            color = QColor(0, 200, 0); // Green
            break;
        case TrayIconState::Error:
            color = QColor(255, 0, 0); // Red
            break;
        case TrayIconState::Warning:
            color = QColor(255, 165, 0); // Orange
            break;
    }
    
    return createColoredIcon(color);
}

QIcon SystemTray::createColoredIcon(const QColor& color) const
{
    // Create a simple circular icon with the given color
    const int size = 64;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw outer circle (border)
    painter.setPen(QPen(color.darker(120), 2));
    painter.setBrush(color);
    painter.drawEllipse(4, 4, size - 8, size - 8);
    
    // Draw inner highlight for depth
    QRadialGradient gradient(size * 0.4, size * 0.4, size * 0.4);
    gradient.setColorAt(0, color.lighter(130));
    gradient.setColorAt(1, color);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(6, 6, size - 12, size - 12);
    
    return QIcon(pixmap);
}

} // namespace ZenRunner::Platform
