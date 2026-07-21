/**
 * @file 12_bounded_blocking_queue.cpp
 * @brief Thread-safe bounded blocking queue using std::mutex and std::condition_variable.
 *
 * This example demonstrates the classic Producer-Consumer pattern.
 *
 * Features:
 * - Thread-safe FIFO queue.
 * - Configurable maximum queue capacity.
 * - Producer blocks when the queue is full.
 * - Consumer blocks when the queue is empty.
 * - Graceful shutdown that wakes all waiting threads.
 *
 * Synchronization primitives:
 * - std::mutex
 * - std::unique_lock
 * - std::condition_variable
 *
 * Learning objectives:
 * - Understand blocking synchronization.
 * - Learn how condition_variable coordinates producers and consumers.
 * - Avoid busy waiting (spin loops).
  *
 * Author: Duy
 * Date: 2026-07-21
 */

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue(size_t max_size)
        : max_size_(max_size), shutdown_(false) {}

    // Blocks until space becomes available.
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_cv_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
        });

        if (shutdown_)
            return;

        queue_.push(item);

        // Wake one waiting consumer.
        not_empty_cv_.notify_one();
    }

    // Blocks until an item is available.
    // Returns false when shutdown is requested
    // and the queue becomes empty.
    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_cv_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
        });

        if (queue_.empty())
            return false;

        out = queue_.front();
        queue_.pop();

        // Wake one producer because
        // one slot has become available.
        not_full_cv_.notify_one();

        return true;
    }

    // Stop the queue and wake all waiting threads.
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            shutdown_ = true;
        }

        not_full_cv_.notify_all();
        not_empty_cv_.notify_all();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;

    mutable std::mutex mutex_;

    // Used by producers waiting for free space.
    std::condition_variable not_full_cv_;

    // Used by consumers waiting for available data.
    std::condition_variable not_empty_cv_;

    size_t max_size_;
    bool shutdown_;
};

int main() {
    ThreadSafeQueue<int> q(3);

    // Producer thread
    std::thread producer([&q] {
        for (int i = 1; i <= 6; ++i) {
            q.push(i);
            std::cout << "Producer pushed: " << i << '\n';
        }

        q.shutdown();
        std::cout << "Producer: shutdown requested\n";
    });

    // Consumer thread (intentionally slower)
    std::thread consumer([&q] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        int value;

        while (q.pop(value)) {
            std::cout << "Consumer popped: " << value << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        std::cout << "Consumer: exit\n";
    });

    producer.join();
    consumer.join();

    std::cout << "Main: all threads finished\n";

    return 0;
}
