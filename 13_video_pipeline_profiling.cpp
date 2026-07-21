/**
 * @file 13_video_pipeline_profiling.cpp
 * @brief Multi-threaded video capture, processing, and display pipeline with real-time profiling.
 *
 * This example demonstrates a thread-safe multi-stage pipeline using the Producer-Consumer pattern
 * for video processing using OpenCV and C++11 concurrency primitives.
 *
 * Features:
 * - Thread-safe bounded blocking queue supporting graceful shutdown.
 * - Decoupled capture, processing, and rendering threads.
 * - Detailed timing/profiling per frame (Capture, Queue latency, Processing, and Total Latency).
 *
 * Synchronization Primitives:
 * - std::mutex & std::lock_guard / std::unique_lock
 * - std::condition_variable
 * - std::atomic
 *
 * Learning Objectives:
 * - Build scalable image/video processing pipelines.
 * - Measure latency and throughput across concurrent stages.
 * - Implement robust thread shutdown mechanisms.
 *
 * Author: Duy
 * Date: 2026-07-21
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>

/**
 * @brief A thread-safe bounded blocking queue implementation.
 * 
 * Provides thread-safe push and pop operations with maximum capacity bounds.
 * Producers block on full queue; consumers block on empty queue.
 *
 * @tparam T Type of elements stored in the queue.
 */
template <typename T>
class ThreadSafeQueue {
   public:
    /**
     * @brief Constructs a new ThreadSafeQueue object.
     * @param max_size Maximum number of elements allowed in the queue.
     */
    explicit ThreadSafeQueue(size_t max_size) : max_size_(max_size), shutdown_(false) {}

    /**
     * @brief Pushes an item into the queue. Blocks if queue is full.
     * @param item Const reference to the item to be pushed.
     */
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_cv_.wait(lock, [this] { return queue_.size() < max_size_ || shutdown_; });
        if (shutdown_) return;
        queue_.push(item);
        not_empty_cv_.notify_one();
    }

    /**
     * @brief Pops an item from the queue. Blocks if queue is empty.
     * @param out Reference to hold the popped item.
     * @return true if an item was successfully popped, false if queue is shut down and empty.
     */
    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        if (queue_.empty()) return false;
        out = queue_.front();
        queue_.pop();
        not_full_cv_.notify_one();
        return true;
    }

    /**
     * @brief Triggers queue shutdown and unblocks all waiting threads.
     */
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        not_full_cv_.notify_all();
        not_empty_cv_.notify_all();
    }

    /**
     * @brief Gets the current size of the queue.
     * @return Current size of the queue.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

   private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    size_t max_size_;
    bool shutdown_;
};

/**
 * @brief Container structure holding image frame along with profiling timestamps.
 */
struct FrameData {
    cv::Mat frame;
    int frame_id;
    std::chrono::time_point<std::chrono::steady_clock> capture_start;
    std::chrono::time_point<std::chrono::steady_clock> capture_end;
    std::chrono::time_point<std::chrono::steady_clock> process_start;
    std::chrono::time_point<std::chrono::steady_clock> process_end;
};

/**
 * @brief Worker function to capture frames from camera device.
 * 
 * @param cap OpenCV VideoCapture instance.
 * @param out_queue Output thread-safe queue to pass captured frames.
 * @param running Atomic flag controlling worker lifecycle.
 */
void capture_worker(cv::VideoCapture& cap, ThreadSafeQueue<FrameData>& out_queue,
                    std::atomic<bool>& running) {
    int frame_counter = 0;
    while (running) {
        FrameData data;
        data.frame_id = frame_counter++;
        data.capture_start = std::chrono::steady_clock::now();

        if (!cap.read(data.frame)) {
            running = false;
            break;
        }

        data.capture_end = std::chrono::steady_clock::now();
        out_queue.push(data);  // Blocks if processing thread falls behind
    }
    out_queue.shutdown();
}

/**
 * @brief Worker function to execute image processing tasks.
 * 
 * @param in_queue Input queue containing captured frames.
 * @param out_queue Output queue to push processed frames.
 * @param running Atomic flag controlling worker lifecycle.
 */
void processing_worker(ThreadSafeQueue<FrameData>& in_queue, ThreadSafeQueue<FrameData>& out_queue,
                       std::atomic<bool>& running) {
    FrameData data;
    while (in_queue.pop(data)) {
        data.process_start = std::chrono::steady_clock::now();

        // Optional image processing operations:
        // data.frame.convertTo(data.frame, -1, 1.5, 20);
        // cv::cvtColor(data.frame, data.frame, cv::COLOR_BGR2GRAY);
        // cv::cvtColor(data.frame, data.frame, cv::COLOR_GRAY2BGR);
        // cv::GaussianBlur(data.frame, data.frame, cv::Size(31, 31), 0);

        data.process_end = std::chrono::steady_clock::now();
        out_queue.push(data);
    }
    out_queue.shutdown();
}

int main() {
    cv::VideoCapture cap(2, cv::CAP_V4L2);
    // Camera configuration options:
    // cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1.0);
    // cap.set(cv::CAP_PROP_AUTO_WB, 0.0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera device.\n";
        return 1;
    }

    // Manual Exposure Tuning (Linux/V4L2)
    // bool exp_ret = cap.set(cv::CAP_PROP_EXPOSURE, 1.0);
    // if (!exp_ret) {
    //     cap.set(cv::CAP_PROP_EXPOSURE, 330.0);
    // }

    // Image adjustment settings
    // cap.set(cv::CAP_PROP_BRIGHTNESS, 30.0);
    // cap.set(cv::CAP_PROP_CONTRAST, 60.0);
    // cap.set(cv::CAP_PROP_SATURATION, 70.0);

    // Output camera hardware parameters
    std::cout << "--- Hardware Camera Info ---\n";
    std::cout << "Hardware Camera FPS: " << cap.get(cv::CAP_PROP_FPS) << "\n";
    std::cout << "Frame Width:  " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "\n";
    std::cout << "Frame Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << "\n";
    std::cout << "--------------------------------------------------\n";

    ThreadSafeQueue<FrameData> capture_queue(5);
    ThreadSafeQueue<FrameData> display_queue(5);
    std::atomic<bool> running(true);

    std::thread t_capture(capture_worker, std::ref(cap), std::ref(capture_queue),
                          std::ref(running));
    std::thread t_process(processing_worker, std::ref(capture_queue), std::ref(display_queue),
                          std::ref(running));

    int frame_count = 0;
    auto fps_timer_start = std::chrono::steady_clock::now();
    FrameData data;

    while (display_queue.pop(data)) {
        auto display_time = std::chrono::steady_clock::now();

        // Calculate latency metrics
        double cap_time_ms =
            std::chrono::duration<double, std::milli>(data.capture_end - data.capture_start)
                .count();
        double queue_wait_ms =
            std::chrono::duration<double, std::milli>(data.process_start - data.capture_end)
                .count();
        double proc_time_ms =
            std::chrono::duration<double, std::milli>(data.process_end - data.process_start)
                .count();
        double total_latency_ms =
            std::chrono::duration<double, std::milli>(display_time - data.capture_start).count();

        cv::imshow("Camera Feed", data.frame);

        frame_count++;
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(current_time - fps_timer_start)
                .count();

        if (elapsed_ms >= 1000) {
            double fps = frame_count * 1000.0 / elapsed_ms;

            // Output profiling metrics
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "[FPS: " << fps << "] "
                      << "| Cap: " << cap_time_ms << " ms "
                      << "| Wait in Q: " << queue_wait_ms << " ms "
                      << "| Proc: " << proc_time_ms << " ms "
                      << "| Total Latency: " << total_latency_ms << " ms\n";

            frame_count = 0;
            fps_timer_start = std::chrono::steady_clock::now();
        }

        if (cv::waitKey(1) == 'q') {
            running = false;
            capture_queue.shutdown();
            display_queue.shutdown();
            break;
        }
    }

    t_capture.join();
    t_process.join();
    cv::destroyAllWindows();
    std::cout << "Clean exit\n";
    return 0;
}
