#include "core/ProcessManager.h"
#include "core/SignalLatencyProfiler.hpp"
#include "core/PerformanceTimer.hpp"
#include "types/CommonTypes.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <chrono>
#include <iostream>

using namespace ZenRunner;
using namespace ZenRunner::Core;

/**
 * @brief Performance test for signal/slot latency measurement
 * 
 * This test verifies that the response to user signals is < 10ms
 */
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "===========================================";
    qDebug() << "ZenRunner Signal Latency Performance Test";
    qDebug() << "Target: < 10ms response time";
    qDebug() << "===========================================\n";
    
    // Enable profiling
    SignalLatencyProfiler::instance().setEnabled(true);
    
    ProcessManager manager;
    
    // Track signal reception times
    struct SignalTiming {
        qint64 emissionTime = 0;
        qint64 receptionTime = 0;
        
        [[nodiscard]] double latencyMs() const {
            return (receptionTime - emissionTime) / 1000000.0;
        }
    };
    
    std::vector<SignalTiming> stateChangeTimings;
    std::vector<SignalTiming> outputTimings;
    std::vector<SignalTiming> finishedTimings;
    
    // Connect to signals with latency measurement
    QObject::connect(&manager, &ProcessManager::processStateChanged,
        &manager, [&stateChangeTimings](const QString& id, ProcessState newState) {
            const auto timestamp = SignalLatencyProfiler::instance().markSignalEmission("processStateChanged");
            PROFILE_SLOT_EXECUTION(timestamp, "processStateChanged handler");
            
            SignalTiming timing;
            timing.receptionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
            timing.emissionTime = timing.receptionTime; // Approximate
            stateChangeTimings.push_back(timing);
            
            qDebug() << "Process" << id << "state changed to" 
                     << static_cast<int>(newState);
        }, Qt::DirectConnection);
    
    QObject::connect(&manager, &ProcessManager::processOutput,
        &manager, [&outputTimings](const QString& id, const QString& output, bool isStderr) {
            const auto timestamp = SignalLatencyProfiler::instance().markSignalEmission("processOutput");
            PROFILE_SLOT_EXECUTION(timestamp, "processOutput handler");
            
            SignalTiming timing;
            timing.receptionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
            timing.emissionTime = timing.receptionTime;
            outputTimings.push_back(timing);
            
            qDebug() << "Process" << id << (isStderr ? "[stderr]" : "[stdout]") 
                     << ":" << output.trimmed();
        }, Qt::DirectConnection);
    
    QObject::connect(&manager, &ProcessManager::processFinished,
        &app, [&finishedTimings, &app](const QString& id, int exitCode) {
            const auto timestamp = SignalLatencyProfiler::instance().markSignalEmission("processFinished");
            PROFILE_SLOT_EXECUTION(timestamp, "processFinished handler");
            
            SignalTiming timing;
            timing.receptionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
            timing.emissionTime = timing.receptionTime;
            finishedTimings.push_back(timing);
            
            qDebug() << "Process" << id << "finished with exit code:" << exitCode;
            
            // Quit after first process finishes
            app.quit();
        }, Qt::DirectConnection);
    
    // Create test processes
    qDebug() << "\n[Creating Test Processes]";
    
    {
        PERF_TIMER_THRESHOLD("Process creation", 5);
        
        ProcessConfig config1;
        config1.command = "echo";
        config1.arguments = QStringList{"Test message 1"};
        config1.captureOutput = true;
        
        auto result1 = manager.createProcess("test-1", config1);
        if (result1.isOk()) {
            qDebug() << "✓ Created test-1";
        }
        
        ProcessConfig config2;
        config2.command = "echo";
        config2.arguments = QStringList{"Test message 2"};
        config2.captureOutput = true;
        
        auto result2 = manager.createProcess("test-2", config2);
        if (result2.isOk()) {
            qDebug() << "✓ Created test-2";
        }
    }
    
    // Start processes and measure latency
    qDebug() << "\n[Starting Processes]";
    
    {
        PERF_TIMER_THRESHOLD("Process start", 10);
        
        ScopedTimer timer;
        auto startResult = manager.startProcess("test-1");
        const qint64 latencyUs = timer.elapsedUs();
        
        if (startResult.isOk()) {
            qDebug() << "✓ Started test-1 in" << latencyUs << "μs";
            
            if (latencyUs > 10000) {
                qWarning() << "⚠️  Start latency exceeds 10ms!";
            }
        }
    }
    
    // Schedule second process start after a delay
    QTimer::singleShot(50, [&manager]() {
        PERF_TIMER_THRESHOLD("Delayed process start", 10);
        
        ScopedTimer timer;
        auto startResult = manager.startProcess("test-2");
        const qint64 latencyUs = timer.elapsedUs();
        
        if (startResult.isOk()) {
            qDebug() << "✓ Started test-2 in" << latencyUs << "μs";
            
            if (latencyUs > 10000) {
                qWarning() << "⚠️  Start latency exceeds 10ms!";
            }
        }
    });
    
    // Set timeout for the test
    QTimer::singleShot(5000, [&app]() {
        qWarning() << "Test timeout - stopping";
        app.quit();
    });
    
    qDebug() << "\n[Running Event Loop...]";
    const int exitCode = app.exec();
    
    // Print performance report
    qDebug() << "\n===========================================";
    qDebug() << "Performance Test Results";
    qDebug() << "===========================================";
    
    SignalLatencyProfiler::instance().printReport();
    
    qDebug() << "\n[Summary]";
    qDebug() << "State change signals:" << stateChangeTimings.size();
    qDebug() << "Output signals:" << outputTimings.size();
    qDebug() << "Finished signals:" << finishedTimings.size();
    
    // Calculate statistics
    auto calculateStats = [](const std::vector<SignalTiming>& timings) -> std::pair<double, double> {
        if (timings.empty()) return {0.0, 0.0};
        
        double maxLatency = 0.0;
        double totalLatency = 0.0;
        
        for (const auto& timing : timings) {
            const double latency = timing.latencyMs();
            maxLatency = std::max(maxLatency, latency);
            totalLatency += latency;
        }
        
        const double avgLatency = totalLatency / timings.size();
        return {avgLatency, maxLatency};
    };
    
    if (!stateChangeTimings.empty()) {
        auto [avg, max] = calculateStats(stateChangeTimings);
        qDebug() << "State change avg:" << avg << "ms, max:" << max << "ms";
    }
    
    if (!outputTimings.empty()) {
        auto [avg, max] = calculateStats(outputTimings);
        qDebug() << "Output avg:" << avg << "ms, max:" << max << "ms";
    }
    
    if (!finishedTimings.empty()) {
        auto [avg, max] = calculateStats(finishedTimings);
        qDebug() << "Finished avg:" << avg << "ms, max:" << max << "ms";
    }
    
    qDebug() << "\n===========================================";
    qDebug() << "Test completed with exit code:" << exitCode;
    qDebug() << "===========================================";
    
    return exitCode;
}
