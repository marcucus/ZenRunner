#include "core/ILogBuffer.hpp"
#include "core/LogBuffer.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <cassert>

using namespace ZenRunner::Core;

// Test helper
void printLogEntry(const LogEntry& entry) {
    qDebug() << "\n--- Log Entry ---";
    qDebug() << "Original text:" << entry.text;
    qDebug() << "Plain text:" << entry.plainText;
    qDebug() << "Has ANSI codes:" << entry.hasAnsiCodes;
    qDebug() << "Is error:" << entry.isError;
    qDebug() << "Segments:" << entry.segments.size();
    
    for (size_t i = 0; i < entry.segments.size(); ++i) {
        const auto& seg = entry.segments[i];
        qDebug() << "  [" << i << "]" << seg.text 
                 << "- FG:" << seg.foregroundColor.name()
                 << "Bold:" << seg.bold;
    }
}

// Test basic log storage
void testBasicLogging() {
    auto buffer = createLogBuffer(100);
    
    buffer->append("Plain log message");
    buffer->append("Error occurred!", true);
    
    assert(buffer->size() == 2);
    
    auto logs = buffer->getAll();
    assert(logs.size() == 2);
    assert(logs[0].plainText == "Plain log message");
    assert(logs[1].plainText == "Error occurred!");
    assert(logs[1].isError == true);
    
    qDebug() << "✓ Basic logging test passed";
}

// Test ANSI code parsing through LogBuffer
void testAnsiParsing() {
    auto buffer = createLogBuffer(100);
    
    // Add logs with ANSI codes
    buffer->append("\x1b[31mError:\x1b[0m Something went wrong");
    buffer->append("\x1b[32m✓\x1b[0m Test passed");
    buffer->append("\x1b[1;33mWARNING:\x1b[0m Deprecated API");
    
    assert(buffer->size() == 3);
    
    auto logs = buffer->getAll();
    
    // First entry: red error
    assert(logs[0].hasAnsiCodes == true);
    assert(logs[0].plainText == "Error: Something went wrong");
    assert(logs[0].segments.size() >= 2);
    assert(logs[0].segments[0].text == "Error:");
    printLogEntry(logs[0]);
    
    // Second entry: green checkmark
    assert(logs[1].hasAnsiCodes == true);
    assert(logs[1].plainText == "✓ Test passed");
    printLogEntry(logs[1]);
    
    // Third entry: bold yellow warning
    assert(logs[2].hasAnsiCodes == true);
    assert(logs[2].plainText == "WARNING: Deprecated API");
    assert(logs[2].segments[0].bold == true);
    printLogEntry(logs[2]);
    
    qDebug() << "✓ ANSI parsing test passed";
}

// Test complex real-world logs
void testRealWorldLogs() {
    auto buffer = createLogBuffer(100);
    
    // Simulate npm/yarn output
    buffer->append("\x1b[32m✓\x1b[0m \x1b[1mBuild successful\x1b[0m in \x1b[36m2.4s\x1b[0m");
    
    // Simulate compiler error
    buffer->append("\x1b[1merror:\x1b[0m \x1b[1m\x1b[31mundefined reference to 'main'\x1b[0m");
    
    // Simulate test output
    buffer->append("  \x1b[32m●\x1b[0m should handle user input");
    buffer->append("  \x1b[31m✕\x1b[0m should validate email \x1b[2m(12ms)\x1b[0m");
    
    auto logs = buffer->getAll();
    assert(logs.size() == 4);
    
    for (const auto& log : logs) {
        printLogEntry(log);
    }
    
    qDebug() << "✓ Real-world logs test passed";
}

// Test search with ANSI codes
void testSearchWithAnsi() {
    auto buffer = createLogBuffer(100);
    
    buffer->append("\x1b[31mERROR:\x1b[0m Database connection failed");
    buffer->append("\x1b[33mWARNING:\x1b[0m Slow query detected");
    buffer->append("\x1b[32mINFO:\x1b[0m Server started successfully");
    buffer->append("\x1b[31mERROR:\x1b[0m File not found");
    
    // Search should work on plain text
    auto results = buffer->search("ERROR");
    assert(results.size() == 2);
    assert(results[0].plainText.contains("Database"));
    assert(results[1].plainText.contains("File not found"));
    
    results = buffer->search("Server");
    assert(results.size() == 1);
    assert(results[0].plainText.contains("Server started"));
    
    qDebug() << "✓ Search with ANSI test passed";
}

// Test circular buffer overflow with ANSI codes
void testCircularBufferOverflow() {
    auto buffer = createLogBuffer(10);  // Small buffer
    
    // Add more than capacity
    for (int i = 0; i < 15; ++i) {
        QString log = QString("\x1b[3%1mLog entry %2\x1b[0m").arg(i % 7 + 1).arg(i);
        buffer->append(log);
    }
    
    // Should only have last 10
    assert(buffer->size() == 10);
    assert(buffer->isFull() == true);
    
    auto logs = buffer->getAll();
    assert(logs.size() == 10);
    
    // First log should be entry 5 (oldest kept)
    assert(logs[0].plainText.contains("Log entry 5"));
    
    // Last log should be entry 14 (newest)
    assert(logs[9].plainText.contains("Log entry 14"));
    
    qDebug() << "✓ Circular buffer overflow test passed";
}

// Test performance with many ANSI logs
void testPerformance() {
    auto buffer = createLogBuffer(5000);
    
    QElapsedTimer timer;
    timer.start();
    
    // Add 1000 logs with ANSI codes
    for (int i = 0; i < 1000; ++i) {
        QString log = QString("\x1b[3%1mLine %2:\x1b[0m \x1b[1mBold text\x1b[0m and \x1b[3mitalic\x1b[0m")
                        .arg(i % 7 + 1).arg(i);
        buffer->append(log);
    }
    
    qint64 elapsed = timer.elapsed();
    
    qDebug() << "\n==== Performance Test ====";
    qDebug() << "Added 1000 ANSI logs in" << elapsed << "ms";
    qDebug() << "Average per log:" << (double)elapsed / 1000.0 << "ms";
    
    // Should be very fast (< 50ms for 1000 logs)
    assert(elapsed < 50);
    assert(buffer->size() == 1000);
    
    // Test retrieval performance
    timer.restart();
    auto logs = buffer->getAll();
    elapsed = timer.elapsed();
    
    qDebug() << "Retrieved all logs in" << elapsed << "ms";
    assert(logs.size() == 1000);
    
    qDebug() << "✓ Performance test passed";
}

// Test plain logs without ANSI codes
void testPlainLogs() {
    auto buffer = createLogBuffer(100);
    
    buffer->append("Plain text without colors");
    buffer->append("Another plain log");
    
    auto logs = buffer->getAll();
    
    assert(logs[0].hasAnsiCodes == false);
    assert(logs[0].plainText == logs[0].text);
    assert(logs[0].segments.size() == 1);
    assert(logs[0].segments[0].text == "Plain text without colors");
    
    qDebug() << "✓ Plain logs test passed";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "==============================================";
    qDebug() << "    LogBuffer with ANSI Integration Test";
    qDebug() << "==============================================\n";
    
    testBasicLogging();
    testAnsiParsing();
    testRealWorldLogs();
    testSearchWithAnsi();
    testCircularBufferOverflow();
    testPlainLogs();
    testPerformance();
    
    qDebug() << "\n==============================================";
    qDebug() << "    ✓ ALL INTEGRATION TESTS PASSED";
    qDebug() << "==============================================\n";
    
    return 0;
}
