#pragma once

/**
 * @file CommonTypes.hpp
 * @brief Common type definitions and enumerations used across all layers
 * 
 * This file contains shared types that are used by multiple layers but don't
 * belong to any specific layer interface.
 */

#include <QString>
#include <cstdint>

namespace ZenRunner::Types {

/**
 * @brief Result type for operations that can succeed or fail
 */
template<typename T>
struct Result {
    bool success{false};       ///< Whether operation succeeded
    T value;                   ///< Result value (valid only if success is true)
    QString errorMessage;      ///< Error message if operation failed

    /**
     * @brief Check if operation was successful
     */
    [[nodiscard]] bool isSuccess() const { return success; }

    /**
     * @brief Check if operation failed
     */
    [[nodiscard]] bool isError() const { return !success; }

    /**
     * @brief Create a successful result
     */
    static Result<T> Success(T val) {
        return Result<T>{true, std::move(val), QString()};
    }

    /**
     * @brief Create a failed result
     */
    static Result<T> Error(const QString& error) {
        return Result<T>{false, T(), error};
    }
};

/**
 * @brief Specialization for void operations
 */
template<>
struct Result<void> {
    bool success{false};
    QString errorMessage;

    [[nodiscard]] bool isSuccess() const { return success; }
    [[nodiscard]] bool isError() const { return !success; }

    static Result<void> Success() {
        return Result<void>{true, QString()};
    }

    static Result<void> Error(const QString& error) {
        return Result<void>{false, error};
    }
};

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    Debug,      ///< Debug information
    Info,       ///< Informational messages
    Warning,    ///< Warning messages
    Error,      ///< Error messages
    Critical    ///< Critical error messages
};

/**
 * @brief Application theme
 */
enum class Theme {
    Auto,       ///< Follow system theme
    Light,      ///< Light theme
    Dark        ///< Dark theme
};

/**
 * @brief Window state
 */
enum class WindowState {
    Normal,     ///< Normal window
    Minimized,  ///< Minimized to taskbar
    Maximized,  ///< Maximized window
    FullScreen, ///< Full screen mode
    Hidden      ///< Hidden (system tray only)
};

} // namespace ZenRunner::Types
