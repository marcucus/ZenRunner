#pragma once

#include <QString>
#include <QElapsedTimer>
#include <QDebug>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace ZenRunner::Core {

/**
 * @brief Performance profiler for measuring signal-to-slot latency
 * 
 * This class provides instrumentation to measure and track the latency
 * of Qt signal/slot connections to ensure they meet the <10ms target.
 */
class SignalLatencyProfiler {
public:
    struct LatencyStats {
        qint64 minLatencyNs = std::numeric_limits<qint64>::max();
        qint64 maxLatencyNs = 0;
        qint64 totalLatencyNs = 0;
        qint64 count = 0;
        
        [[nodiscard]] double avgLatencyMs() const noexcept {
            return count > 0 ? (totalLatencyNs / static_cast<double>(count)) / 1000000.0 : 0.0;
        }
        
        [[nodiscard]] double minLatencyMs() const noexcept {
            return minLatencyNs / 1000000.0;
        }
        
        [[nodiscard]] double maxLatencyMs() const noexcept {
            return maxLatencyNs / 1000000.0;
        }
    };
    
    /**
     * @brief Get singleton instance
     */
    static SignalLatencyProfiler& instance() {
        static SignalLatencyProfiler profiler;
        return profiler;
    }
    
    /**
     * @brief Mark the start of a signal emission
     * @param signalName Name of the signal being emitted
     * @return Timestamp token for this emission
     */
    [[nodiscard]] qint64 markSignalEmission(const QString& signalName) {
        if (!enabled_.load(std::memory_order_relaxed)) [[unlikely]] {
            return 0;
        }
        
        const auto now = std::chrono::steady_clock::now();
        const qint64 timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count();
        
        std::lock_guard lock(mutex_);
        pendingSignals_[timestamp] = signalName;
        
        return timestamp;
    }
    
    /**
     * @brief Mark the completion of a slot execution
     * @param timestamp The timestamp from markSignalEmission
     * @param slotName Name of the slot that was executed
     */
    void markSlotExecution(qint64 timestamp, const QString& slotName) {
        if (!enabled_.load(std::memory_order_relaxed) || timestamp == 0) [[unlikely]] {
            return;
        }
        
        const auto now = std::chrono::steady_clock::now();
        const qint64 currentTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count();
        
        const qint64 latencyNs = currentTime - timestamp;
        
        std::lock_guard lock(mutex_);
        
        auto it = pendingSignals_.find(timestamp);
        if (it != pendingSignals_.end()) [[likely]] {
            const QString connectionKey = it->second + " -> " + slotName;
            pendingSignals_.erase(it);
            
            auto& stats = connectionStats_[connectionKey];
            stats.count++;
            stats.totalLatencyNs += latencyNs;
            stats.minLatencyNs = std::min(stats.minLatencyNs, latencyNs);
            stats.maxLatencyNs = std::max(stats.maxLatencyNs, latencyNs);
            
            // Warn if latency exceeds 10ms threshold
            if (latencyNs > 10'000'000) [[unlikely]] {
                qWarning() << "[LATENCY WARNING]" << connectionKey 
                          << "took" << (latencyNs / 1000000.0) << "ms (> 10ms threshold)";
            }
        }
    }
    
    /**
     * @brief Get statistics for a specific signal-slot connection
     */
    [[nodiscard]] LatencyStats getStats(const QString& connectionKey) const {
        std::lock_guard lock(mutex_);
        auto it = connectionStats_.find(connectionKey);
        return it != connectionStats_.end() ? it->second : LatencyStats{};
    }
    
    /**
     * @brief Get all collected statistics
     */
    [[nodiscard]] std::unordered_map<QString, LatencyStats> getAllStats() const {
        std::lock_guard lock(mutex_);
        return connectionStats_;
    }
    
    /**
     * @brief Print a summary report of all measurements
     */
    void printReport() const {
        std::lock_guard lock(mutex_);
        
        qDebug() << "=== Signal Latency Profile Report ===";
        qDebug() << "Total connections tracked:" << connectionStats_.size();
        
        for (const auto& [key, stats] : connectionStats_) {
            qDebug() << "\n" << key;
            qDebug() << "  Count:" << stats.count;
            qDebug() << "  Avg:" << stats.avgLatencyMs() << "ms";
            qDebug() << "  Min:" << stats.minLatencyMs() << "ms";
            qDebug() << "  Max:" << stats.maxLatencyMs() << "ms";
            
            if (stats.maxLatencyMs() > 10.0) {
                qDebug() << "  ⚠️  EXCEEDS 10ms TARGET!";
            }
        }
        
        qDebug() << "\n=====================================";
    }
    
    /**
     * @brief Reset all statistics
     */
    void reset() {
        std::lock_guard lock(mutex_);
        connectionStats_.clear();
        pendingSignals_.clear();
    }
    
    /**
     * @brief Enable or disable profiling
     */
    void setEnabled(bool enabled) noexcept {
        enabled_.store(enabled, std::memory_order_relaxed);
    }
    
    /**
     * @brief Check if profiling is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return enabled_.load(std::memory_order_relaxed);
    }
    
private:
    SignalLatencyProfiler() = default;
    ~SignalLatencyProfiler() = default;
    
    // Non-copyable
    SignalLatencyProfiler(const SignalLatencyProfiler&) = delete;
    SignalLatencyProfiler& operator=(const SignalLatencyProfiler&) = delete;
    
    mutable std::mutex mutex_;
    std::unordered_map<QString, LatencyStats> connectionStats_;
    std::unordered_map<qint64, QString> pendingSignals_;
    std::atomic<bool> enabled_{false};
};

/**
 * @brief RAII helper for automatic slot latency measurement
 */
class ScopedSlotProfiler {
public:
    explicit ScopedSlotProfiler(qint64 timestamp, const QString& slotName)
        : timestamp_(timestamp)
        , slotName_(slotName)
    {
    }
    
    ~ScopedSlotProfiler() {
        SignalLatencyProfiler::instance().markSlotExecution(timestamp_, slotName_);
    }
    
    // Non-copyable, non-movable
    ScopedSlotProfiler(const ScopedSlotProfiler&) = delete;
    ScopedSlotProfiler& operator=(const ScopedSlotProfiler&) = delete;
    ScopedSlotProfiler(ScopedSlotProfiler&&) = delete;
    ScopedSlotProfiler& operator=(ScopedSlotProfiler&&) = delete;
    
private:
    qint64 timestamp_;
    QString slotName_;
};

} // namespace ZenRunner::Core

// Convenience macros for profiling
#ifdef ZENRUNNER_ENABLE_PROFILING
    #define PROFILE_SIGNAL_EMISSION(signalName) \
        ZenRunner::Core::SignalLatencyProfiler::instance().markSignalEmission(signalName)
    
    #define PROFILE_SLOT_EXECUTION(timestamp, slotName) \
        ZenRunner::Core::ScopedSlotProfiler _slot_profiler(timestamp, slotName)
#else
    #define PROFILE_SIGNAL_EMISSION(signalName) (0)
    #define PROFILE_SLOT_EXECUTION(timestamp, slotName) ((void)0)
#endif
