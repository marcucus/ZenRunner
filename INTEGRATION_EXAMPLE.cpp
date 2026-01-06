// Example integration code for main.cpp
// Add these lines to integrate the statistics dashboard

// 1. Add includes at the top of main.cpp:
#include "core/ProcessResourceMonitor.h"
#include "ui/StatisticsViewModel.h"

// 2. In main(), after creating projectManager, add:

// Create resource monitor for statistics
auto resourceMonitor = std::make_shared<Core::ProcessResourceMonitor>();
qDebug() << "Resource monitor initialized";

// Create statistics view model
auto statisticsViewModel = new UI::StatisticsViewModel();
statisticsViewModel->setResourceMonitor(resourceMonitor);
qDebug() << "Statistics view model initialized";

// Expose to QML
engine.rootContext()->setContextProperty("statisticsViewModel", statisticsViewModel);

// 3. To connect with ProcessManager, you need to modify ProcessManager to notify
//    the resource monitor when processes start/stop. Here's an example:

// In ProcessManager class, add:
void ProcessManager::setResourceMonitor(std::shared_ptr<Core::IProcessResourceMonitor> monitor) {
    resourceMonitor_ = monitor;
}

// In AsyncProcess::onStarted() callback:
void AsyncProcess::onStarted() {
    setState(ProcessState::Running);
    startTime_ = QDateTime::currentDateTime();
    
    // Notify resource monitor
    if (processManager_->getResourceMonitor()) {
        const qint64 pid = process_->processId();
        processManager_->getResourceMonitor()->startMonitoring(config_.processId, pid);
        qDebug() << "Started monitoring" << config_.processId << "PID:" << pid;
    }
    
    emit started(config_.processId);
}

// In AsyncProcess::onFinished() callback:
void AsyncProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    // ... existing code ...
    
    // Stop monitoring
    if (processManager_->getResourceMonitor()) {
        processManager_->getResourceMonitor()->stopMonitoring(config_.processId);
        qDebug() << "Stopped monitoring" << config_.processId;
    }
    
    emit finished(config_.processId, exitCode, exitStatus);
}
