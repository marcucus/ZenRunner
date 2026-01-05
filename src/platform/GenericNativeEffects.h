#ifndef GENERICNATIVEEFFECTS_H
#define GENERICNATIVEEFFECTS_H

#include "platform/NativeEffects.h"

/**
 * @brief Generic fallback implementation for unsupported platforms
 * 
 * Provides no-op implementations for platforms that don't support
 * native material effects (Linux, older Windows/macOS versions).
 */
class GenericNativeEffects : public NativeEffects
{
    Q_OBJECT
    
public:
    explicit GenericNativeEffects(QObject *parent = nullptr);
    ~GenericNativeEffects() override = default;
    
    bool applyEffect(QWindow *window, Effect effect) override;
    void removeEffect(QWindow *window) override;
    bool isEffectSupported(Effect effect) const override;
};

#endif // GENERICNATIVEEFFECTS_H
