#pragma once

#include <QObject>
#include <QString>

namespace ZenRunner::Platform {

/**
 * @brief Platform-specific visual effect types
 */
enum class EffectType {
    None,           ///< No effect
    Mica,           ///< Windows 11 Mica material
    Acrylic,        ///< Windows 10/11 Acrylic material
    Vibrancy,       ///< macOS Vibrancy effect
    Blur            ///< Generic blur effect
};

/**
 * @brief Effect theme/appearance
 */
enum class EffectTheme {
    Auto,           ///< Follow system theme
    Light,          ///< Light appearance
    Dark            ///< Dark appearance
};

/**
 * @brief Interface for platform-specific visual effects
 * 
 * This interface defines the contract for applying OS-native visual effects
 * such as Mica on Windows and Vibrancy on macOS.
 */
class IPlatformEffect {
public:
    virtual ~IPlatformEffect() = default;

    /**
     * @brief Check if a specific effect is supported on current platform
     * @param effect Effect type to check
     * @return true if effect is supported
     */
    virtual bool isEffectSupported(EffectType effect) const = 0;

    /**
     * @brief Get the best available effect for the current platform
     * @return Recommended effect type
     */
    virtual EffectType getBestAvailableEffect() const = 0;

    /**
     * @brief Apply a visual effect to a window
     * @param windowId Native window identifier
     * @param effect Effect type to apply
     * @param theme Theme/appearance for the effect
     * @return true if effect was applied successfully
     */
    virtual bool applyEffect(
        quintptr windowId,
        EffectType effect,
        EffectTheme theme = EffectTheme::Auto
    ) = 0;

    /**
     * @brief Remove all effects from a window
     * @param windowId Native window identifier
     * @return true if effects were removed
     */
    virtual bool removeEffect(quintptr windowId) = 0;

    /**
     * @brief Update the theme of an applied effect
     * @param windowId Native window identifier
     * @param theme New theme
     * @return true if theme was updated
     */
    virtual bool updateTheme(quintptr windowId, EffectTheme theme) = 0;

    /**
     * @brief Get the currently applied effect type
     * @param windowId Native window identifier
     * @return Current effect type
     */
    virtual EffectType getCurrentEffect(quintptr windowId) const = 0;

    /**
     * @brief Check if system is in dark mode
     * @return true if dark mode is active
     */
    virtual bool isSystemDarkMode() const = 0;
};

} // namespace ZenRunner::Platform
