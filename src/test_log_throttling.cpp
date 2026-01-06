/**
 * @file test_log_throttling.cpp
 * @brief Test suite for log throttling mechanism in LogViewModel
 * 
 * This test validates that the throttling system prevents UI updates
 * from overwhelming the rendering system during log floods.
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include "core/LogBuffer.h"
#include "ui/LogViewModel.h"

using namespace ZenRunner;

class TestLogThrottling : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief Test that throttling limits update frequency
     */
    void testThrottlingLimitsUpdateFrequency() {
        // Create log buffer and view model
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        auto logViewModel = UI::createLogViewModel();
        
        // Set throttle interval to 50ms for testing
        logViewModel->setThrottleInterval(50);
        QCOMPARE(logViewModel->getThrottleInterval(), 50);
        
        // Connect log buffer to view model
        logViewModel->setLogBuffer(logBuffer);
        
        // Track number of UI updates
        int updateCount = 0;
        QObject::connect(logViewModel.get(), &UI::ILogViewModel::logsAdded, [&updateCount]() {
            updateCount++;
        });
        
        // Add many logs rapidly (simulating log flood)
        QElapsedTimer timer;
        timer.start();
        
        for (int i = 0; i < 100; i++) {
            logBuffer->append(QString("Log line %1").arg(i), false);
        }
        
        // Process events to allow timers to fire
        QCoreApplication::processEvents();
        qint64 elapsed = timer.elapsed();
        
        // Give time for throttled updates to occur
        QTest::qWait(200);
        QCoreApplication::processEvents();
        
        // With 50ms throttle and 200ms wait, we should have at most 4-5 updates
        // (even though we added 100 logs)
        QVERIFY(updateCount <= 6);
        QVERIFY(updateCount >= 1);
        
        qDebug() << "Added 100 logs in" << elapsed << "ms";
        qDebug() << "UI was updated" << updateCount << "times (throttled)";
    }
    
    /**
     * @brief Test that immediate refresh bypasses throttling
     */
    void testImmediateRefreshBypassesThrottling() {
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        auto logViewModel = UI::createLogViewModel();
        
        logViewModel->setThrottleInterval(100);
        logViewModel->setLogBuffer(logBuffer);
        
        // Add logs
        for (int i = 0; i < 10; i++) {
            logBuffer->append(QString("Log %1").arg(i), false);
        }
        
        // Immediate refresh should work right away
        logViewModel->refresh();
        
        // Check that logs are visible immediately
        QCOMPARE(logViewModel->rowCount(), 10);
    }
    
    /**
     * @brief Test that throttled updates eventually show all logs
     */
    void testThrottledUpdatesShowAllLogs() {
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        auto logViewModel = UI::createLogViewModel();
        
        logViewModel->setThrottleInterval(30);
        logViewModel->setLogBuffer(logBuffer);
        
        // Add logs
        int logCount = 50;
        for (int i = 0; i < logCount; i++) {
            logBuffer->append(QString("Test log %1").arg(i), false);
        }
        
        // Wait for throttled updates to complete
        QTest::qWait(100);
        QCoreApplication::processEvents();
        
        // All logs should eventually be visible
        QCOMPARE(logViewModel->rowCount(), logCount);
    }
    
    /**
     * @brief Test callback mechanism in LogBuffer
     */
    void testLogBufferCallbackMechanism() {
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        
        int callbackCount = 0;
        logBuffer->setUpdateCallback([&callbackCount]() {
            callbackCount++;
        });
        
        // Add logs
        for (int i = 0; i < 10; i++) {
            logBuffer->append(QString("Log %1").arg(i), false);
        }
        
        // Callback should have been called for each log
        QCOMPARE(callbackCount, 10);
    }
    
    /**
     * @brief Test configurable throttle interval
     */
    void testConfigurableThrottleInterval() {
        auto logViewModel = UI::createLogViewModel();
        
        // Default should be 16ms (60 FPS)
        QCOMPARE(logViewModel->getThrottleInterval(), 16);
        
        // Set to different values
        logViewModel->setThrottleInterval(33); // ~30 FPS
        QCOMPARE(logViewModel->getThrottleInterval(), 33);
        
        logViewModel->setThrottleInterval(8); // ~120 FPS
        QCOMPARE(logViewModel->getThrottleInterval(), 8);
    }
    
    /**
     * @brief Test that requestLogUpdate uses throttling
     */
    void testRequestLogUpdateUsesThrottling() {
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        auto logViewModel = UI::createLogViewModel();
        
        logViewModel->setThrottleInterval(50);
        logViewModel->setLogBuffer(logBuffer);
        
        int updateCount = 0;
        QObject::connect(logViewModel.get(), &UI::ILogViewModel::logsAdded, [&updateCount]() {
            updateCount++;
        });
        
        // Manually request many log updates
        for (int i = 0; i < 50; i++) {
            logViewModel->requestLogUpdate();
        }
        
        QCoreApplication::processEvents();
        QTest::qWait(150);
        QCoreApplication::processEvents();
        
        // Should have throttled to only a few updates
        QVERIFY(updateCount <= 4);
        QVERIFY(updateCount >= 1);
        
        qDebug() << "Requested 50 updates, got" << updateCount << "actual updates (throttled)";
    }
    
    /**
     * @brief Stress test: simulate realistic log flood scenario
     */
    void testRealisticLogFloodScenario() {
        auto logBuffer = std::shared_ptr<Core::ILogBuffer>(Core::createLogBuffer(5000));
        auto logViewModel = UI::createLogViewModel();
        
        // Use default throttle (16ms for 60 FPS)
        logViewModel->setLogBuffer(logBuffer);
        
        int updateCount = 0;
        QObject::connect(logViewModel.get(), &UI::ILogViewModel::logsAdded, [&updateCount]() {
            updateCount++;
        });
        
        // Simulate a process that generates 1000 log lines rapidly
        QElapsedTimer timer;
        timer.start();
        
        for (int i = 0; i < 1000; i++) {
            logBuffer->append(QString("Build output line %1: Compiling file_%1.cpp").arg(i), false);
            
            // Occasionally process events (simulating event loop)
            if (i % 100 == 0) {
                QCoreApplication::processEvents();
            }
        }
        
        qint64 logGenerationTime = timer.elapsed();
        
        // Wait for all throttled updates to complete
        QTest::qWait(100);
        QCoreApplication::processEvents();
        
        // Verify all logs are present
        QCOMPARE(logViewModel->rowCount(), 1000);
        
        // With 16ms throttle, in logGenerationTime + 100ms, we should have
        // significantly fewer updates than logs
        QVERIFY(updateCount < 1000);
        
        qDebug() << "Generated 1000 logs in" << logGenerationTime << "ms";
        qDebug() << "UI was updated only" << updateCount << "times (throttled)";
        qDebug() << "Throttling reduced update calls by" 
                 << (100.0 * (1000 - updateCount) / 1000.0) << "%";
    }
};

QTEST_MAIN(TestLogThrottling)
#include "test_log_throttling.moc"
