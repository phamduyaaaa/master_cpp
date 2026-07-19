/**
 * @file 08_simple_queue_template.cpp
 * @brief Practice implementing a generic bounded queue using C++ templates.
 *
 * Description:
 * This program generalizes the previous FrameQueue class into a reusable
 * SimpleQueue<T> template that supports any data type.
 *
 * The queue follows the FIFO (First-In, First-Out) principle and enforces
 * a fixed maximum capacity. When the queue becomes full, new items are
 * rejected instead of expanding dynamically.
 *
 * The implementation is demonstrated with two different data types:
 *   - int
 *   - Frame
 *
 * This illustrates one of the primary advantages of C++ templates:
 * writing a data structure once and reusing it for many types without
 * duplicating code.
 *
 * Topics covered:
 * - Class templates
 * - Template member function implementation
 * - Generic programming
 * - std::queue
 * - FIFO data structure
 * - Bounded queue design
 * - Function templates vs class templates
 * - Object-oriented programming
 * - Encapsulation
 * - Const member functions
 * - Reusing data structures for multiple types
 *
 * Learning objectives:
 * - Understand why templates eliminate code duplication.
 * - Learn how to implement a generic container class.
 * - Practice defining template member functions.
 * - Compare a type-specific class (FrameQueue) with a generic class
 *   (SimpleQueue<T>).
 * - Prepare for implementing reusable components such as thread-safe
 *   queues, ring buffers, and memory pools.
 *
 * Author: Duy
 * Date: 2026-07-19
 */


#include <iostream>
#include <vector>
#include <iomanip>
#include <queue>

struct Frame {
    int camera_id;
    int frame_number;
    double timestamp;
};

void printFrame(const Frame& f);

void advanceFrame(Frame& f);

Frame makeFrame(int camId, int frameNum, double t);

void printFrame(const Frame& f) {
    std::cout << "[Camera " << f.camera_id << "] Frame #" << f.frame_number
              << " @ t=" << std::fixed << std::setprecision(2)
              << f.timestamp << " s\n";
}

void advanceFrame(Frame& f) {
    f.frame_number++;
}

Frame makeFrame(int camID, int frameNum, double t) {
    Frame f;
    f.camera_id = camID;
    f.frame_number = frameNum;
    f.timestamp = t;

    return f;
}

class FrameCamera {
public:
    FrameCamera(int camera_id);
    ~FrameCamera();

    Frame captureNext();

    int getTotalCaptured() const;

private:
    int camera_id_ = 0;
    int frame_counter_ = 0;
    double current_time_ = 0.0;
};

FrameCamera::FrameCamera(int camera_id)
    : camera_id_(camera_id) {}

FrameCamera::~FrameCamera() {
    std::cout << "Camera " << camera_id_
              << " closed. Total captured: "
              << frame_counter_ << " frame(s).\n";
}

Frame FrameCamera::captureNext() {
    frame_counter_++;
    current_time_ += 0.033;

    Frame x = makeFrame(camera_id_, frame_counter_, current_time_);
    return x;
}

int FrameCamera::getTotalCaptured() const {
    return frame_counter_;
}

class FrameQueue {
public:
    FrameQueue(size_t max_size);
    ~FrameQueue();

    bool tryPush(const Frame& f);

    bool tryPop(Frame& out);

    size_t size() const;

    bool isFull() const;

    bool isEmpty() const;

private:
    std::queue<Frame> queue_;
    size_t max_size_;
};

FrameQueue::FrameQueue(size_t max_size)
    : max_size_(max_size) {}

FrameQueue::~FrameQueue() {
    std::cout << "FrameQueue closed. Maximum capacity: "
              << max_size_ << '\n';
}

bool FrameQueue::tryPush(const Frame& f) {
    if (!isFull()) {
        queue_.push(f);
        std::cout<< "[FrameQueue] Pushed Frame #" << f.frame_number
                  << " from Camera " << f.camera_id << '\n';
        return true;
    } else {
        std::cout << "[FrameQueue] Queue is full. Dropping Frame #"
                  << f.frame_number << '\n';
        return false;
    }
}

bool FrameQueue::tryPop(Frame& out) {
    if (!isEmpty()) {
        out = queue_.front();
        queue_.pop();
        return true;
    } else {
        std::cout << "[FrameQueue] Queue is empty.\n";
        return false;
    }
}

size_t FrameQueue::size() const {
    return queue_.size();
}

bool FrameQueue::isFull() const {
    return queue_.size() >= max_size_;
}

bool FrameQueue::isEmpty() const {
    return queue_.empty();
}

void runPipeline(int num_cameras, int frames_per_camera) {
    std::vector<FrameCamera> cameras;
    FrameQueue capture_queue(10);

    for (int i = 0; i < num_cameras; ++i) {
        cameras.emplace_back(i);
    }

    int total_dropped = 0;
    int total_processed = 0;
    
    std::cout<< "[Pipeline] tryPush()\n";
    for (int j = 0; j < frames_per_camera; ++j) {
        for (int i = 0; i < num_cameras; ++i) {
            Frame f = cameras[i].captureNext();
            bool ok = capture_queue.tryPush(f);
            if (!ok) total_dropped++;
        }
    }


    std::cout<< "[Pipeline] tryPop()\n";
    Frame f;
    while (!capture_queue.isEmpty()) {
            if (capture_queue.tryPop(f)) {
                printFrame(f);
                total_processed++;
            }
    }

    int total_captured = 0;
    for (const auto& cam : cameras) {
        total_captured += cam.getTotalCaptured();
    }
    std::cout<< "[Pipeline] Statistic:\n";
    std::cout<< "Total captured frames: " << total_captured << '\n';
    std::cout<< "Total frames processed: " << total_processed << '\n';
    std::cout<< "Total dropped frames: " << total_dropped << '\n';
};


template <typename T>
class SimpleQueue {
public:
    explicit SimpleQueue(size_t max_size);

    bool tryPush(const T& item);
    bool tryPop(T& out);
    size_t size() const;
    bool isFull() const;
    bool isEmpty() const;
private:
    std::queue<T> queue_;
    size_t max_size_;
};

template <typename T>
SimpleQueue<T>::SimpleQueue(size_t max_size)
    : max_size_(max_size) {}

template <typename T>
bool SimpleQueue<T>::tryPush(const T& item) {
    if (!isFull()) {
        queue_.push(item);
        return true;
    } else {
        std::cout << "[SimpleQueue] Queue is full. Dropping item.\n";
        return false;
    }
}

template <typename T>
bool SimpleQueue<T>::tryPop(T& out) {
    if (!isEmpty()) {
        out = queue_.front();
        queue_.pop();
        return true;
    } else {
        std::cout << "[SimpleQueue] Queue is empty.\n";
        return false;
    }
}

template <typename T>
bool SimpleQueue<T>::isFull() const {
    return queue_.size() >= max_size_;
}

template <typename T>
size_t SimpleQueue<T>::size() const {
    return queue_.size();
}

template <typename T>
bool SimpleQueue<T>::isEmpty() const {
    return queue_.empty();
}

int main() {
    // Test 1: SimpleQueue<int>
    SimpleQueue<int> qi(3);
    qi.tryPush(10);
    qi.tryPush(20);
    qi.tryPush(30);
    qi.tryPush(40);  // phải bị từ chối

    int val;
    while (!qi.isEmpty()) {
        qi.tryPop(val);
        std::cout << "int pop: " << val << "\n";
    }

    // Test 2: SimpleQueue<Frame>
    SimpleQueue<Frame> qf(3);
    FrameCamera cam(1);
    for (int i = 0; i < 5; i++) {
        Frame f = cam.captureNext();
        qf.tryPush(f);  // 2 frame cuối bị từ chối
    }

    Frame out;
    while (!qf.isEmpty()) {
        qf.tryPop(out);
        printFrame(out);
    }

    return 0;
}
