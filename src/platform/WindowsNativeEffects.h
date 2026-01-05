#ifndef WINDOWSNATIVEEFFECTS_H
#define WINDOWSNATIVEEFFECTS_H

#include "platform/NativeEffects.h"

#ifdef Q_OS_WIN

/**
 * @brief Windows-specific implementation for Mica effect
 * 
 * Uses DWM (Desktop Window Manager) API to apply Windows 11 Mica material.
 * Mica samples the desktop wallpaper to create a translucent, blurred background.
 */
class WindowsNativeEffects : public NativeEffects
{
    Q_OBJECT
    
public:
    explicit WindowsNativeEffects(QObject *parent = nullptr);
    ~WindowsNativeEffects() override = default;
    
    bool applyEffect(QWindow *window, Effect effect) override;
    void removeEffect(QWindow *window) override;
    bool isEffectSupported(Effect effect) const override;
    
private:
    bool applyMica(QWindow *window, bool altVariant = false);
    bool isWindows11OrGreater() const;
};

#endif // Q_OS_WIN

#endif // WINDOWSNATIVEEFFECTS_H
