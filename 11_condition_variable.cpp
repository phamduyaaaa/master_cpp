#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex m;
std::condition_variable cv;
int data_count = 0;

void consumer() {
    for (int i = 0; i < 3; i++) {
        std::unique_lock<std::mutex> lock(m);

        cv.wait(lock, [i] { return data_count > i; });

        std::cout << "Consumer: Received " << i + 1 << "\n";
    }
}
void producer() {
    for (int i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        {
            std::unique_lock<std::mutex> lock(m);
            data_count++;
        }

        cv.notify_one();
        std::cout << "Producer: Sent sign!\n";
    };
}

int main() {
    std::thread t_consumer(consumer);
    std::thread t_producer(producer);

    t_consumer.join();
    t_producer.join();

    return 0;
}
