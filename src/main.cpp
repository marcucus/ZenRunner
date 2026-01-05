#include "core/ProcessManager.h"
#include "core/Project.h"
#include "core/CircularBuffer.h"
#include "core/MemoryMonitor.hpp"
#include "ui/ProjectManager.h"
#include "types/CommonTypes.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
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
    
    QGuiApplication app(argc, argv);
    
    // Log initial memory usage
    qDebug() << "\n[Initial Memory Usage]";
    Memory::MemoryMonitor::logUsage();
    
    // Create ProjectManager instance
    UI::ProjectManager projectManager;
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose ProjectManager to QML
    engine.rootContext()->setContextProperty("projectManager", &projectManager);
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    
    qDebug() << "\n[Application started]";
    
    // Log memory after setup
    qDebug() << "\n[Memory Usage After Setup]";
    Memory::MemoryMonitor::logUsage();
    
    return app.exec();
}
