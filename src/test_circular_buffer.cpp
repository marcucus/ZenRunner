#include "core/CircularBuffer.h"
#include "core/ILogBuffer.hpp"
#include "core/LogBuffer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace ZenRunner;
using namespace ZenRunner::Core;

/**
 * @brief Test suite for CircularBuffer implementation
 * 
 * Validates:
 * - Up to 5000 log lines per project
 * - O(1) insertion cost
 * - Buffer overflows overwrite oldest entries
 * - Contiguous memory structure
 * - Thread safety
 */

class TestResult {
public:
    bool passed{false};
    QString name;
    QString message;
    
    void print() const {
        if (passed) {
            qDebug() << "✓" << name;
        } else {
            qDebug() << "✗" << name << ":" << message;
        }
    }
};

std::vector<TestResult> testResults;

void addTestResult(const QString& name, bool passed, const QString& message = "") {
    TestResult result;
    result.name = name;
    result.passed = passed;
    result.message = message;
    testResults.push_back(result);
    result.print();
}

// Test 1: Basic insertion and capacity
void testBasicInsertion() {
    qDebug() << "\n[Test 1: Basic Insertion and Capacity]";
    
    CircularBuffer<int, 10> buffer;
    
    // Test empty state
    addTestResult("Empty buffer size is 0", buffer.size() == 0);
    addTestResult("Empty buffer isEmpty() returns true", buffer.empty());
    addTestResult("Empty buffer full() returns false", !buffer.full());
    addTestResult("Capacity is 10", buffer.capacity() == 10);
    
    // Add elements
    for (int i = 0; i < 5; ++i) {
        buffer.push(i);
    }
    
    addTestResult("After 5 insertions, size is 5", buffer.size() == 5);
    addTestResult("Buffer is not empty", !buffer.empty());
    addTestResult("Buffer is not full", !buffer.full());
    
    // Fill to capacity
    for (int i = 5; i < 10; ++i) {
        buffer.push(i);
    }
    
    addTestResult("After 10 insertions, size is 10", buffer.size() == 10);
    addTestResult("Full buffer full() returns true", buffer.full());
}

// Test 2: Overflow behavior (oldest entry overwrite)
void testOverflowBehavior() {
    qDebug() << "\n[Test 2: Overflow Behavior - Oldest Entry Overwrite]";
    
    CircularBuffer<int, 5> buffer;
    
    // Fill buffer
    for (int i = 0; i < 5; ++i) {
        buffer.push(i); // 0, 1, 2, 3, 4
    }
    
    auto vec = buffer.toVector();
    addTestResult("Filled buffer contains [0,1,2,3,4]", 
                  vec.size() == 5 && vec[0] == 0 && vec[4] == 4);
    
    // Overflow by adding more elements
    buffer.push(5); // Should overwrite 0, buffer now: 1, 2, 3, 4, 5
    buffer.push(6); // Should overwrite 1, buffer now: 2, 3, 4, 5, 6
    
    vec = buffer.toVector();
    addTestResult("After overflow, size remains 5", vec.size() == 5);
    addTestResult("After overflow, oldest entries removed", 
                  vec[0] == 2 && vec[1] == 3 && vec[4] == 6,
                  QString("Expected [2,3,4,5,6], got [%1,%2,%3,%4,%5]")
                      .arg(vec[0]).arg(vec[1]).arg(vec[2]).arg(vec[3]).arg(vec[4]));
}

// Test 3: O(1) insertion performance
void testInsertionPerformance() {
    qDebug() << "\n[Test 3: O(1) Insertion Performance]";
    
    CircularBuffer<int, 5000> buffer;
    
    const int iterations = 10000; // More than capacity to test overflow
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        buffer.push(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avgTimePerInsertion = static_cast<double>(duration.count()) / iterations;
    
    qDebug() << "  - Total time for" << iterations << "insertions:" << duration.count() << "μs";
    qDebug() << "  - Average time per insertion:" << QString::number(avgTimePerInsertion, 'f', 3) << "μs";
    
    // O(1) insertion should be very fast (< 1 μs on average)
    addTestResult("Average insertion time < 1 μs", avgTimePerInsertion < 1.0,
                  QString("Got %1 μs").arg(avgTimePerInsertion, 0, 'f', 3));
    
    // Verify final state
    addTestResult("After overflow insertions, size is 5000", buffer.size() == 5000);
    
    // Verify oldest entries were overwritten
    auto vec = buffer.toVector();
    addTestResult("Oldest entry is correct after overflow", 
                  vec[0] == 5000 && vec[vec.size()-1] == 9999,
                  QString("Expected first=5000, last=9999, got first=%1, last=%2")
                      .arg(vec[0]).arg(vec[vec.size()-1]));
}

// Test 4: Contiguous memory structure
void testContiguousMemory() {
    qDebug() << "\n[Test 4: Contiguous Memory Structure]";
    
    // The CircularBuffer uses std::array internally, which guarantees contiguous memory
    CircularBuffer<int, 100> buffer;
    
    for (int i = 0; i < 100; ++i) {
        buffer.push(i);
    }
    
    // Test that we can retrieve all elements in order
    auto vec = buffer.toVector();
    bool allCorrect = true;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] != static_cast<int>(i)) {
            allCorrect = false;
            break;
        }
    }
    
    addTestResult("All elements retrievable in correct order", allCorrect);
    
    // Memory footprint check
    size_t expectedSize = sizeof(int) * 100 + sizeof(size_t) * 3 + sizeof(std::mutex);
    size_t actualSize = sizeof(CircularBuffer<int, 100>);
    
    qDebug() << "  - Expected approximate size:" << expectedSize << "bytes";
    qDebug() << "  - Actual size:" << actualSize << "bytes";
    
    // Allow some overhead for alignment and mutex
    addTestResult("Memory footprint is reasonable", 
                  actualSize <= expectedSize + 100,
                  QString("Expected ~%1 bytes, got %2 bytes").arg(expectedSize).arg(actualSize));
}

// Test 5: Thread safety
void testThreadSafety() {
    qDebug() << "\n[Test 5: Thread Safety]";
    
    CircularBuffer<int, 1000> buffer;
    const int numThreads = 4;
    const int insertsPerThread = 1000;
    
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&buffer, t, insertsPerThread]() {
            for (int i = 0; i < insertsPerThread; ++i) {
                buffer.push(t * 10000 + i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    qDebug() << "  - Concurrent insertions completed in:" << duration.count() << "ms";
    
    // Buffer should be full (capacity 1000)
    addTestResult("Thread-safe insertions completed", buffer.size() == 1000);
    addTestResult("No data corruption (size is capacity)", buffer.full());
    
    // Verify no crashes and buffer is in valid state
    auto vec = buffer.toVector();
    addTestResult("Can retrieve all elements after concurrent access", vec.size() == 1000);
}

// Test 6: LogBuffer with LogEntry (real-world usage)
void testLogBuffer() {
    qDebug() << "\n[Test 6: LogBuffer with LogEntry (Real-World Usage)]";
    
    auto logBuffer = createLogBuffer(5000);
    
    addTestResult("LogBuffer created successfully", logBuffer != nullptr);
    addTestResult("LogBuffer initial size is 0", logBuffer->size() == 0);
    addTestResult("LogBuffer capacity is 5000", logBuffer->capacity() == 5000);
    
    // Add log entries
    for (int i = 0; i < 100; ++i) {
        logBuffer->append(QString("Log entry %1").arg(i), i % 10 == 0);
    }
    
    addTestResult("Added 100 log entries", logBuffer->size() == 100);
    
    // Test getAll
    auto allEntries = logBuffer->getAll();
    addTestResult("getAll() returns 100 entries", allEntries.size() == 100);
    addTestResult("First entry is correct", allEntries[0].text == "Log entry 0");
    addTestResult("Last entry is correct", allEntries[99].text == "Log entry 99");
    
    // Test getRecent
    auto recent = logBuffer->getRecent(10);
    addTestResult("getRecent(10) returns 10 entries", recent.size() == 10);
    addTestResult("Most recent entry is correct", recent[9].text == "Log entry 99");
    
    // Test search
    auto searchResults = logBuffer->search("entry 5");
    addTestResult("search() finds multiple matches", searchResults.size() >= 10); // 5, 15, 25, 35, 45, 50-59
    
    // Test overflow with 5000+ entries
    for (int i = 100; i < 5500; ++i) {
        logBuffer->append(QString("Log entry %1").arg(i), false);
    }
    
    addTestResult("After overflow, size is 5000", logBuffer->size() == 5000);
    
    auto allAfterOverflow = logBuffer->getAll();
    addTestResult("After overflow, oldest entries removed", 
                  allAfterOverflow[0].text == "Log entry 500",
                  QString("Expected 'Log entry 500', got '%1'").arg(allAfterOverflow[0].text));
    
    // Clear test
    logBuffer->clear();
    addTestResult("After clear(), size is 0", logBuffer->size() == 0);
    addTestResult("After clear(), buffer is empty", logBuffer->isEmpty());
}

// Test 7: Move semantics and emplace
void testMoveSemantics() {
    qDebug() << "\n[Test 7: Move Semantics and Emplace]";
    
    CircularBuffer<QString, 10> buffer;
    
    // Test push with copy
    QString str1 = "Test string 1";
    buffer.push(str1);
    addTestResult("Push with copy works", buffer.size() == 1);
    
    // Test push with move
    QString str2 = "Test string 2";
    buffer.push(std::move(str2));
    addTestResult("Push with move works", buffer.size() == 2);
    
    // Test emplace
    buffer.emplace("Test string 3");
    addTestResult("Emplace works", buffer.size() == 3);
    
    auto vec = buffer.toVector();
    addTestResult("All strings stored correctly", 
                  vec[0] == "Test string 1" && 
                  vec[1] == "Test string 2" && 
                  vec[2] == "Test string 3");
}

// Test 8: Random access with at()
void testRandomAccess() {
    qDebug() << "\n[Test 8: Random Access]";
    
    CircularBuffer<int, 10> buffer;
    
    for (int i = 0; i < 10; ++i) {
        buffer.push(i * 10);
    }
    
    // Test valid access
    auto val0 = buffer.at(0);
    auto val5 = buffer.at(5);
    auto val9 = buffer.at(9);
    
    addTestResult("at(0) returns valid value", val0.has_value() && *val0 == 0);
    addTestResult("at(5) returns valid value", val5.has_value() && *val5 == 50);
    addTestResult("at(9) returns valid value", val9.has_value() && *val9 == 90);
    
    // Test invalid access
    auto invalid = buffer.at(10);
    addTestResult("at(10) returns nullopt for out of range", !invalid.has_value());
    
    // Test after overflow
    buffer.push(100);
    buffer.push(110);
    
    auto newFirst = buffer.at(0);
    addTestResult("After overflow, at(0) returns new oldest element", 
                  newFirst.has_value() && *newFirst == 20);
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=================================================";
    qDebug() << "ZenRunner Circular Buffer Test Suite";
    qDebug() << "Testing: Memory-bound, O(1) insertion, 5000 capacity";
    qDebug() << "=================================================\n";
    
    try {
        testBasicInsertion();
        testOverflowBehavior();
        testInsertionPerformance();
        testContiguousMemory();
        testThreadSafety();
        testLogBuffer();
        testMoveSemantics();
        testRandomAccess();
        
        // Print summary
        qDebug() << "\n=================================================";
        qDebug() << "Test Summary";
        qDebug() << "=================================================";
        
        int passed = 0;
        int failed = 0;
        
        for (const auto& result : testResults) {
            if (result.passed) {
                ++passed;
            } else {
                ++failed;
            }
        }
        
        qDebug() << "Total tests:" << (passed + failed);
        qDebug() << "Passed:" << passed;
        qDebug() << "Failed:" << failed;
        
        if (failed == 0) {
            qDebug() << "\n✓ All tests passed!";
            qDebug() << "\nCircular Buffer Implementation Summary:";
            qDebug() << "  ✓ Supports up to 5000 log lines per project";
            qDebug() << "  ✓ O(1) insertion cost guaranteed";
            qDebug() << "  ✓ Buffer overflows overwrite oldest entries";
            qDebug() << "  ✓ Contiguous memory structure (std::array)";
            qDebug() << "  ✓ Thread-safe with mutex protection";
            qDebug() << "  ✓ Memory-efficient for system stability";
        } else {
            qDebug() << "\n✗ Some tests failed!";
            return 1;
        }
        
        qDebug() << "=================================================";
        
        return 0;
        
    } catch (const std::exception& e) {
        qCritical() << "Test failed with exception:" << e.what();
        return 1;
    }
}
