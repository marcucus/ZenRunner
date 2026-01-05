#include "WindowsNativeEffects.h"

#ifdef Q_OS_WIN

#include <windows.h>
#include <dwmapi.h>
#include <QOperatingSystemVersion>

// Windows 11 build number
constexpr int WINDOWS_11_BUILD = 22000;

// DWM attributes for Mica (Windows 11 22H2+)
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

enum DWM_SYSTEMBACKDROP_TYPE {
    DWMSBT_AUTO = 0,
    DWMSBT_NONE = 1,
    DWMSBT_MAINWINDOW = 2,      // Mica
    DWMSBT_TRANSIENTWINDOW = 3, // Mica Alt
    DWMSBT_TABBEDWINDOW = 4     // Tabbed
};

WindowsNativeEffects::WindowsNativeEffects(QObject *parent)
    : NativeEffects(parent)
{
}

bool WindowsNativeEffects::applyEffect(QWindow *window, Effect effect)
{
    if (!window) {
        return false;
    }
    
    switch (effect) {
        case Effect::Mica:
            return applyMica(window, false);
        case Effect::MicaAlt:
            return applyMica(window, true);
        default:
            return false;
    }
}

void WindowsNativeEffects::removeEffect(QWindow *window)
{
    if (!window) {
        return;
    }
    
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_NONE;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
}

bool WindowsNativeEffects::isEffectSupported(Effect effect) const
{
    if (!isWindows11OrGreater()) {
        return false;
    }
    
    return (effect == Effect::Mica || effect == Effect::MicaAlt);
}

bool WindowsNativeEffects::applyMica(QWindow *window, bool altVariant)
{
    if (!isWindows11OrGreater()) {
        return false;
    }
    
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return false;
    }
    
    // Set the backdrop type
    DWM_SYSTEMBACKDROP_TYPE backdrop = altVariant ? 
        DWMSBT_TRANSIENTWINDOW : DWMSBT_MAINWINDOW;
    
    HRESULT hr = DwmSetWindowAttribute(
        hwnd,
        DWMWA_SYSTEMBACKDROP_TYPE,
        &backdrop,
        sizeof(backdrop)
    );
    
    return SUCCEEDED(hr);
}

bool WindowsNativeEffects::isWindows11OrGreater() const
{
    auto version = QOperatingSystemVersion::current();
    return (version >= QOperatingSystemVersion::Windows11);
}

#endif // Q_OS_WIN
