#pragma once

#include <QString>
#include <QObject>
#include <memory>
#include <vector>
#include <chrono>

namespace ZenRunner::Core {

/**
 * @brief Resource usage statistics for a process
 */
struct ProcessResourceStats {
    qint64 pid{0};                      ///< Process ID
    QString processId;                  ///< Internal process identifier
    double cpuPercent{0.0};             ///< CPU usage percentage (0-100+)
    size_t memoryBytes{0};              ///< Memory usage in bytes (RSS)
    std::chrono::system_clock::time_point timestamp;  ///< When this measurement was taken
    
    /**
     * @brief Convert memory to human-readable format
     */
    QString memoryToString() const;
    
    /**
     * @brief Get memory in MB
     */
    double memoryMB() const {
        return static_cast<double>(memoryBytes) / (1024.0 * 1024.0);
    }
};

/**
 * @brief Historical data point for resource usage trends
 */
struct ResourceDataPoint {
    std::chrono::system_clock::time_point timestamp;
    double cpuPercent{0.0};
    double memoryMB{0.0};
};

/**
 * @brief Statistics summary including trends and spikes
 */
struct ResourceSummary {
    QString processId;
    double avgCpu{0.0};              ///< Average CPU usage
    double maxCpu{0.0};              ///< Peak CPU usage
    double avgMemoryMB{0.0};         ///< Average memory usage in MB
    double maxMemoryMB{0.0};         ///< Peak memory usage in MB
    bool hasRecentSpike{false};      ///< True if there was a spike in last minute
    int dataPoints{0};               ///< Number of data points collected
    
    std::vector<ResourceDataPoint> recentHistory;  ///< Recent history (limited size)
};

/**
 * @brief Interface for monitoring process resource usage
 * 
 * This interface defines the contract for tracking CPU and RAM usage
 * of managed processes. Implementations should use lightweight sampling
 * to avoid impacting performance targets.
 */
class IProcessResourceMonitor : public QObject {
    Q_OBJECT
    
public:
    explicit IProcessResourceMonitor(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IProcessResourceMonitor() = default;
    
    /**
     * @brief Start monitoring a process
     * @param processId Internal process identifier
     * @param pid Operating system process ID
     * @return true if monitoring started successfully
     */
    virtual bool startMonitoring(const QString& processId, qint64 pid) = 0;
    
    /**
     * @brief Stop monitoring a process
     * @param processId Internal process identifier
     */
    virtual void stopMonitoring(const QString& processId) = 0;
    
    /**
     * @brief Get current resource usage for a process
     * @param processId Internal process identifier
     * @return Current resource statistics, or nullptr if not monitored
     */
    virtual std::shared_ptr<ProcessResourceStats> getCurrentStats(const QString& processId) const = 0;
    
    /**
     * @brief Get resource summary with trends for a process
     * @param processId Internal process identifier
     * @return Summary statistics, or nullptr if not monitored
     */
    virtual std::shared_ptr<ResourceSummary> getSummary(const QString& processId) const = 0;
    
    /**
     * @brief Get all monitored process IDs
     * @return List of process IDs currently being monitored
     */
    virtual QStringList getMonitoredProcesses() const = 0;
    
    /**
     * @brief Set the sampling interval
     * @param intervalMs Sampling interval in milliseconds (default: 2000ms)
     */
    virtual void setSamplingInterval(int intervalMs) = 0;

signals:
    /**
     * @brief Emitted when a resource spike is detected
     * @param processId Process identifier
     * @param cpuPercent Current CPU percentage
     * @param memoryMB Current memory in MB
     */
    void resourceSpikeDetected(const QString& processId, double cpuPercent, double memoryMB);
    
    /**
     * @brief Emitted periodically with updated stats
     * @param processId Process identifier
     * @param stats Current statistics
     */
    void statsUpdated(const QString& processId, ProcessResourceStats stats);
};

} // namespace ZenRunner::Core
