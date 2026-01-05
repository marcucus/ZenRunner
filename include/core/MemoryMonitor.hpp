#pragma once

#include <cstddef>
#include <QString>

namespace ZenRunner::Memory {

/**
 * @brief Memory usage statistics
 */
struct MemoryStats {
    size_t currentRSS{0};       ///< Current Resident Set Size in bytes
    size_t peakRSS{0};          ///< Peak RSS in bytes
    size_t virtualMemory{0};    ///< Virtual memory size in bytes
    
    // Convert to human-readable format
    QString toMB() const;
    QString toString() const;
};

/**
 * @brief Memory monitoring utility
 * 
 * Provides lightweight memory usage tracking to ensure RAM stays under 30 MB.
 * Uses platform-specific APIs for accurate measurements.
 */
class MemoryMonitor {
public:
    /**
     * @brief Get current memory usage statistics
     * @return Memory statistics
     */
    static MemoryStats getCurrentUsage();
    
    /**
     * @brief Check if memory usage is within target (< 30 MB)
     * @return true if within target
     */
    static bool isWithinTarget();
    
    /**
     * @brief Get memory usage as percentage of 30 MB target
     * @return Percentage (0-100+)
     */
    static double getTargetPercentage();
    
    /**
     * @brief Log current memory usage to console
     */
    static void logUsage();
};

} // namespace ZenRunner::Memory
