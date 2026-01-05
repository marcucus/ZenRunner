#include "core/ProcessManager.h"
#include "core/Workspace.h"
#include "storage/WorkspaceRepository.h"
#include "ui/WorkspaceViewModel.h"
#include "core/MemoryMonitor.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDebug>

using namespace ZenRunner;

/**
 * @brief ZenRunner Application with Workspace Management
 * 
 * Demonstrates the workspace system with batch process management.
 */
int main(int argc, char *argv[]) {
    // Enable GPU acceleration and optimal rendering settings
    qputenv("QSG_RENDER_LOOP", "threaded");
    
    QGuiApplication app(argc, argv);
    
    // Set application metadata
    app.setOrganizationName("ZenRunner");
    app.setOrganizationDomain("zenrunner.dev");
    app.setApplicationName("ZenRunner");
    
    qDebug() << "\n[ZenRunner Starting]";
    qDebug() << "[Initial Memory Usage]";
    Memory::MemoryMonitor::logUsage();
    
    // Initialize core components
    ProcessManager processManager;
    qDebug() << "[ProcessManager] Initialized";
    
    // Initialize storage layer
    Storage::WorkspaceRepository workspaceRepository;
    qDebug() << "[WorkspaceRepository] Storage directory:" << workspaceRepository.getStorageDirectory();
    
    // Initialize UI view models
    UI::WorkspaceViewModel workspaceViewModel;
    workspaceViewModel.setRepository(&workspaceRepository);
    // Note: ProcessManager needs to implement Core::IProcessManager interface
    // For now, we'll use dynamic_cast which is safer
    workspaceViewModel.setProcessManager(dynamic_cast<Core::IProcessManager*>(&processManager));
    qDebug() << "[WorkspaceViewModel] Initialized";
    
    // Load existing workspaces
    workspaceViewModel.loadWorkspaces();
    qDebug() << "[WorkspaceViewModel] Loaded" << workspaceViewModel.count() << "workspaces";
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose view models to QML
    engine.rootContext()->setContextProperty("workspaceViewModel", &workspaceViewModel);
    
    // Load main QML
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "Failed to load QML";
                QCoreApplication::exit(-1);
            } else {
                qDebug() << "[QML] Loaded successfully";
            }
        }, Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "No root QML objects loaded";
        return -1;
    }
    
    qDebug() << "[Memory Usage After Initialization]";
    Memory::MemoryMonitor::logUsage();
    qDebug() << "[Event Loop] Starting...\n";
    
    return app.exec();
}
