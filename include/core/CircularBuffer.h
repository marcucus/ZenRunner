#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <optional>
#include <concepts>

namespace ZenRunner {

/**
 * @brief Thread-safe circular buffer with O(1) insertion and retrieval
 * 
 * This implementation uses C++20 concepts and is optimized for performance
 * with contiguous memory layout for cache efficiency.
 * 
 * @tparam T The type of elements stored in the buffer
 * @tparam Capacity Maximum number of elements in the buffer
 */
template<typename T, std::size_t Capacity>
requires std::is_default_constructible_v<T> && (Capacity > 0)
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0), size_(0) {}

    /**
     * @brief Add an element to the buffer
     * 
     * If the buffer is full, the oldest element is overwritten.
     * This operation is O(1) and thread-safe.
     * 
     * @param item The item to add
     */
    void push(const T& item) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) [[likely]] {
            ++size_;
        } else [[unlikely]] {
            // Buffer is full, move head forward (overwrite oldest)
            head_ = (head_ + 1) % Capacity;
        }
    }

    /**
     * @brief Add an element to the buffer (move semantics)
     */
    void push(T&& item) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) [[likely]] {
            ++size_;
        } else [[unlikely]] {
            head_ = (head_ + 1) % Capacity;
        }
    }

    /**
     * @brief Emplace an element directly in the buffer
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        
        buffer_[tail_] = T(std::forward<Args>(args)...);
        tail_ = (tail_ + 1) % Capacity;
        
        if (size_ < Capacity) [[likely]] {
            ++size_;
        } else [[unlikely]] {
            head_ = (head_ + 1) % Capacity;
        }
    }

    /**
     * @brief Get the element at the specified index
     * 
     * @param index Index from the head (0 = oldest element)
     * @return The element if valid index, nullopt otherwise
     */
    [[nodiscard]] std::optional<T> at(std::size_t index) const {
        std::lock_guard lock(mutex_);
        
        if (index >= size_) [[unlikely]] {
            return std::nullopt;
        }
        
        const std::size_t actual_index = (head_ + index) % Capacity;
        return buffer_[actual_index];
    }

    /**
     * @brief Get all elements in chronological order
     * 
     * @return Vector containing all elements from oldest to newest
     */
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

    /**
     * @brief Get the last N elements
     * 
     * @param n Number of elements to retrieve
     * @return Vector containing the last n elements (or all if n > size)
     */
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

    /**
     * @brief Clear all elements from the buffer
     */
    void clear() {
        std::lock_guard lock(mutex_);
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    /**
     * @brief Get the current number of elements in the buffer
     */
    [[nodiscard]] std::size_t size() const noexcept {
        return size_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Check if the buffer is empty
     */
    [[nodiscard]] bool empty() const noexcept {
        return size_.load(std::memory_order_relaxed) == 0;
    }

    /**
     * @brief Check if the buffer is full
     */
    [[nodiscard]] bool full() const noexcept {
        return size_.load(std::memory_order_relaxed) == Capacity;
    }

    /**
     * @brief Get the maximum capacity of the buffer
     */
    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    /**
     * @brief Apply a function to each element in the buffer
     * 
     * The function is applied in chronological order (oldest to newest).
     * 
     * @param func Function to apply to each element
     */
    template<typename Func>
    requires std::invocable<Func, const T&>
    void forEach(Func&& func) const {
        std::lock_guard lock(mutex_);
        
        for (std::size_t i = 0; i < size_; ++i) {
            const std::size_t index = (head_ + i) % Capacity;
            func(buffer_[index]);
        }
    }

private:
    std::array<T, Capacity> buffer_;
    std::size_t head_;
    std::size_t tail_;
    std::atomic<std::size_t> size_;
    mutable std::mutex mutex_;
};

} // namespace ZenRunner
