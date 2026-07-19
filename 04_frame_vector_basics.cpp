/**
 * @file 04_frame_vector_basics.cpp
 * @brief Practice basic C++ concepts using a Frame structure.
 *
 * Topics covered:
 * - Define and initialize a struct
 * - Pass objects by const reference
 * - Modify objects by reference
 * - Return objects from functions
 * - Store objects in std::vector
 * - Add elements with push_back()
 * - Traverse and print vector elements
 *
 * Learning objectives:
 * - Understand object lifetime and value semantics.
 * - Distinguish between pass-by-value and pass-by-reference.
 * - Learn the difference between std::vector::size() and capacity().
 * - Know when to use push_back() instead of operator[].
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

int main(){
    int camID = 0;
    int frameNum = 0;
    double t = 10.0;
    Frame a = makeFrame(camID, frameNum, t);
    for (int i = 0; i < 3; i++){
    advanceFrame(a);
    printFrame(a);
    }

    std::vector<Frame> arr_frame;
    for (int i = 0; i < 5; i++){
        camID ++;
        frameNum ++;
        t ++;
        arr_frame.push_back(makeFrame(camID, frameNum, t));
    }

    for (int i = 0; i < 3; i++){
        advanceFrame(a);
        printFrame(a);
    }

    for (int i = 0; i < 5; i++) {
        printFrame(arr_frame[i]);
    }

    return 0;
}

