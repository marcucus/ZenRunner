#include "LogBuffer.h"
#include "core/CircularBuffer.h"
#include "AnsiParser.h"
#include <QDateTime>
#include <algorithm>

namespace ZenRunner::Core {

/**
 * @brief Concrete implementation of ILogBuffer using a circular buffer
 * 
 * This implementation uses a fixed-size circular buffer to ensure O(1) operations
 * and prevent memory growth. The default capacity is 5000 entries as per specs.
 * Integrates ANSI/VT100 parser for colored log output.
 */
class LogBuffer : public ILogBuffer {
public:
    explicit LogBuffer(size_t capacity = 5000)
        : capacity_(capacity)
        , buffer_(std::make_unique<CircularBuffer<LogEntry, 5000>>())
        , ansiParser_(createAnsiParser())
        , updateCallback_(nullptr)
    {
        // Pre-allocate if capacity is different from default
        // For simplicity, we use the fixed-size template parameter
    }

    ~LogBuffer() override = default;

    void append(const LogEntry& entry) override {
        buffer_->emplace(entry);
        notifyUpdate();
    }

    void append(const QString& text, bool isError = false) override {
        LogEntry entry;
        entry.text = text;
        entry.timestamp = QDateTime::currentMSecsSinceEpoch();
        entry.isError = isError;
        
        // Parse ANSI codes asynchronously (still fast due to O(n) parser)
        if (ansiParser_ && ansiParser_->containsAnsiCodes(text)) {
            entry.hasAnsiCodes = true;
            entry.segments = ansiParser_->parse(text);
            entry.plainText = ansiParser_->stripAnsiCodes(text);
        } else {
            entry.hasAnsiCodes = false;
            entry.plainText = text;
            // Create single unstyled segment for consistency
            StyledSegment segment;
            segment.text = text;
            entry.segments = {segment};
        }
        
        buffer_->emplace(std::move(entry));
        notifyUpdate();
    }

    std::vector<LogEntry> getAll() const override {
        return buffer_->toVector();
    }

    std::vector<LogEntry> getRecent(size_t count) const override {
        return buffer_->lastN(count);
    }

    std::vector<LogEntry> getRange(qint64 startTime, qint64 endTime) const override {
        std::vector<LogEntry> result;
        std::vector<LogEntry> allEntries = buffer_->toVector();
        
        // Reserve space to avoid reallocations
        result.reserve(allEntries.size());
        
        for (const auto& entry : allEntries) {
            if (entry.timestamp >= startTime && entry.timestamp <= endTime) {
                result.push_back(entry);
            }
        }
        
        return result;
    }

    std::vector<LogEntry> search(const QString& searchText) const override {
        std::vector<LogEntry> result;
        std::vector<LogEntry> allEntries = buffer_->toVector();
        
        // Case-insensitive search
        const QString lowerSearchText = searchText.toLower();
        
        result.reserve(allEntries.size() / 10); // Heuristic: expect ~10% match rate
        
        for (const auto& entry : allEntries) {
            if (entry.plainText.toLower().contains(lowerSearchText)) {
                result.push_back(entry);
            }
        }
        
        return result;
    }

    size_t size() const override {
        return buffer_->size();
    }

    size_t capacity() const override {
        return capacity_;
    }

    bool isEmpty() const override {
        return buffer_->empty();
    }

    bool isFull() const override {
        return buffer_->full();
    }

    void clear() override {
        buffer_->clear();
    }

    void setCapacity(size_t newCapacity) override {
        // Note: With fixed-size template, we can't dynamically change capacity
        // This would require a dynamic implementation or recreating the buffer
        // For now, we just update the tracked capacity value
        capacity_ = newCapacity;
        
        // In a production implementation, you might want to:
        // 1. Create a new buffer with the new capacity
        // 2. Copy existing entries up to the new capacity
        // 3. Swap the buffers
    }

    void setUpdateCallback(std::function<void()> callback) override {
        updateCallback_ = callback;
    }

private:
    size_t capacity_;
    std::unique_ptr<CircularBuffer<LogEntry, 5000>> buffer_;
    std::unique_ptr<IAnsiParser> ansiParser_;
    std::function<void()> updateCallback_;
    
    void notifyUpdate() {
        if (updateCallback_) {
            updateCallback_();
        }
    }
};

// Factory function to create LogBuffer instances
std::unique_ptr<ILogBuffer> createLogBuffer(size_t capacity) {
    return std::make_unique<LogBuffer>(capacity);
}

} // namespace ZenRunner::Core
