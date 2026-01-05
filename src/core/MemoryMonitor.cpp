#include "core/MemoryMonitor.hpp"
#include <QDebug>

// Platform-specific includes
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#include <string>
#elif defined(Q_OS_MAC)
#include <mach/mach.h>
#include <sys/resource.h>
#endif

namespace ZenRunner::Memory {

QString MemoryStats::toMB() const {
    const double mb = static_cast<double>(currentRSS) / (1024.0 * 1024.0);
    return QString::number(mb, 'f', 2) + " MB";
}

QString MemoryStats::toString() const {
    const double currentMB = static_cast<double>(currentRSS) / (1024.0 * 1024.0);
    const double peakMB = static_cast<double>(peakRSS) / (1024.0 * 1024.0);
    const double virtualMB = static_cast<double>(virtualMemory) / (1024.0 * 1024.0);
    
    return QString("Current: %1 MB, Peak: %2 MB, Virtual: %3 MB")
        .arg(currentMB, 0, 'f', 2)
        .arg(peakMB, 0, 'f', 2)
        .arg(virtualMB, 0, 'f', 2);
}

MemoryStats MemoryMonitor::getCurrentUsage() {
    MemoryStats stats;
    
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), 
                            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                            sizeof(pmc))) {
        stats.currentRSS = pmc.WorkingSetSize;
        stats.peakRSS = pmc.PeakWorkingSetSize;
        stats.virtualMemory = pmc.PrivateUsage;
    }
    
#elif defined(Q_OS_LINUX)
    // Read from /proc/self/status
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    
    while (std::getline(statusFile, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            // Extract KB value
            size_t pos = line.find_last_of('\t');
            if (pos != std::string::npos) {
                const size_t kb = std::stoull(line.substr(pos + 1));
                stats.currentRSS = kb * 1024;
            }
        } else if (line.substr(0, 10) == "VmHWM:") {
            // Peak RSS
            size_t pos = line.find_last_of('\t');
            if (pos != std::string::npos) {
                const size_t kb = std::stoull(line.substr(pos + 1));
                stats.peakRSS = kb * 1024;
            }
        } else if (line.substr(0, 7) == "VmSize:") {
            // Virtual memory
            size_t pos = line.find_last_of('\t');
            if (pos != std::string::npos) {
                const size_t kb = std::stoull(line.substr(pos + 1));
                stats.virtualMemory = kb * 1024;
            }
        }
    }
    
#elif defined(Q_OS_MAC)
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS) {
        stats.currentRSS = info.resident_size;
        stats.virtualMemory = info.virtual_size;
    }
    
    // Get peak RSS from rusage
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        stats.peakRSS = usage.ru_maxrss;
    }
#endif
    
    return stats;
}

bool MemoryMonitor::isWithinTarget() {
    constexpr size_t TARGET_BYTES = 30 * 1024 * 1024; // 30 MB
    const MemoryStats stats = getCurrentUsage();
    return stats.currentRSS < TARGET_BYTES;
}

double MemoryMonitor::getTargetPercentage() {
    constexpr double TARGET_MB = 30.0;
    const MemoryStats stats = getCurrentUsage();
    const double currentMB = static_cast<double>(stats.currentRSS) / (1024.0 * 1024.0);
    return (currentMB / TARGET_MB) * 100.0;
}

void MemoryMonitor::logUsage() {
    const MemoryStats stats = getCurrentUsage();
    const double percentage = getTargetPercentage();
    const bool withinTarget = isWithinTarget();
    
    qDebug() << "=== Memory Usage ===";
    qDebug() << stats.toString();
    qDebug() << QString("Target: %1% of 30 MB limit").arg(percentage, 0, 'f', 1);
    qDebug() << QString("Status: %1").arg(withinTarget ? "✓ Within target" : "✗ Exceeds target");
    qDebug() << "==================";
}

} // namespace ZenRunner::Memory
