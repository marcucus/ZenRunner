#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QString>
#include <QDebug>
#include <chrono>

namespace ZenRunner::Core {

/**
 * @brief RAII-based performance measurement utility
 * 
 * Measures elapsed time from construction to destruction and logs
 * warnings if the duration exceeds a threshold.
 */
class PerformanceTimer {
public:
    explicit PerformanceTimer(QString operation, qint64 warningThresholdMs = 10)
        : operation_(std::move(operation))
        , warningThresholdMs_(warningThresholdMs)
        , start_(std::chrono::steady_clock::now())
    {
    }
    
    ~PerformanceTimer() {
        const auto end = std::chrono::steady_clock::now();
        const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start_
        ).count();
        
        if (durationMs > warningThresholdMs_) [[unlikely]] {
            qWarning() << "[PERFORMANCE]" << operation_ 
                      << "took" << durationMs << "ms (threshold:" 
                      << warningThresholdMs_ << "ms)";
        } else [[likely]] {
            qDebug() << "[PERFORMANCE]" << operation_ << "took" << durationMs << "ms";
        }
    }
    
    // Non-copyable, non-movable
    PerformanceTimer(const PerformanceTimer&) = delete;
    PerformanceTimer& operator=(const PerformanceTimer&) = delete;
    PerformanceTimer(PerformanceTimer&&) = delete;
    PerformanceTimer& operator=(PerformanceTimer&&) = delete;
    
private:
    QString operation_;
    qint64 warningThresholdMs_;
    std::chrono::steady_clock::time_point start_;
};

/**
 * @brief Helper to measure and return elapsed time
 */
class ScopedTimer {
public:
    ScopedTimer() : start_(std::chrono::steady_clock::now()) {}
    
    [[nodiscard]] qint64 elapsedMs() const {
        const auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start_
        ).count();
    }
    
    [[nodiscard]] qint64 elapsedUs() const {
        const auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            end - start_
        ).count();
    }
    
    [[nodiscard]] qint64 elapsedNs() const {
        const auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start_
        ).count();
    }
    
    void reset() {
        start_ = std::chrono::steady_clock::now();
    }
    
private:
    std::chrono::steady_clock::time_point start_;
};

} // namespace ZenRunner::Core

// Convenience macro for performance measurement
#ifdef ZENRUNNER_ENABLE_PROFILING
    #define PERF_TIMER(operation) \
        ZenRunner::Core::PerformanceTimer _perf_timer(operation)
    
    #define PERF_TIMER_THRESHOLD(operation, thresholdMs) \
        ZenRunner::Core::PerformanceTimer _perf_timer(operation, thresholdMs)
#else
    #define PERF_TIMER(operation) ((void)0)
    #define PERF_TIMER_THRESHOLD(operation, thresholdMs) ((void)0)
#endif
