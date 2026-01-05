#include "platform/NativeEffects.h"

NativeEffects::NativeEffects(QObject *parent)
    : QObject(parent)
{
}

// Platform-specific implementations will be in separate files
#ifdef Q_OS_WIN
#include "WindowsNativeEffects.h"
NativeEffects* NativeEffects::create(QObject *parent)
{
    return new WindowsNativeEffects(parent);
}
#elif defined(Q_OS_MACOS) || defined(Q_OS_MAC)
#include "MacOSNativeEffects.h"
NativeEffects* NativeEffects::create(QObject *parent)
{
    return new MacOSNativeEffects(parent);
}
#else
#include "GenericNativeEffects.h"
NativeEffects* NativeEffects::create(QObject *parent)
{
    return new GenericNativeEffects(parent);
}
#endif
