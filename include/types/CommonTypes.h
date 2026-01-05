#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QProcessEnvironment>
#include <concepts>
#include <optional>
#include <memory>
#include <functional>

namespace ZenRunner {

// C++20 Concepts for type safety
template<typename T>
concept Stringlike = std::convertible_to<T, QString> || std::convertible_to<T, std::string>;

template<typename T>
concept Callable = std::is_invocable_v<T>;

template<typename T, typename R, typename... Args>
concept CallableWith = std::is_invocable_r_v<R, T, Args...>;

// Process states
enum class ProcessState {
    NotStarted,
    Starting,
    Running,
    Paused,
    Stopping,
    Stopped,
    Crashed,
    Finished
};

// Process priority levels
enum class ProcessPriority {
    Low,
    Normal,
    High,
    Critical
};

// Log entry types
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Result type for operations that can fail
template<typename T, typename E = QString>
class Result {
public:
    static Result Ok(T value) {
        Result r;
        r.value_ = std::move(value);
        r.is_ok_ = true;
        return r;
    }

    static Result Err(E error) {
        Result r;
        r.error_ = std::move(error);
        r.is_ok_ = false;
        return r;
    }

    [[nodiscard]] bool isOk() const noexcept { return is_ok_; }
    [[nodiscard]] bool isErr() const noexcept { return !is_ok_; }

    /**
     * @brief Get the success value
     * 
     * @warning Precondition: isOk() must be true, otherwise throws std::bad_optional_access
     * @note Always check isOk() before calling this method
     */
    [[nodiscard]] T& value() & { return value_.value(); }
    [[nodiscard]] const T& value() const & { return value_.value(); }
    [[nodiscard]] T&& value() && { return std::move(value_.value()); }

    /**
     * @brief Get the error value
     * 
     * @warning Precondition: isErr() must be true, otherwise throws std::bad_optional_access
     * @note Always check isErr() before calling this method
     */
    [[nodiscard]] E& error() & { return error_.value(); }
    [[nodiscard]] const E& error() const & { return error_.value(); }
    [[nodiscard]] E&& error() && { return std::move(error_.value()); }

    [[nodiscard]] std::optional<T> ok() const {
        if (is_ok_) return value_;
        return std::nullopt;
    }

    [[nodiscard]] std::optional<E> err() const {
        if (!is_ok_) return error_;
        return std::nullopt;
    }

private:
    std::optional<T> value_;
    std::optional<E> error_;
    bool is_ok_ = false;
};

// Log entry structure
struct LogEntry {
    QString text;
    LogLevel level = LogLevel::Info;
    qint64 timestamp = 0;
    bool isStderr = false;

    LogEntry() = default;
    LogEntry(QString text_, LogLevel level_ = LogLevel::Info, bool isStderr_ = false)
        : text(std::move(text_)), level(level_), isStderr(isStderr_) {}
};

// Process configuration
struct ProcessConfig {
    QString command;
    QStringList arguments;
    QString workingDirectory;
    QProcessEnvironment environment;
    ProcessPriority priority = ProcessPriority::Normal;
    bool captureOutput = true;
    bool mergeChannels = false;
    int maxLogLines = 5000;

    ProcessConfig() = default;
    explicit ProcessConfig(QString cmd) : command(std::move(cmd)) {}
};

} // namespace ZenRunner
