#include <iostream>
#include "../include/thread_pool.hpp"
#include <thread>
#include <mutex>

int main() {
    ThreadPool pool(4);
    std::mutex print_mtx;

    for (int i = 0; i < 20; ++i) {
        pool.submit([i, &print_mtx] {
            std::lock_guard<std::mutex> lg(print_mtx);
            std::cout << "Task " << i
                      << " running on thread "
                      << std::this_thread::get_id()
                      << "\n";
        });
    }

    {
        std::lock_guard<std::mutex> lg(print_mtx);
        std::cout << "Submitted tasks.\n";
    }
}
