/**
 * @file 05_frame_camera_class.cpp
 * @brief Practice object-oriented programming (OOP) in C++ using a camera simulation.
 *
 * Description:
 * This program models a simple camera device that continuously captures frames.
 * Each captured frame contains:
 *   - Camera ID
 *   - Frame number
 *   - Timestamp
 *
 * The FrameCamera class is responsible for maintaining the camera's internal
 * state and generating new Frame objects.
 *
 * Topics covered:
 * - Class definition and implementation
 * - Constructor and destructor
 * - Initializer list
 * - Encapsulation (private data members)
 * - Member functions
 * - Const member functions
 * - Object lifetime and scope
 * - Returning objects by value
 * - Using helper functions to construct objects
 *
 * Learning objectives:
 * - Understand how a class manages its own state.
 * - Learn when constructors and destructors are invoked.
 * - Practice encapsulating data behind public interfaces.
 * - Observe RAII by letting the destructor release resources automatically.
 * - Prepare for more advanced topics such as multithreading and camera pipelines.
 *
 * Author: Duy
 * Date: 2026-07-19
 */

#include <iostream>
#include <vector>
#include <iomanip>

struct Frame {
    int camera_id;
    int frame_number;
    double timestamp;
};

void printFrame(const Frame& f);

void advanceFrame(Frame& f);

Frame makeFrame(int camId, int frameNum, double t);


void printFrame(const Frame& f) {
    std::cout << "[Cam " << f.camera_id << "] Frame #" << f.frame_number
              << " @ t=" << std::fixed << std::setprecision(2) << f.timestamp << "s\n";
}

void advanceFrame(Frame& f){
    f.frame_number ++;
}

Frame makeFrame(int camID, int frameNum, double t){
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

FrameCamera::FrameCamera(int camera_id) : camera_id_(camera_id) {}
FrameCamera::~FrameCamera() {
    std::cout << "Camera "<< camera_id_ << " closed. " << "Sum: " << frame_counter_ << " frame\n";
}

Frame FrameCamera::captureNext(){
    frame_counter_ ++;
    current_time_ += 0.033;
    Frame x = makeFrame(camera_id_, frame_counter_, current_time_);
    return x;
}

int FrameCamera::getTotalCaptured() const{
    return frame_counter_;
}


int main(){
    {
        FrameCamera cam(1);
        for (int i = 0; i < 5; i++){
            Frame f = cam.captureNext();
            printFrame(f);
        }
    }
    std::cout << "Out of scope\n";

    return 0;
}
