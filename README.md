# master_cpp
C++ 17/20 | Concurrency | Parallel | Multithreading

<details>
<summary><strong>Mini Project: Frame Station</strong></summary>

## Giới thiệu

Mục tiêu của mini-project này là xây dựng một **pipeline xử lý ảnh đơn luồng (single-thread)** để hiểu rõ luồng dữ liệu trước khi học `std::thread`, `std::mutex` và `std::condition_variable`.

Pipeline sẽ mô phỏng:

```text
Camera
    │
    ▼
Capture Frame
    │
    ▼
Frame Queue
    │
    ▼
Processing
    │
    ▼
Output
```

Đây chính là logic của:

```
capture_worker()
        ↓
ThreadSafeQueue
        ↓
processing_worker()
```

chỉ khác là mọi thứ đều chạy trên **một thread**, chưa có đồng bộ.

---

# 📚 Danh sách bài tập

- Bài 1 — Struct Frame và Reference cơ bản
- Bài 2 — Class FrameCamera (OOP + RAII)
- Bài 3 — FrameQueue
- Bài 4 — Single Thread Pipeline
- Bài 5 — Template Queue

---

<details>
<summary><strong>📘 Bài 1 — Struct Frame và Reference cơ bản</strong></summary>

### 📄 Tên file

```
04_frame_vector_basics.cpp
```

---

### Đề bài

Cho cấu trúc:

```cpp
struct Frame {
    int camera_id;
    int frame_number;
    double timestamp;
};
```

Viết các hàm:

```cpp
// In:
// [Cam 2] Frame #15 @ t=3.20s
void printFrame(const Frame& f);

// Tăng frame_number lên 1
// Sửa trực tiếp object gốc
void advanceFrame(Frame& f);

// Tạo Frame mới
Frame makeFrame(int camId,
                int frameNum,
                double t);
```

---

### Viết `main()` để test

- Tạo một `Frame` bằng `makeFrame()`.
- Gọi `advanceFrame()` 3 lần.
- In sau mỗi lần gọi.
- Tạo `std::vector<Frame>`.
- Thêm 5 `Frame`.
- In toàn bộ bằng vòng lặp.

---

### 🎯 Mục tiêu học

- Struct
- Pass by value
- Pass by reference
- Const reference
- Return object by value
- `std::vector`
- `push_back()`

Đây là nền tảng để thiết kế API như:

```cpp
push(const T&);
pop(T&);
```

sau này trong `ThreadSafeQueue`.

</details>

---

<details>
<summary><strong>📘 Bài 2 — Class FrameCamera (OOP + RAII)</strong></summary>

### 📄 Tên file

```
05_frame_camera_class.cpp
```

---

### Đề bài

Viết class:

```cpp
class FrameCamera {
public:
    FrameCamera(int camera_id);

    ~FrameCamera();

    Frame captureNext();

    int getTotalCaptured() const;

private:
    int camera_id_;
    int frame_counter_;
    double current_time_;
};
```

---

### Yêu cầu

Constructor:

- lưu `camera_id`
- `frame_counter = 0`
- `current_time = 0`

Destructor:

In:

```
Camera X đã đóng.
Tổng Y frame.
```

`captureNext()`:

Mỗi lần gọi:

- frame_number tăng 1
- timestamp tăng `0.033`
- trả về một `Frame`

---

### Viết `main()` để test

Trong một scope riêng:

```cpp
{
    FrameCamera cam(1);

    ...
}
```

- Capture 5 frame.
- In từng frame.
- Quan sát destructor tự chạy khi ra khỏi scope.

---

### 🎯 Mục tiêu học

- Class
- Constructor
- Destructor
- RAII
- Initializer List
- Encapsulation
- Const Member Function
- Object Lifetime

Hiểu được vì sao:

```cpp
std::lock_guard
```

và

```cpp
std::unique_lock
```

tự unlock khi ra khỏi scope.

</details>

---

<details>
<summary><strong>📘 Bài 3 — FrameQueue (Chuẩn bị cho ThreadSafeQueue)</strong></summary>

### 📄 Tên file

```
06_frame_queue.cpp
```

---

### Đề bài

Viết class:

```cpp
class FrameQueue {
public:
    FrameQueue(size_t max_size);

    bool tryPush(const Frame& f);

    bool tryPop(Frame& out);

    size_t size() const;

    bool isFull() const;

    bool isEmpty() const;

private:
    std::queue<Frame> queue_;
    size_t max_size_;
};
```

---

### Quy tắc

Nếu queue đầy:

```
Queue đầy, bỏ qua frame #X
```

Không được thêm.

Nếu queue rỗng:

```
return false;
```

---

### Viết `main()` để test

- Tạo `FrameQueue q(3)`.
- Dùng `FrameCamera` sinh 5 frame.
- Push từng frame.
- Quan sát:
  - 3 frame đầu thành công.
  - 2 frame cuối bị từ chối.
- Pop 2 frame.
- Push tiếp.
- In `size()` sau mỗi thao tác.

---

### 🎯 Mục tiêu học

- `std::queue`
- `push`
- `pop`
- `front`
- `empty`
- `size`
- Output parameter (`Frame& out`)

Đây là phiên bản đơn luồng của `ThreadSafeQueue`.

Sau này chỉ cần thay:

```cpp
if(full)
    return false;
```

thành:

```cpp
while(full)
    cv.wait(lock);
```

</details>

---

<details>
<summary><strong>📘 Bài 4 — Single Thread Pipeline</strong></summary>

### 📄 Tên file

```
07_single_thread_pipeline.cpp
```

---

### Đề bài

Viết:

```cpp
void runPipeline(
    int num_cameras,
    int frames_per_camera
);
```

---

### Logic

Tạo:

```cpp
std::vector<FrameCamera>
```

Tạo:

```cpp
FrameQueue capture_queue(10);
```

#### Giai đoạn Capture

```
Camera

↓

captureNext()

↓

tryPush()

↓

FrameQueue
```

Lặp đến khi mỗi camera tạo đủ:

```
frames_per_camera
```

---

#### Giai đoạn Processing

```
tryPop()

↓

printFrame()

↓

Đã xử lý
```

Cho đến khi queue rỗng.

---

### In thống kê cuối

- Tổng frame capture
- Tổng frame xử lý
- Tổng frame bị từ chối

---

### 🎯 Mục tiêu học

Hiểu toàn bộ pipeline xử lý ảnh.

Sau này chỉ việc:

```
Capture Loop
```

↓

đưa vào

```
capture_worker()
```

và

```
Processing Loop
```

↓

đưa vào

```
processing_worker()
```

chạy trên hai thread khác nhau.

</details>

---

<details>
<summary><strong>📘 Bài 5 — Template hóa Queue</strong></summary>

### 📄 Tên file

```
08_simple_queue_template.cpp
```

---

### Đề bài

Viết:

```cpp
template <typename T>
class SimpleQueue {
public:
    explicit SimpleQueue(size_t max_size);

    bool tryPush(const T& item);

    bool tryPop(T& out);

    size_t size() const;

private:
    std::queue<T> queue_;
    size_t max_size_;
};
```

---

### Lưu ý

Đây là template.

Toàn bộ implementation phải nằm trong:

```
SimpleQueue.hpp
```

hoặc

```
SimpleQueue.h
```

hoặc:

```
SimpleQueue.h
#include "SimpleQueue.tpp"
```

Không được tách `.cpp`.

---

### Viết `main()` để test

Test với:

```cpp
SimpleQueue<int>
```

và

```cpp
SimpleQueue<Frame>
```

Chứng minh cùng một queue hoạt động với nhiều kiểu dữ liệu.

---

### 🎯 Mục tiêu học

- C++ Template
- Generic Programming
- Header-only Template

Chuẩn bị cho:

```cpp
ThreadSafeQueue<T>
```

</details>

---

# 📖 Từ khóa nên tra cứu

| Bài | Từ khóa |
|------|----------|
| 1 | pass by value, pass by reference, const reference |
| 2 | constructor, destructor, member initializer list, RAII |
| 3 | std::queue, push/pop/front/empty/size |
| 4 | std::vector, copy constructor |
| 5 | template<typename T>, header-only template |

---

# ✅ Cách tự chấm bài

Sau mỗi bài hãy tự kiểm tra:

### 1. Compile với warning đầy đủ

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o main
```

---

### 2. Có copy object không cần thiết không?

Ví dụ:

```cpp
Frame f = ...
```

Có thể thay bằng:

```cpp
const Frame&
```

ở đâu?

---

### 3. Có dùng reference đúng chỗ không?

- Hàm chỉ đọc:

```cpp
const Frame&
```

- Hàm sửa object:

```cpp
Frame&
```

---

### 4. Có dùng `push_back()` đúng chỗ không?

Nếu vector chưa có phần tử:

```cpp
push_back()
```

Không dùng:

```cpp
operator[]
```

---

### 5. Có tận dụng `const` đúng không?

Thử bỏ `const` và quan sát compiler báo lỗi để hiểu `const` đang bảo vệ điều gì.

---

# 🚀 Sau khi hoàn thành

Khi hoàn thành toàn bộ 5 bài, bạn sẽ sẵn sàng chuyển sang **Milestone 1a**.

Lúc đó, `ThreadSafeQueue<T>` gần như chỉ là:

- `SimpleQueue<T>`
- + `std::mutex`
- + `std::condition_variable`

Thay vì phải học một khái niệm hoàn toàn mới.


</details>
