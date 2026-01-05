#include <iostream>
#include <array>
#include <vector>
#include <mutex>
#include <optional>
#include <thread>
#include <chrono>
#include <string>
#include <algorithm>
#include <memory>

/**
 * Simplified CircularBuffer test without Qt dependencies
 * This tests the core circular buffer implementation concepts
 */

template<typename T, std::size_t Capacity>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0), size_(0) {}

    void push(const T& item) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) {
            ++size_;
        } else {
            head_ = (head_ + 1) % Capacity;
        }
    }

    void push(T&& item) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) {
            ++size_;
        } else {
            head_ = (head_ + 1) % Capacity;
        }
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = T(std::forward<Args>(args)...);
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) {
            ++size_;
        } else {
            head_ = (head_ + 1) % Capacity;
        }
    }

    [[nodiscard]] std::optional<T> at(std::size_t index) const {
        std::lock_guard lock(mutex_);
        
        if (index >= size_) {
            return std::nullopt;
        }
        
        const std::size_t actual_index = (head_ + index) % Capacity;
        return buffer_[actual_index];
    }

    [[nodiscard]] std::vector<T> toVector() const {
        std::lock_guard lock(mutex_);
        std::vector<T> result;
        result.reserve(size_);
        
        for (std::size_t i = 0; i < size_; ++i) {
            const std::size_t index = (head_ + i) % Capacity;
            result.push_back(buffer_[index]);
        }
        
        return result;
    }

    [[nodiscard]] std::vector<T> lastN(std::size_t n) const {
        std::lock_guard lock(mutex_);
        std::vector<T> result;
        
        const std::size_t count = std::min(n, size_);
        result.reserve(count);
        
        const std::size_t start_offset = size_ - count;
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t index = (head_ + start_offset + i) % Capacity;
            result.push_back(buffer_[index]);
        }
        
        return result;
    }

    void clear() {
        std::lock_guard lock(mutex_);
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        std::lock_guard lock(mutex_);
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept {
        std::lock_guard lock(mutex_);
        return size_ == 0;
    }

    [[nodiscard]] bool full() const noexcept {
        std::lock_guard lock(mutex_);
        return size_ == Capacity;
    }

    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

private:
    std::array<T, Capacity> buffer_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t size_;
    mutable std::mutex mutex_;
};

// Test result tracking
struct TestResult {
    bool passed{false};
    std::string name;
    std::string message;
};

std::vector<TestResult> testResults;

void addTestResult(const std::string& name, bool passed, const std::string& message = "") {
    TestResult result;
    result.name = name;
    result.passed = passed;
    result.message = message;
    testResults.push_back(result);
    
    if (passed) {
        std::cout << "✓ " << name << std::endl;
    } else {
        std::cout << "✗ " << name;
        if (!message.empty()) {
            std::cout << ": " << message;
        }
        std::cout << std::endl;
    }
}

// Test 1: Basic insertion and capacity
void testBasicInsertion() {
    std::cout << "\n[Test 1: Basic Insertion and Capacity]" << std::endl;
    
    CircularBuffer<int, 10> buffer;
    
    addTestResult("Empty buffer size is 0", buffer.size() == 0);
    addTestResult("Empty buffer isEmpty() returns true", buffer.empty());
    addTestResult("Empty buffer full() returns false", !buffer.full());
    addTestResult("Capacity is 10", buffer.capacity() == 10);
    
    for (int i = 0; i < 5; ++i) {
        buffer.push(i);
    }
    
    addTestResult("After 5 insertions, size is 5", buffer.size() == 5);
    addTestResult("Buffer is not empty", !buffer.empty());
    addTestResult("Buffer is not full", !buffer.full());
    
    for (int i = 5; i < 10; ++i) {
        buffer.push(i);
    }
    
    addTestResult("After 10 insertions, size is 10", buffer.size() == 10);
    addTestResult("Full buffer full() returns true", buffer.full());
}

// Test 2: Overflow behavior (oldest entry overwrite)
void testOverflowBehavior() {
    std::cout << "\n[Test 2: Overflow Behavior - Oldest Entry Overwrite]" << std::endl;
    
    CircularBuffer<int, 5> buffer;
    
    for (int i = 0; i < 5; ++i) {
        buffer.push(i);
    }
    
    auto vec = buffer.toVector();
    addTestResult("Filled buffer contains [0,1,2,3,4]", 
                  vec.size() == 5 && vec[0] == 0 && vec[4] == 4);
    
    buffer.push(5);
    buffer.push(6);
    
    vec = buffer.toVector();
    addTestResult("After overflow, size remains 5", vec.size() == 5);
    
    bool orderCorrect = (vec[0] == 2 && vec[1] == 3 && vec[2] == 4 && vec[3] == 5 && vec[4] == 6);
    addTestResult("After overflow, oldest entries removed [2,3,4,5,6]", orderCorrect);
}

// Test 3: O(1) insertion performance
void testInsertionPerformance() {
    std::cout << "\n[Test 3: O(1) Insertion Performance]" << std::endl;
    
    CircularBuffer<int, 5000> buffer;
    
    const int iterations = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        buffer.push(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avgTimePerInsertion = static_cast<double>(duration.count()) / iterations;
    
    std::cout << "  - Total time for " << iterations << " insertions: " 
              << duration.count() << " μs" << std::endl;
    std::cout << "  - Average time per insertion: " 
              << avgTimePerInsertion << " μs" << std::endl;
    
    addTestResult("Average insertion time < 1 μs", avgTimePerInsertion < 1.0);
    addTestResult("After overflow insertions, size is 5000", buffer.size() == 5000);
    
    auto vec = buffer.toVector();
    addTestResult("Oldest entry is correct after overflow", 
                  vec[0] == 5000 && vec[vec.size()-1] == 9999);
}

// Test 4: Contiguous memory structure
void testContiguousMemory() {
    std::cout << "\n[Test 4: Contiguous Memory Structure]" << std::endl;
    
    CircularBuffer<int, 100> buffer;
    
    for (int i = 0; i < 100; ++i) {
        buffer.push(i);
    }
    
    auto vec = buffer.toVector();
    bool allCorrect = true;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] != static_cast<int>(i)) {
            allCorrect = false;
            break;
        }
    }
    
    addTestResult("All elements retrievable in correct order", allCorrect);
    
    size_t actualSize = sizeof(CircularBuffer<int, 100>);
    
    std::cout << "  - Buffer memory size: " << actualSize << " bytes" << std::endl;
    std::cout << "  - Expected for 100 ints: ~" << (100 * sizeof(int)) << " bytes + overhead" << std::endl;
    
    addTestResult("Memory footprint is reasonable (< 1KB)", actualSize < 1024);
}

// Test 5: Thread safety
void testThreadSafety() {
    std::cout << "\n[Test 5: Thread Safety]" << std::endl;
    
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
    
    std::cout << "  - Concurrent insertions completed in: " 
              << duration.count() << " ms" << std::endl;
    
    addTestResult("Thread-safe insertions completed", buffer.size() == 1000);
    addTestResult("No data corruption (size is capacity)", buffer.full());
    
    auto vec = buffer.toVector();
    addTestResult("Can retrieve all elements after concurrent access", vec.size() == 1000);
}

// Test 6: 5000 capacity requirement
void test5000Capacity() {
    std::cout << "\n[Test 6: 5000 Log Lines Capacity Requirement]" << std::endl;
    
    CircularBuffer<std::string, 5000> logBuffer;
    
    addTestResult("Buffer capacity is exactly 5000", logBuffer.capacity() == 5000);
    
    // Add 5000 log entries
    for (int i = 0; i < 5000; ++i) {
        logBuffer.emplace("Log entry " + std::to_string(i));
    }
    
    addTestResult("Buffer holds exactly 5000 entries", logBuffer.size() == 5000);
    addTestResult("Buffer is full after 5000 insertions", logBuffer.full());
    
    // Add more entries to test overflow
    for (int i = 5000; i < 5100; ++i) {
        logBuffer.emplace("Log entry " + std::to_string(i));
    }
    
    addTestResult("After overflow, size remains 5000", logBuffer.size() == 5000);
    
    auto vec = logBuffer.toVector();
    addTestResult("Oldest 100 entries were overwritten", 
                  vec[0] == "Log entry 100" && vec[vec.size()-1] == "Log entry 5099");
    
    // Calculate approximate memory usage
    size_t bufferSize = sizeof(CircularBuffer<std::string, 5000>);
    std::cout << "  - CircularBuffer<string, 5000> size: " << bufferSize << " bytes" << std::endl;
    std::cout << "  - Approximate size per project: ~" << (bufferSize / 1024.0) << " KB" << std::endl;
}

// Test 7: Move semantics and emplace
void testMoveSemantics() {
    std::cout << "\n[Test 7: Move Semantics and Emplace]" << std::endl;
    
    CircularBuffer<std::string, 10> buffer;
    
    std::string str1 = "Test string 1";
    buffer.push(str1);
    addTestResult("Push with copy works", buffer.size() == 1);
    
    std::string str2 = "Test string 2";
    buffer.push(std::move(str2));
    addTestResult("Push with move works", buffer.size() == 2);
    
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
    std::cout << "\n[Test 8: Random Access]" << std::endl;
    
    CircularBuffer<int, 10> buffer;
    
    for (int i = 0; i < 10; ++i) {
        buffer.push(i * 10);
    }
    
    auto val0 = buffer.at(0);
    auto val5 = buffer.at(5);
    auto val9 = buffer.at(9);
    
    addTestResult("at(0) returns valid value", val0.has_value() && *val0 == 0);
    addTestResult("at(5) returns valid value", val5.has_value() && *val5 == 50);
    addTestResult("at(9) returns valid value", val9.has_value() && *val9 == 90);
    
    auto invalid = buffer.at(10);
    addTestResult("at(10) returns nullopt for out of range", !invalid.has_value());
    
    buffer.push(100);
    buffer.push(110);
    
    auto newFirst = buffer.at(0);
    addTestResult("After overflow, at(0) returns new oldest element", 
                  newFirst.has_value() && *newFirst == 20);
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "ZenRunner Circular Buffer Test Suite" << std::endl;
    std::cout << "Testing: Memory-bound, O(1) insertion, 5000 capacity" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    try {
        testBasicInsertion();
        testOverflowBehavior();
        testInsertionPerformance();
        testContiguousMemory();
        testThreadSafety();
        test5000Capacity();
        testMoveSemantics();
        testRandomAccess();
        
        std::cout << "\n=================================================" << std::endl;
        std::cout << "Test Summary" << std::endl;
        std::cout << "=================================================" << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        for (const auto& result : testResults) {
            if (result.passed) {
                ++passed;
            } else {
                ++failed;
            }
        }
        
        std::cout << "Total tests: " << (passed + failed) << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        
        if (failed == 0) {
            std::cout << "\n✓ All tests passed!" << std::endl;
            std::cout << "\nCircular Buffer Implementation Summary:" << std::endl;
            std::cout << "  ✓ Supports up to 5000 log lines per project" << std::endl;
            std::cout << "  ✓ O(1) insertion cost guaranteed" << std::endl;
            std::cout << "  ✓ Buffer overflows overwrite oldest entries" << std::endl;
            std::cout << "  ✓ Contiguous memory structure (std::array)" << std::endl;
            std::cout << "  ✓ Thread-safe with mutex protection" << std::endl;
            std::cout << "  ✓ Memory-efficient for system stability" << std::endl;
        } else {
            std::cout << "\n✗ Some tests failed!" << std::endl;
            return 1;
        }
        
        std::cout << "=================================================" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
