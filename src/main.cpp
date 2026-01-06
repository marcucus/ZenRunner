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
#include "platform/NativePlatformManager.h"

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
    
    // Create Native Platform Manager for system tray and notifications
    // Note: Stack allocation is safe here - lives until app.exec() returns
    Platform::NativePlatformManager platformManager;
    
    // Initialize system tray
    if (platformManager.initializeSystemTray()) {
        qDebug() << "System tray initialized successfully";
    } else {
        qDebug() << "System tray not available on this platform";
    }
    
    // Create ProcessManager instance for managing all processes
    // Note: Stack allocation is safe here - lives until app.exec() returns
    ProcessManager processManager;
    
    // Create ProjectManager instance
    UI::ProjectManager projectManager;
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose managers to QML
    engine.rootContext()->setContextProperty("projectManager", &projectManager);
    engine.rootContext()->setContextProperty("stateManager", stateManager.get());
    engine.rootContext()->setContextProperty("processManager", &processManager);
    engine.rootContext()->setContextProperty("platformManager", &platformManager);
    
    // Connect process crash events to notification system
    QObject::connect(&processManager, &ProcessManager::processCrashed,
                     &platformManager, [&platformManager](const QString& id, int exitCode) {
        qWarning() << "Process crashed:" << id << "with exit code:" << exitCode;
        platformManager.showNotification(
            "Process Crashed",
            QString("Process '%1' crashed with exit code %2").arg(id).arg(exitCode)
        );
        platformManager.setTrayState(Platform::TrayIconState::Error);
    });
    
    // Connect process error events to notification system
    QObject::connect(&processManager, &ProcessManager::processError,
                     &platformManager, [&platformManager](const QString& id, const QString& error) {
        qWarning() << "Process error:" << id << "-" << error;
        platformManager.showNotification(
            "Process Error",
            QString("Process '%1': %2").arg(id, error)
        );
        platformManager.setTrayState(Platform::TrayIconState::Warning);
    });
    
    // Update tray state based on process states
    QObject::connect(&processManager, &ProcessManager::processStateChanged,
                     &platformManager, [&platformManager, &processManager](const QString& /*id*/, ProcessState newState) {
        if (newState == ProcessState::Running) {
            if (processManager.hasRunningProcesses()) {
                platformManager.setTrayState(Platform::TrayIconState::Active);
            }
        } else if (newState == ProcessState::Finished || newState == ProcessState::Stopped) {
            if (!processManager.hasRunningProcesses()) {
                platformManager.setTrayState(Platform::TrayIconState::Idle);
            }
        }
    });
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url, &platformManager](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
            return;
        }
        
        // Apply native effects if window is created
        if (obj) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(obj);
            if (window && platformManager.getNativeEffects()) {
                if (platformManager.initializeNativeEffects(window)) {
                    qDebug() << "Native effects applied successfully";
                }
            }
        }
    }, Qt::QueuedConnection);
    engine.load(url);
    
    // Connect platform manager signals for window management
    QObject::connect(&platformManager, &Platform::NativePlatformManager::showRequested,
                     &app, [&engine]() {
        auto rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObjects.first());
            if (window) {
                window->show();
                window->raise();
                window->requestActivate();
            }
        }
    });
    
    QObject::connect(&platformManager, &Platform::NativePlatformManager::quitRequested,
                     &app, &QGuiApplication::quit);
    
    qDebug() << "\n[Application started]";
    
    // Log memory after setup
    qDebug() << "[Memory Usage After Setup]";
    Memory::MemoryMonitor::logUsage();
    
    // Setup cleanup on exit - ensure all processes are stopped gracefully
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        qDebug() << "\n[Application shutting down]";
        
        // Stop all running processes gracefully before exit
        if (processManager.hasRunningProcesses()) {
            qDebug() << "Stopping" << processManager.runningCount() << "running processes...";
            processManager.stopAll(5000);  // 5 second timeout for graceful termination
            
            // Allow event loop to process termination signals
            // ProcessManager destructor will force-kill any remaining processes
            // The 100ms timeout is just for initial signal processing
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        
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
