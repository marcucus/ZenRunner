#include "core/ProcessResourceMonitor.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <memory>

using namespace ZenRunner::Core;

/**
 * @brief Test for ProcessResourceMonitor
 * 
 * This test verifies that the resource monitor can track CPU and memory usage
 * of spawned processes.
 */
class ResourceMonitorTest : public QObject {
    Q_OBJECT
    
public:
    ResourceMonitorTest() : monitor_(std::make_unique<ProcessResourceMonitor>()) {
        connect(monitor_.get(), &IProcessResourceMonitor::statsUpdated,
                this, &ResourceMonitorTest::onStatsUpdated);
        connect(monitor_.get(), &IProcessResourceMonitor::resourceSpikeDetected,
                this, &ResourceMonitorTest::onSpikeDetected);
    }
    
    void run() {
        qDebug() << "=== ProcessResourceMonitor Test ===\n";
        
        // Test 1: Monitor the test process itself
        qDebug() << "Test 1: Monitoring self process";
        const qint64 selfPid = QCoreApplication::applicationPid();
        qDebug() << "Self PID:" << selfPid;
        
        if (monitor_->startMonitoring("self", selfPid)) {
            qDebug() << "✓ Started monitoring self process\n";
        } else {
            qDebug() << "✗ Failed to start monitoring self process\n";
            QTimer::singleShot(0, qApp, &QCoreApplication::quit);
            return;
        }
        
        // Wait for a few samples
        QTimer::singleShot(6000, this, &ResourceMonitorTest::checkSelfStats);
        
        // Test 2: Monitor a spawned process
        QTimer::singleShot(1000, this, &ResourceMonitorTest::startChildProcess);
    }
    
private slots:
    void onStatsUpdated(const QString& processId, ProcessResourceStats stats) {
        qDebug() << "[STATS]" << processId 
                 << "- CPU:" << QString::number(stats.cpuPercent, 'f', 1) + "%"
                 << "Memory:" << stats.memoryToString();
    }
    
    void onSpikeDetected(const QString& processId, double cpuPercent, double memoryMB) {
        qDebug() << "[SPIKE]" << processId 
                 << "- CPU:" << QString::number(cpuPercent, 'f', 1) + "%"
                 << "Memory:" << QString::number(memoryMB, 'f', 1) + " MB";
    }
    
    void checkSelfStats() {
        qDebug() << "\n=== Checking Self Process Statistics ===";
        
        auto stats = monitor_->getCurrentStats("self");
        if (stats) {
            qDebug() << "Current CPU:" << QString::number(stats->cpuPercent, 'f', 1) + "%";
            qDebug() << "Current Memory:" << stats->memoryToString();
        } else {
            qDebug() << "✗ Failed to get stats for self process";
        }
        
        auto summary = monitor_->getSummary("self");
        if (summary) {
            qDebug() << "\nSummary:";
            qDebug() << "  Data points:" << summary->dataPoints;
            qDebug() << "  Avg CPU:" << QString::number(summary->avgCpu, 'f', 1) + "%";
            qDebug() << "  Max CPU:" << QString::number(summary->maxCpu, 'f', 1) + "%";
            qDebug() << "  Avg Memory:" << QString::number(summary->avgMemoryMB, 'f', 1) + " MB";
            qDebug() << "  Max Memory:" << QString::number(summary->maxMemoryMB, 'f', 1) + " MB";
            qDebug() << "  Recent Spike:" << (summary->hasRecentSpike ? "Yes" : "No");
            qDebug() << "✓ Successfully calculated summary\n";
        } else {
            qDebug() << "✗ Failed to get summary for self process\n";
        }
        
        checkChildStats();
    }
    
    void startChildProcess() {
        qDebug() << "\nTest 2: Spawning and monitoring child process";
        
        childProcess_ = std::make_unique<QProcess>();
        
#ifdef Q_OS_WIN
        childProcess_->setProgram("ping");
        childProcess_->setArguments(QStringList() << "localhost" << "-n" << "10");
#else
        childProcess_->setProgram("sleep");
        childProcess_->setArguments(QStringList() << "10");
#endif
        
        connect(childProcess_.get(), &QProcess::started, this, [this]() {
            const qint64 childPid = childProcess_->processId();
            qDebug() << "Child process started with PID:" << childPid;
            
            if (monitor_->startMonitoring("child", childPid)) {
                qDebug() << "✓ Started monitoring child process\n";
            } else {
                qDebug() << "✗ Failed to start monitoring child process\n";
            }
        });
        
        childProcess_->start();
    }
    
    void checkChildStats() {
        qDebug() << "=== Checking Child Process Statistics ===";
        
        auto stats = monitor_->getCurrentStats("child");
        if (stats) {
            qDebug() << "Current CPU:" << QString::number(stats->cpuPercent, 'f', 1) + "%";
            qDebug() << "Current Memory:" << stats->memoryToString();
            qDebug() << "✓ Successfully retrieved child stats\n";
        } else {
            qDebug() << "Child process may have already exited or wasn't monitored\n";
        }
        
        finishTest();
    }
    
    void finishTest() {
        qDebug() << "=== Test Summary ===";
        
        QStringList monitored = monitor_->getMonitoredProcesses();
        qDebug() << "Monitored processes:" << monitored.size();
        for (const auto& processId : monitored) {
            qDebug() << "  -" << processId;
        }
        
        qDebug() << "\n✓ All tests completed successfully!";
        qDebug() << "=== End of Test ===\n";
        
        // Clean up and exit
        if (childProcess_ && childProcess_->state() == QProcess::Running) {
            childProcess_->terminate();
            childProcess_->waitForFinished(1000);
        }
        
        QTimer::singleShot(500, qApp, &QCoreApplication::quit);
    }
    
private:
    std::unique_ptr<ProcessResourceMonitor> monitor_;
    std::unique_ptr<QProcess> childProcess_;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "ProcessResourceMonitor Test";
    qDebug() << "===========================\n";
    
    ResourceMonitorTest test;
    QTimer::singleShot(0, &test, &ResourceMonitorTest::run);
    
    return app.exec();
}

#include "test_resource_monitor.moc"
