#include "MacOSNativeEffects.h"

#ifdef Q_OS_MACOS

#import <AppKit/AppKit.h>
#include <QMargins>

MacOSNativeEffects::MacOSNativeEffects(QObject *parent)
    : NativeEffects(parent)
{
}

bool MacOSNativeEffects::applyEffect(QWindow *window, Effect effect)
{
    if (!window) {
        return false;
    }
    
    if (effect == Effect::Vibrancy) {
        return applyVibrancy(window);
    }
    
    return false;
}

void MacOSNativeEffects::removeEffect(QWindow *window)
{
    if (!window) {
        return;
    }
    
    NSView *view = reinterpret_cast<NSView *>(window->winId());
    if (!view) {
        return;
    }
    
    NSWindow *nsWindow = [view window];
    if (!nsWindow) {
        return;
    }
    
    // Remove vibrancy by setting opaque background
    [nsWindow setOpaque:YES];
    [nsWindow setBackgroundColor:[NSColor windowBackgroundColor]];
}

bool MacOSNativeEffects::isEffectSupported(Effect effect) const
{
    // Vibrancy is supported on macOS 10.10+
    return (effect == Effect::Vibrancy);
}

bool MacOSNativeEffects::applyVibrancy(QWindow *window)
{
    @autoreleasepool {
        NSView *view = reinterpret_cast<NSView *>(window->winId());
        if (!view) {
            return false;
        }
        
        NSWindow *nsWindow = [view window];
        if (!nsWindow) {
            return false;
        }
        
        // Make window transparent
        [nsWindow setOpaque:NO];
        [nsWindow setBackgroundColor:[NSColor clearColor]];
        
        // Create and configure NSVisualEffectView
        NSVisualEffectView *effectView = [[[NSVisualEffectView alloc] 
            initWithFrame:[view bounds]] autorelease];
        
        // Configure vibrancy material
        [effectView setMaterial:NSVisualEffectMaterialHUDWindow];
        [effectView setState:NSVisualEffectStateFollowsWindowActiveState];
        [effectView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
        
        // Auto-resize with window
        [effectView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        
        // Insert as bottom layer (this will retain effectView)
        [view addSubview:effectView positioned:NSWindowBelow relativeTo:nil];
        
        return true;
    }
}

QMargins MacOSNativeEffects::getSafeAreaInsets(QWindow *window)
{
    @autoreleasepool {
        if (!window) {
            return QMargins();
        }
        
        NSView *view = reinterpret_cast<NSView *>(window->winId());
        if (!view) {
            return QMargins();
        }
        
        NSWindow *nsWindow = [view window];
        if (!nsWindow) {
            return QMargins();
        }
        
        NSScreen *screen = [nsWindow screen];
        if (!screen) {
            return QMargins();
        }
        
        // Get safe area insets (for notch detection)
        if (@available(macOS 12.0, *)) {
            NSEdgeInsets insets = [screen safeAreaInsets];
            return QMargins(
                static_cast<int>(insets.left),
                static_cast<int>(insets.top),
                static_cast<int>(insets.right),
                static_cast<int>(insets.bottom)
            );
        }
        
        return QMargins();
    }
}

#endif // Q_OS_MACOS
