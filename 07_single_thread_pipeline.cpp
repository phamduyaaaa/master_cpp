/**
 * @file 07_single_thread_pipeline.cpp
 * @brief Practice building a simple camera pipeline using a bounded frame queue.
 *
 * Description:
 * This program simulates a simplified robotics vision pipeline consisting of
 * multiple cameras producing image frames and a bounded queue buffering the
 * captured frames before processing.
 *
 * Each FrameCamera continuously generates Frame objects, while a single
 * FrameQueue stores incoming frames up to a fixed capacity. When the queue
 * becomes full, additional frames are dropped. After all frames are captured,
 * the pipeline removes and processes every remaining frame from the queue.
 *
 * At the end of execution, the program reports:
 *   - Total captured frames
 *   - Total processed frames
 *   - Total dropped frames
 *
 * Topics covered:
 * - Class composition
 * - std::vector
 * - std::queue
 * - Object lifetime
 * - Constructor and destructor
 * - Encapsulation
 * - Producer-consumer workflow
 * - Bounded queue design
 * - Queue overflow handling
 * - Basic pipeline statistics
 * - emplace_back()
 * - Const member functions
 *
 * Learning objectives:
 * - Understand the basic architecture of a camera pipeline.
 * - Learn how multiple producers generate data for a shared buffer.
 * - Practice implementing a bounded queue with push/pop operations.
 * - Observe how queue capacity affects frame loss.
 * - Prepare for multithreaded camera pipelines using std::thread,
 *   mutexes, and condition variables.
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

int main() {
    int num_cameras = 3;
    int frames_per_camera = 5;
    runPipeline(num_cameras, frames_per_camera);


    return 0;
}
