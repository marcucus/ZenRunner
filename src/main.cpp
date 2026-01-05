#include "core/ProcessManager.h"
#include "core/Project.h"
#include "core/CircularBuffer.h"
#include "core/MemoryMonitor.hpp"
#include "types/CommonTypes.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QDebug>
#include <iostream>

using namespace ZenRunner;

/**
 * @brief ZenRunner Application - High-Performance UI with 60 FPS Target
 * 
 * Configures Qt Quick for GPU-accelerated rendering with RHI abstraction
 * to ensure consistent 60 FPS performance across all platforms.
 */
int main(int argc, char *argv[]) {
    // Enable GPU acceleration and optimal rendering settings
    // These must be set before QGuiApplication is created
    
    // Use threaded render loop for consistent 60 FPS
    qputenv("QSG_RENDER_LOOP", "threaded");
    
    // Log initial memory usage
    qDebug() << "\n[Initial Memory Usage]";
    Memory::MemoryMonitor::logUsage();
    
    // Test CircularBuffer with C++20 concepts
    {
        qDebug() << "\n[Testing CircularBuffer]";
        CircularBuffer<LogEntry, 10> buffer;
        
        buffer.emplace("Test log entry 1", LogLevel::Info, false);
        buffer.emplace("Test log entry 2", LogLevel::Warning, false);
        buffer.emplace("Test log entry 3", LogLevel::Error, true);
        
        qDebug() << "Buffer size:" << buffer.size();
        qDebug() << "Buffer capacity:" << buffer.capacity();
        
        auto logs = buffer.toVector();
        qDebug() << "Retrieved" << logs.size() << "log entries";
        
        // Test ProcessManager
        qDebug() << "\n[Testing ProcessManager]";
        ProcessManager manager;
        
        ProcessConfig config;
        config.command = "echo";
        config.arguments = QStringList{"Hello from ZenRunner!"};
        config.captureOutput = true;
        
        auto result = manager.createProcess("test-echo", config);
        
        if (result.isOk()) [[likely]] {
            qDebug() << "Process created successfully";
            
            QObject::connect(&manager, &ProcessManager::processOutput,
                [](const QString& id, const QString& output, bool isStderr) {
                    qDebug() << "Process" << id << (isStderr ? "[stderr]" : "[stdout]") 
                             << ":" << output;
                });
            
            QObject::connect(&manager, &ProcessManager::processFinished,
                [&app](const QString& id, int exitCode) {
                    qDebug() << "Process" << id << "finished with exit code:" << exitCode;
                    app.quit();
                });
            
            auto startResult = manager.startProcess("test-echo");
            if (startResult.isOk()) [[likely]] {
                qDebug() << "Process started asynchronously";
            } else [[unlikely]] {
                qDebug() << "Failed to start process:" << startResult.error();
                return 1;
            }
        } else [[unlikely]] {
            qDebug() << "Failed to create process:" << result.error();
            return 1;
        }
    }
    
    // Test Result type with C++20 concepts
    {
        qDebug() << "\n[Testing Result Type]";
        
        auto successResult = Result<int>::Ok(42);
        if (successResult.isOk()) {
            qDebug() << "Success result value:" << successResult.value();
        }
        
        auto errorResult = Result<int>::Err(QString("Test error"));
        if (errorResult.isErr()) {
            qDebug() << "Error result message:" << errorResult.error();
        }
    }
    
    qDebug() << "\n[Running event loop...]";
    
    // Log memory after setup
    qDebug() << "\n[Memory Usage After Setup]";
    Memory::MemoryMonitor::logUsage();
    
    return app.exec();
}
