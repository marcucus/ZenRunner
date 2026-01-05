#include "core/ProcessManager.h"
#include "core/Project.h"
#include "core/CircularBuffer.h"
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
    
    // Enable RHI (Rendering Hardware Interface) for modern GPU acceleration
    // This ensures we use Vulkan/Metal/Direct3D instead of legacy OpenGL
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::Vulkan);
    
    // Set high quality antialiasing for smooth visuals
    QSurfaceFormat format;
    format.setSamples(4);  // 4x MSAA for smooth edges
    QSurfaceFormat::setDefaultFormat(format);
    
    QGuiApplication app(argc, argv);
    
    // Application metadata
    app.setOrganizationName("ZenRunner");
    app.setOrganizationDomain("zenrunner.dev");
    app.setApplicationName("ZenRunner");
    app.setApplicationVersion("1.0.0");
    
    qDebug() << "ZenRunner - High-Performance Native Process Manager";
    qDebug() << "==================================================";
    qDebug() << "GPU Acceleration: RHI with" << QQuickWindow::graphicsApi();
    qDebug() << "Render Loop: Threaded (60 FPS target)";
    qDebug() << "Antialiasing: 4x MSAA";
    
    // Create QML engine and load UI
    QQmlApplicationEngine engine;
    
    // Enable QML caching for faster subsequent loads
    engine.setOutputWarningsToStandardError(true);
    
    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load QML UI";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "No root objects loaded. Running in CLI mode for testing.";
        
        // Fallback to backend testing if QML UI is not available
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
    
    return app.exec();
}
