#include "core/ProcessResourceMonitor.h"
#include <QDebug>
#include <algorithm>

// Platform-specific includes
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#elif defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include <mach/mach.h>
#endif

namespace ZenRunner::Core {

// ============================================================================
// ProcessResourceStats Implementation
// ============================================================================

QString ProcessResourceStats::memoryToString() const {
    const double mb = memoryMB();
    if (mb < 1.0) {
        return QString::number(memoryBytes / 1024.0, 'f', 1) + " KB";
    }
    return QString::number(mb, 'f', 1) + " MB";
}

// ============================================================================
// ProcessResourceMonitor Implementation
// ============================================================================

ProcessResourceMonitor::ProcessResourceMonitor(QObject* parent)
    : IProcessResourceMonitor(parent)
    , samplingTimer_(new QTimer(this))
{
    samplingTimer_->setInterval(DEFAULT_SAMPLING_INTERVAL_MS);
    connect(samplingTimer_, &QTimer::timeout, this, &ProcessResourceMonitor::sampleResources);
}

bool ProcessResourceMonitor::startMonitoring(const QString& processId, qint64 pid) {
    if (monitoredProcesses_.contains(processId)) {
        qDebug() << "Already monitoring process:" << processId;
        return true;
    }
    
    if (pid <= 0) {
        qDebug() << "Invalid PID for process:" << processId;
        return false;
    }
    
    MonitoredProcess process;
    process.processId = processId;
    process.pid = pid;
    process.lastSampleTime = std::chrono::system_clock::now();
    process.lastCpuTime = getCpuTime(pid);
    
    // Initial sample
    if (!sampleProcess(process)) {
        qDebug() << "Failed to sample process initially:" << processId;
        return false;
    }
    
    monitoredProcesses_[processId] = std::move(process);
    
    // Start timer if this is the first process
    if (monitoredProcesses_.size() == 1) {
        samplingTimer_->start();
    }
    
    qDebug() << "Started monitoring process:" << processId << "PID:" << pid;
    return true;
}

void ProcessResourceMonitor::stopMonitoring(const QString& processId) {
    if (monitoredProcesses_.remove(processId) > 0) {
        qDebug() << "Stopped monitoring process:" << processId;
        
        // Stop timer if no processes are being monitored
        if (monitoredProcesses_.isEmpty()) {
            samplingTimer_->stop();
        }
    }
}

std::shared_ptr<ProcessResourceStats> ProcessResourceMonitor::getCurrentStats(const QString& processId) const {
    auto it = monitoredProcesses_.find(processId);
    if (it == monitoredProcesses_.end()) {
        return nullptr;
    }
    
    return std::make_shared<ProcessResourceStats>(it->lastStats);
}

std::shared_ptr<ResourceSummary> ProcessResourceMonitor::getSummary(const QString& processId) const {
    auto it = monitoredProcesses_.find(processId);
    if (it == monitoredProcesses_.end()) {
        return nullptr;
    }
    
    return std::make_shared<ResourceSummary>(calculateSummary(*it));
}

QStringList ProcessResourceMonitor::getMonitoredProcesses() const {
    return monitoredProcesses_.keys();
}

void ProcessResourceMonitor::setSamplingInterval(int intervalMs) {
    if (intervalMs < 100 || intervalMs > 60000) {
        qWarning() << "Invalid sampling interval:" << intervalMs << "ms. Using default.";
        return;
    }
    
    samplingTimer_->setInterval(intervalMs);
}

void ProcessResourceMonitor::sampleResources() {
    for (auto it = monitoredProcesses_.begin(); it != monitoredProcesses_.end(); ++it) {
        if (sampleProcess(*it)) {
            emit statsUpdated(it->processId, it->lastStats);
            
            // Check for spikes
            if (detectSpike(*it)) {
                emit resourceSpikeDetected(it->processId, it->lastStats.cpuPercent, it->lastStats.memoryMB());
            }
        }
    }
}

bool ProcessResourceMonitor::sampleProcess(MonitoredProcess& process) {
    const auto now = std::chrono::system_clock::now();
    
    // Get current CPU time and memory
    const unsigned long long currentCpuTime = getCpuTime(process.pid);
    const size_t currentMemory = getMemoryUsage(process.pid);
    
    // Check if process still exists
    if (currentCpuTime == 0 && currentMemory == 0) {
        return false;
    }
    
    // Calculate CPU percentage
    double cpuPercent = 0.0;
    if (process.lastCpuTime > 0) {
        const auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(now - process.lastSampleTime).count();
        const unsigned long long cpuDiff = currentCpuTime - process.lastCpuTime;
        
        if (timeDiff > 0) {
            // CPU percentage = (cpu_time_diff / real_time_diff) * 100
            cpuPercent = (static_cast<double>(cpuDiff) / static_cast<double>(timeDiff)) * 100.0;
            
            // Clamp to reasonable values (can exceed 100% on multi-core systems)
            cpuPercent = std::min(cpuPercent, 1000.0);  // Max 1000% (10 cores at 100%)
        }
    }
    
    // Update last stats
    process.lastStats.pid = process.pid;
    process.lastStats.processId = process.processId;
    process.lastStats.cpuPercent = cpuPercent;
    process.lastStats.memoryBytes = currentMemory;
    process.lastStats.timestamp = now;
    
    process.lastCpuTime = currentCpuTime;
    process.lastSampleTime = now;
    
    // Add to history
    ResourceDataPoint dataPoint;
    dataPoint.timestamp = now;
    dataPoint.cpuPercent = cpuPercent;
    dataPoint.memoryMB = process.lastStats.memoryMB();
    
    process.history.push_back(dataPoint);
    
    // Maintain max history size
    if (process.history.size() > MAX_HISTORY_SIZE) {
        process.history.pop_front();
    }
    
    return true;
}

bool ProcessResourceMonitor::detectSpike(const MonitoredProcess& process) const {
    if (process.history.size() < 3) {
        return false;  // Need at least 3 samples
    }
    
    const auto& current = process.lastStats;
    
    // CPU spike: current usage > threshold
    if (current.cpuPercent > CPU_SPIKE_THRESHOLD) {
        return true;
    }
    
    // Memory spike: current usage > threshold
    if (current.memoryMB() > MEMORY_SPIKE_THRESHOLD_MB) {
        return true;
    }
    
    return false;
}

ResourceSummary ProcessResourceMonitor::calculateSummary(const MonitoredProcess& process) const {
    ResourceSummary summary;
    summary.processId = process.processId;
    summary.dataPoints = static_cast<int>(process.history.size());
    
    if (process.history.empty()) {
        return summary;
    }
    
    // Calculate averages and peaks
    double totalCpu = 0.0;
    double totalMemory = 0.0;
    
    for (const auto& point : process.history) {
        totalCpu += point.cpuPercent;
        totalMemory += point.memoryMB;
        summary.maxCpu = std::max(summary.maxCpu, point.cpuPercent);
        summary.maxMemoryMB = std::max(summary.maxMemoryMB, point.memoryMB);
    }
    
    summary.avgCpu = totalCpu / process.history.size();
    summary.avgMemoryMB = totalMemory / process.history.size();
    
    // Check for recent spikes (last 5 samples or 10 seconds)
    const auto now = std::chrono::system_clock::now();
    const auto oneMinuteAgo = now - std::chrono::minutes(1);
    
    for (const auto& point : process.history) {
        if (point.timestamp > oneMinuteAgo) {
            if (point.cpuPercent > CPU_SPIKE_THRESHOLD || point.memoryMB > MEMORY_SPIKE_THRESHOLD_MB) {
                summary.hasRecentSpike = true;
                break;
            }
        }
    }
    
    // Copy recent history (last 30 points)
    summary.recentHistory.assign(process.history.begin(), process.history.end());
    
    return summary;
}

// ============================================================================
// Platform-specific implementations
// ============================================================================

#ifdef Q_OS_WIN

unsigned long long ProcessResourceMonitor::getCpuTime(qint64 pid) const {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, static_cast<DWORD>(pid));
    if (!hProcess) {
        return 0;
    }
    
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hProcess);
        return 0;
    }
    
    CloseHandle(hProcess);
    
    // Convert FILETIME to microseconds
    ULARGE_INTEGER kernel, user;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;
    
    // FILETIME is in 100-nanosecond intervals, convert to microseconds
    return (kernel.QuadPart + user.QuadPart) / 10;
}

size_t ProcessResourceMonitor::getMemoryUsage(qint64 pid) const {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, static_cast<DWORD>(pid));
    if (!hProcess) {
        return 0;
    }
    
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        CloseHandle(hProcess);
        return 0;
    }
    
    CloseHandle(hProcess);
    return pmc.WorkingSetSize;
}

#elif defined(Q_OS_LINUX)

unsigned long long ProcessResourceMonitor::getCpuTime(qint64 pid) const {
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    if (!statFile.is_open()) {
        return 0;
    }
    
    std::string line;
    std::getline(statFile, line);
    
    // Parse /proc/[pid]/stat
    // Format: pid (comm) state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt utime stime ...
    std::istringstream iss(line);
    std::string token;
    
    // Skip first 13 fields to get to utime (14th) and stime (15th)
    for (int i = 0; i < 13; ++i) {
        iss >> token;
    }
    
    unsigned long long utime = 0, stime = 0;
    iss >> utime >> stime;
    
    // Convert from clock ticks to microseconds
    const long clockTick = sysconf(_SC_CLK_TCK);
    if (clockTick <= 0) {
        return 0;
    }
    
    return ((utime + stime) * 1000000) / clockTick;
}

size_t ProcessResourceMonitor::getMemoryUsage(qint64 pid) const {
    std::ifstream statusFile("/proc/" + std::to_string(pid) + "/status");
    if (!statusFile.is_open()) {
        return 0;
    }
    
    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line.substr(6));
            size_t kb = 0;
            iss >> kb;
            return kb * 1024;  // Convert to bytes
        }
    }
    
    return 0;
}

#elif defined(Q_OS_MAC)

unsigned long long ProcessResourceMonitor::getCpuTime(qint64 pid) const {
    struct proc_taskinfo taskInfo;
    const int result = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskInfo, sizeof(taskInfo));
    
    if (result != sizeof(taskInfo)) {
        return 0;
    }
    
    // Convert to microseconds
    const unsigned long long userTime = taskInfo.pti_total_user / 1000;  // nanoseconds to microseconds
    const unsigned long long systemTime = taskInfo.pti_total_system / 1000;
    
    return userTime + systemTime;
}

size_t ProcessResourceMonitor::getMemoryUsage(qint64 pid) const {
    struct proc_taskinfo taskInfo;
    const int result = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskInfo, sizeof(taskInfo));
    
    if (result != sizeof(taskInfo)) {
        return 0;
    }
    
    return taskInfo.pti_resident_size;
}

#else

// Fallback for unsupported platforms
unsigned long long ProcessResourceMonitor::getCpuTime(qint64 pid) const {
    Q_UNUSED(pid);
    return 0;
}

size_t ProcessResourceMonitor::getMemoryUsage(qint64 pid) const {
    Q_UNUSED(pid);
    return 0;
}

#endif

} // namespace ZenRunner::Core
