#include "core/ProcessManager.h"
#include "core/Project.h"
#include "core/CircularBuffer.h"
#include "core/MemoryMonitor.hpp"
#include "ui/ProjectManager.h"
#include "types/CommonTypes.h"
#include "storage/SettingsManager.h"
#include "storage/WorkspaceRepository.h"
#include "storage/ProjectRepository.h"
#include "storage/ApplicationStateManager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QDebug>
#include <iostream>
#include <memory>

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
    
    // Set application metadata for QSettings
    QCoreApplication::setOrganizationName("ZenRunner");
    QCoreApplication::setApplicationName("ZenRunner");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    qDebug() << "\n========================================";
    qDebug() << "ZenRunner - High-Performance Process Manager";
    qDebug() << "Version:" << QCoreApplication::applicationVersion();
    qDebug() << "========================================\n";
    
    // Log initial memory usage
    qDebug() << "[Initial Memory Usage]";
    Memory::MemoryMonitor::logUsage();
    
    // Initialize state persistence system
    qDebug() << "\n[Initializing State Persistence System]";
    auto settingsManager = Storage::createSettingsManager();
    auto workspaceRepo = std::make_shared<Storage::WorkspaceRepository>();
    auto projectRepo = std::make_shared<Storage::ProjectRepository>();
    
    auto stateManager = Storage::createApplicationStateManager(
        settingsManager, workspaceRepo, projectRepo);
    
    if (!stateManager->initialize()) {
        qWarning() << "Failed to initialize state manager - continuing with defaults";
    } else {
        qDebug() << "State persistence initialized successfully";
        
        // Log first run status
        if (stateManager->isFirstRun()) {
            qDebug() << "First run detected - welcome to ZenRunner!";
            stateManager->setFirstRunComplete();
        } else {
            qDebug() << "Restored previous session state";
            
            // Log restored state
            QString lastWorkspace = stateManager->getLastWorkspaceId();
            if (!lastWorkspace.isEmpty()) {
                qDebug() << "  Last workspace:" << lastWorkspace;
            }
            
            auto recentProjects = stateManager->getRecentProjects(5);
            if (!recentProjects.isEmpty()) {
                qDebug() << "  Recent projects:" << recentProjects.size();
            }
        }
    }
    
    // Enable auto-save every 60 seconds
    stateManager->setAutoSaveEnabled(true, 60);
    qDebug() << "Auto-save enabled (60 second interval)";
    
    // Create ProjectManager instance
    UI::ProjectManager projectManager;
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose managers to QML
    engine.rootContext()->setContextProperty("projectManager", &projectManager);
    engine.rootContext()->setContextProperty("stateManager", stateManager.get());
    
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
    qDebug() << "[Memory Usage After Setup]";
    Memory::MemoryMonitor::logUsage();
    
    // Setup cleanup on exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        qDebug() << "\n[Application shutting down]";
        qDebug() << "Saving application state...";
        
        if (stateManager->saveState()) {
            qDebug() << "State saved successfully";
        } else {
            qWarning() << "Failed to save state";
        }
        
        qDebug() << "Goodbye!";
    });
    
    return app.exec();
}
