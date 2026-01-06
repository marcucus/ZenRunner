#ifndef PLATFORMHELPER_H
#define PLATFORMHELPER_H

#include <QObject>
#include <QWindow>

namespace ZenRunner::Platform {

/**
 * @brief Helper class to expose platform-specific information to QML
 * 
 * Provides QML-accessible properties for platform detection and
 * safe area insets (for notch detection on macOS).
 */
class PlatformHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasNotch READ hasNotch NOTIFY notchChanged)
    Q_PROPERTY(int notchSafeAreaTop READ notchSafeAreaTop NOTIFY notchChanged)
    Q_PROPERTY(int notchSafeAreaBottom READ notchSafeAreaBottom NOTIFY notchChanged)
    Q_PROPERTY(int notchSafeAreaLeft READ notchSafeAreaLeft NOTIFY notchChanged)
    Q_PROPERTY(int notchSafeAreaRight READ notchSafeAreaRight NOTIFY notchChanged)
    Q_PROPERTY(QString platformName READ platformName CONSTANT)
    Q_PROPERTY(bool isWindows READ isWindows CONSTANT)
    Q_PROPERTY(bool isMacOS READ isMacOS CONSTANT)
    Q_PROPERTY(bool isLinux READ isLinux CONSTANT)

public:
    explicit PlatformHelper(QObject *parent = nullptr);
    ~PlatformHelper() override = default;

    // Notch detection (macOS)
    bool hasNotch() const { return m_hasNotch; }
    int notchSafeAreaTop() const { return m_safeAreaTop; }
    int notchSafeAreaBottom() const { return m_safeAreaBottom; }
    int notchSafeAreaLeft() const { return m_safeAreaLeft; }
    int notchSafeAreaRight() const { return m_safeAreaRight; }

    // Platform detection
    QString platformName() const;
    bool isWindows() const;
    bool isMacOS() const;
    bool isLinux() const;

public slots:
    /**
     * @brief Update safe area insets for the given window
     * @param window Window to check
     */
    void updateSafeAreaInsets(QWindow *window);

signals:
    void notchChanged();

private:
    bool m_hasNotch;
    int m_safeAreaTop;
    int m_safeAreaBottom;
    int m_safeAreaLeft;
    int m_safeAreaRight;
};

} // namespace ZenRunner::Platform

#endif // PLATFORMHELPER_H
