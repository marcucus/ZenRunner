#pragma once

#include "core/IProcessResourceMonitor.hpp"
#include <QTimer>
#include <QMap>
#include <memory>
#include <deque>

namespace ZenRunner::Core {

/**
 * @brief Concrete implementation of process resource monitoring
 * 
 * Uses platform-specific APIs to track CPU and memory usage with minimal overhead.
 * Implements circular buffering for historical data to maintain memory efficiency.
 */
class ProcessResourceMonitor : public IProcessResourceMonitor {
    Q_OBJECT
    
public:
    explicit ProcessResourceMonitor(QObject* parent = nullptr);
    ~ProcessResourceMonitor() override = default;
    
    // IProcessResourceMonitor interface
    bool startMonitoring(const QString& processId, qint64 pid) override;
    void stopMonitoring(const QString& processId) override;
    std::shared_ptr<ProcessResourceStats> getCurrentStats(const QString& processId) const override;
    std::shared_ptr<ResourceSummary> getSummary(const QString& processId) const override;
    QStringList getMonitoredProcesses() const override;
    void setSamplingInterval(int intervalMs) override;
    
private slots:
    /**
     * @brief Sample resource usage for all monitored processes
     */
    void sampleResources();
    
private:
    /**
     * @brief Monitored process data
     */
    struct MonitoredProcess {
        QString processId;
        qint64 pid;
        std::deque<ResourceDataPoint> history;  // Limited size circular buffer
        ProcessResourceStats lastStats;
        
        // For CPU calculation (platform-specific)
        unsigned long long lastCpuTime{0};
        std::chrono::system_clock::time_point lastSampleTime;
    };
    
    /**
     * @brief Sample resource usage for a specific process
     * @param process Monitored process data
     * @return true if sampling was successful
     */
    bool sampleProcess(MonitoredProcess& process);
    
    /**
     * @brief Detect if there's a resource spike
     * @param process Monitored process data
     * @return true if spike detected
     */
    bool detectSpike(const MonitoredProcess& process) const;
    
    /**
     * @brief Calculate resource summary from history
     * @param process Monitored process data
     * @return Resource summary
     */
    ResourceSummary calculateSummary(const MonitoredProcess& process) const;
    
    /**
     * @brief Get CPU time for a process (platform-specific)
     * @param pid Process ID
     * @return CPU time in microseconds, or 0 if failed
     */
    unsigned long long getCpuTime(qint64 pid) const;
    
    /**
     * @brief Get memory usage for a process (platform-specific)
     * @param pid Process ID
     * @return Memory in bytes (RSS), or 0 if failed
     */
    size_t getMemoryUsage(qint64 pid) const;
    
    // Configuration
    static constexpr int DEFAULT_SAMPLING_INTERVAL_MS = 2000;
    static constexpr size_t MAX_HISTORY_SIZE = 30;  // Keep last 30 samples (~1 minute at 2s interval)
    static constexpr double CPU_SPIKE_THRESHOLD = 80.0;  // 80% CPU
    static constexpr double MEMORY_SPIKE_THRESHOLD_MB = 100.0;  // 100 MB
    
    QTimer* samplingTimer_;
    QMap<QString, MonitoredProcess> monitoredProcesses_;
};

} // namespace ZenRunner::Core
