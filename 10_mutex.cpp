#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int counter = 0;
std::mutex m;
void increase_count(int threadID) {
    for (int i = 0; i < 1000000; i++) {
        std::unique_lock<std::mutex> lock(m);
        counter++;
    }
}

int main() {
    int num_thread = 3;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_thread; i++) {
        threads.emplace_back(increase_count, i);
    }
    for (auto& t : threads) t.join();
    std::cout << counter << "\n";
    return 0;
}
