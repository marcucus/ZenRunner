#ifndef NATIVEEFFECTS_H
#define NATIVEEFFECTS_H

#include <QObject>
#include <QWindow>

/**
 * @brief Abstract interface for native platform effects (Mica/Vibrancy)
 * 
 * This class provides a platform-agnostic interface for applying native
 * material effects to windows, following the glassmorphism design philosophy.
 */
class NativeEffects : public QObject
{
    Q_OBJECT
    
public:
    enum class Effect {
        None,
        Mica,           // Windows 11 Mica effect
        MicaAlt,        // Windows 11 Mica Alt (darker variant)
        Vibrancy,       // macOS Vibrancy effect
        Blur            // Generic blur fallback
    };
    Q_ENUM(Effect)
    
    explicit NativeEffects(QObject *parent = nullptr);
    virtual ~NativeEffects() = default;
    
    /**
     * @brief Apply native material effect to window
     * @param window Target QWindow
     * @param effect Effect type to apply
     * @return true if effect was successfully applied
     */
    virtual bool applyEffect(QWindow *window, Effect effect) = 0;
    
    /**
     * @brief Remove native effect from window
     * @param window Target QWindow
     */
    virtual void removeEffect(QWindow *window) = 0;
    
    /**
     * @brief Check if effect is supported on current platform
     * @param effect Effect type to check
     * @return true if effect is supported
     */
    virtual bool isEffectSupported(Effect effect) const = 0;
    
    /**
     * @brief Create platform-specific implementation
     * @return Platform-specific NativeEffects instance
     */
    static NativeEffects* create(QObject *parent = nullptr);
};

#endif // NATIVEEFFECTS_H
