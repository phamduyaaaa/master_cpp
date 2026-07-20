#include <iostream>
#include <thread>
#include <vector>

void myFunction(int x) {
    std::cout<< "Thread " << x << " is running.\n";
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(myFunction, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Done." << std::endl;
    return 0;
}
