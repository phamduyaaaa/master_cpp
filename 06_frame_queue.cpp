/**
 * @file 06_frame_queue_class.cpp
 * @brief Practice implementing a bounded frame queue for a camera pipeline.
 *
 * Description:
 * This program simulates a simple producer-consumer workflow in a robotics
 * vision system. A FrameCamera continuously generates Frame objects, while
 * a FrameQueue stores captured frames with a fixed maximum capacity.
 *
 * When the queue reaches its capacity, newly captured frames are rejected.
 * After frames are removed from the queue, new frames can be inserted again.
 *
 * Topics covered:
 * - std::queue
 * - Queue abstraction
 * - Class composition
 * - Constructor and destructor
 * - Encapsulation
 * - Queue capacity management
 * - Push and pop operations
 * - Returning success/failure using bool
 * - Const member functions
 * - Basic producer-consumer architecture
 *
 * Learning objectives:
 * - Understand how a bounded queue works.
 * - Learn to design a reusable queue abstraction.
 * - Practice managing queue capacity safely.
 * - Prepare for multithreaded producer-consumer systems.
 * - Build the foundation for camera pipelines in robotics and computer vision.
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

int main() {
    FrameCamera cam(2);
    FrameQueue q(3);

    // Push five frames.
    // The last two should be rejected because the queue is full.
    for (int i = 0; i < 5; i++) {
        Frame f = cam.captureNext();
        bool ok = q.tryPush(f);

        std::cout << "Push Frame #" << f.frame_number
                  << (ok ? " -> SUCCESS" : " -> REJECTED")
                  << '\n';
    }

    std::cout << "Current queue size: "
              << q.size() << "\n\n";

    // Pop two frames from the queue.
    Frame out;

    for (int i = 0; i < 2; i++) {
        if (q.tryPop(out)) {
            std::cout << "Pop: ";
            printFrame(out);
        }
    }

    std::cout << "\nQueue size after pop: "
              << q.size() << "\n\n";

    // Push two more frames.
    // These should succeed because space is available again.
    for (int i = 0; i < 2; i++) {
        Frame f = cam.captureNext();
        bool ok = q.tryPush(f);

        std::cout << "Push Frame #" << f.frame_number
                  << (ok ? " -> SUCCESS" : " -> REJECTED")
                  << '\n';
    }

    return 0;
}
