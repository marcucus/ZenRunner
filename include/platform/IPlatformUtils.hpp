#pragma once

#include <QRect>
#include <QString>

namespace ZenRunner::Platform {

/**
 * @brief Platform-specific information
 */
enum class PlatformType {
    Unknown,
    Windows,
    MacOS,
    Linux
};

/**
 * @brief Screen information for notch detection
 */
struct ScreenInfo {
    QRect totalArea;        ///< Total screen area including notch
    QRect safeArea;         ///< Safe area excluding notch and system UI
    bool hasNotch{false};   ///< Whether screen has a notch
    QRect notchArea;        ///< Area occupied by the notch (if present)
};

/**
 * @brief Interface for platform-specific utilities
 * 
 * This interface provides access to platform-specific features and information.
 */
class IPlatformUtils {
public:
    virtual ~IPlatformUtils() = default;

    /**
     * @brief Get the current platform type
     * @return Platform type
     */
    virtual PlatformType getPlatform() const = 0;

    /**
     * @brief Get the platform name as a string
     * @return Platform name (e.g., "Windows", "macOS", "Linux")
     */
    virtual QString getPlatformName() const = 0;

    /**
     * @brief Get the OS version string
     * @return OS version (e.g., "Windows 11", "macOS 14.0")
     */
    virtual QString getOSVersion() const = 0;

    /**
     * @brief Check if running on Windows 11 or later
     * @return true if Windows 11+
     */
    virtual bool isWindows11OrLater() const = 0;

    /**
     * @brief Check if running on macOS with notch support
     * @return true if macOS with potential notch
     */
    virtual bool isMacOSWithNotch() const = 0;

    /**
     * @brief Get screen information including notch detection
     * @param screenIndex Screen index (0 for primary)
     * @return Screen information
     */
    virtual ScreenInfo getScreenInfo(int screenIndex = 0) const = 0;

    /**
     * @brief Check if system supports transparency effects
     * @return true if transparency is supported
     */
    virtual bool supportsTransparency() const = 0;

    /**
     * @brief Check if running in dark mode
     * @return true if dark mode is active
     */
    virtual bool isDarkMode() const = 0;

    /**
     * @brief Get the accent color from the system
     * @return Accent color as hex string (e.g., "#0078D4")
     */
    virtual QString getSystemAccentColor() const = 0;

    /**
     * @brief Open a file or URL with the default system application
     * @param path File path or URL to open
     * @return true if opened successfully
     */
    virtual bool openWithDefaultApplication(const QString& path) = 0;

    /**
     * @brief Reveal a file in the system file explorer
     * @param filePath Path to the file
     * @return true if revealed successfully
     */
    virtual bool revealInFileExplorer(const QString& filePath) = 0;
};

} // namespace ZenRunner::Platform
