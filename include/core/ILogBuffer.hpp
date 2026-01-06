#pragma once

#include <QString>
#include <QStringList>
#include <cstddef>
#include <functional>
#include "IAnsiParser.hpp"

namespace ZenRunner::Core {

/**
 * @brief Log entry structure with ANSI color support
 */
struct LogEntry {
    QString text;                           ///< Log text content (with or without ANSI codes)
    QString plainText;                      ///< Plain text without ANSI codes
    qint64 timestamp;                       ///< Timestamp (milliseconds since epoch)
    bool isError{false};                    ///< Whether this is an error message
    std::vector<StyledSegment> segments;    ///< Styled text segments (parsed ANSI)
    bool hasAnsiCodes{false};               ///< Whether the original text had ANSI codes
};

/**
 * @brief Interface for circular buffer log storage
 * 
 * This interface defines the contract for memory-efficient log storage
 * with automatic overflow management using a circular buffer.
 */
class ILogBuffer {
public:
    virtual ~ILogBuffer() = default;

    /**
     * @brief Add a log entry to the buffer
     * @param entry Log entry to add
     */
    virtual void append(const LogEntry& entry) = 0;

    /**
     * @brief Add a log line to the buffer
     * @param text Log text
     * @param isError Whether this is an error message
     */
    virtual void append(const QString& text, bool isError = false) = 0;

    /**
     * @brief Get all log entries in the buffer
     * @return Vector of log entries in chronological order
     */
    virtual std::vector<LogEntry> getAll() const = 0;

    /**
     * @brief Get the most recent N log entries
     * @param count Number of entries to retrieve
     * @return Vector of log entries
     */
    virtual std::vector<LogEntry> getRecent(size_t count) const = 0;

    /**
     * @brief Get log entries within a time range
     * @param startTime Start timestamp
     * @param endTime End timestamp
     * @return Vector of log entries
     */
    virtual std::vector<LogEntry> getRange(qint64 startTime, qint64 endTime) const = 0;

    /**
     * @brief Search for log entries containing a specific text
     * @param searchText Text to search for (case-insensitive)
     * @return Vector of matching log entries
     */
    virtual std::vector<LogEntry> search(const QString& searchText) const = 0;

    /**
     * @brief Get the current number of entries in the buffer
     * @return Entry count
     */
    virtual size_t size() const = 0;

    /**
     * @brief Get the maximum capacity of the buffer
     * @return Maximum number of entries
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief Check if the buffer is empty
     * @return true if buffer has no entries
     */
    virtual bool isEmpty() const = 0;

    /**
     * @brief Check if the buffer is full
     * @return true if buffer has reached capacity
     */
    virtual bool isFull() const = 0;

    /**
     * @brief Clear all log entries from the buffer
     */
    virtual void clear() = 0;

    /**
     * @brief Set the maximum capacity of the buffer
     * @param newCapacity New maximum number of entries
     */
    virtual void setCapacity(size_t newCapacity) = 0;

    /**
     * @brief Set a callback to be invoked when logs are added
     * @param callback Function to call when logs are added (for throttled UI updates)
     */
    virtual void setUpdateCallback(std::function<void()> callback) = 0;
};

} // namespace ZenRunner::Core
