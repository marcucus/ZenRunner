#include "core/ProcessManager.h"
#include "core/Project.h"
#include "core/CircularBuffer.h"
#include "core/MemoryMonitor.hpp"
#include "ui/ProjectManager.h"
#include "types/CommonTypes.h"
#include "platform/NativePlatformManager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QDebug>
#include <iostream>

using namespace ZenRunner;

/**
 * @brief ZenRunner Application with Native OS Integration
 * 
 * This version demonstrates full native OS integration including:
 * - Windows: Mica effect via DWM
 * - macOS: Vibrancy effect and notch detection
 * - System tray with real-time status icons
 * - Native notifications
 */
int main(int argc, char *argv[]) {
    // Enable GPU acceleration and optimal rendering settings
    qputenv("QSG_RENDER_LOOP", "threaded");
    
    QGuiApplication app(argc, argv);
    
    // Set application metadata for system tray
    app.setApplicationName("ZenRunner");
    app.setOrganizationName("ZenRunner");
    app.setApplicationDisplayName("ZenRunner - Native Process Manager");
    
    // Log initial memory usage
    qDebug() << "\n[Initial Memory Usage]";
    Memory::MemoryMonitor::logUsage();
    
    // Create Native Platform Manager
    Platform::NativePlatformManager platformManager;
    
    // Initialize system tray
    if (platformManager.initializeSystemTray()) {
        qDebug() << "System tray initialized successfully";
    } else {
        qDebug() << "System tray not available on this platform";
    }
    
    // Create ProjectManager instance
    UI::ProjectManager projectManager;
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose ProjectManager to QML
    engine.rootContext()->setContextProperty("projectManager", &projectManager);
    engine.rootContext()->setContextProperty("platformManager", &platformManager);
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url, &platformManager](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
            return;
        }
        
        // Apply native effects to the window
        if (obj) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(obj);
            if (window) {
                if (platformManager.initializeNativeEffects(window)) {
                    qDebug() << "Native effects applied successfully";
                } else {
                    qDebug() << "Native effects not available on this platform";
                }
            }
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    // Connect platform manager signals to handle tray interactions
    QObject::connect(&platformManager, &Platform::NativePlatformManager::showRequested,
                     &app, [&engine]() {
        // Show the main window
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
    
    // Example: Update tray state based on project manager state
    QObject::connect(&projectManager, &UI::ProjectManager::projectsChanged,
                     &platformManager, [&platformManager, &projectManager]() {
        // This is a simple example - in a real app you'd check actual process states
        auto projects = projectManager.getProjects();
        if (projects.isEmpty()) {
            platformManager.setTrayState(Platform::TrayIconState::Idle);
        } else {
            // Check if any processes are running
            bool hasRunningProcesses = false;
            // ... check process states ...
            
            if (hasRunningProcesses) {
                platformManager.setTrayState(Platform::TrayIconState::Active);
            } else {
                platformManager.setTrayState(Platform::TrayIconState::Idle);
            }
        }
    });
    
    qDebug() << "\n[Application started with native integration]";
    
    // Log memory after setup
    qDebug() << "\n[Memory Usage After Setup]";
    Memory::MemoryMonitor::logUsage();
    
    // Show welcome notification
    platformManager.showNotification(
        "ZenRunner Started",
        "Native process manager is running in the background"
    );
    
    return app.exec();
}
