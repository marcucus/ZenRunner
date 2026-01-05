#include "GenericNativeEffects.h"

GenericNativeEffects::GenericNativeEffects(QObject *parent)
    : NativeEffects(parent)
{
}

bool GenericNativeEffects::applyEffect([[maybe_unused]] QWindow *window, 
                                       [[maybe_unused]] Effect effect)
{
    // No native effects available on this platform
    // QML glassmorphism effects will be used instead
    return false;
}

void GenericNativeEffects::removeEffect([[maybe_unused]] QWindow *window)
{
    // No-op
}

bool GenericNativeEffects::isEffectSupported([[maybe_unused]] Effect effect) const
{
    // No effects supported in generic implementation
    return false;
}
