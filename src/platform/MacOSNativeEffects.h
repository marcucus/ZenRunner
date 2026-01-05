#ifndef MACOSNATIVEEFFECTS_H
#define MACOSNATIVEEFFECTS_H

#include "platform/NativeEffects.h"

#ifdef Q_OS_MACOS

/**
 * @brief macOS-specific implementation for Vibrancy effect
 * 
 * Uses NSVisualEffectView to apply native macOS vibrancy materials.
 * Integrates with system appearance (light/dark mode).
 */
class MacOSNativeEffects : public NativeEffects
{
    Q_OBJECT
    
public:
    explicit MacOSNativeEffects(QObject *parent = nullptr);
    ~MacOSNativeEffects() override = default;
    
    bool applyEffect(QWindow *window, Effect effect) override;
    void removeEffect(QWindow *window) override;
    bool isEffectSupported(Effect effect) const override;
    
    /**
     * @brief Detect MacBook Pro notch safe area
     * @param window Target window
     * @return Safe area insets (top, left, bottom, right)
     */
    static QMargins getSafeAreaInsets(QWindow *window);
    
private:
    bool applyVibrancy(QWindow *window);
};

#endif // Q_OS_MACOS

#endif // MACOSNATIVEEFFECTS_H
