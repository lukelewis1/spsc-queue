//
// Created by Luke Lewis on 22/2/2026.
//

#include "../include/thread_pool.hpp"

ThreadPool::ThreadPool(size_t n_threads) : tasks_(1024) {

    if (n_threads == 0) {
        throw std::invalid_argument("ThreadPool size must be > 0");
    }

    workers_.reserve(n_threads);
    for (size_t i {}; i < n_threads; i++) {
        workers_.emplace_back(&ThreadPool::worker_loop, this, i);
    }
}

ThreadPool::~ThreadPool() {
    // sets stop flag to true, submit() cannot accept any more work
    stop_.store(true, std::memory_order_relaxed);

    //wake workers
    tasks_.close();

    for (auto& t : workers_) {
        if (t.joinable()) t.join();
    }
}

void ThreadPool::worker_loop(size_t worker_id) {
    while (true) {
        try {
            // blocks until task available (or closed + empty throws)
            auto task = tasks_.pop();
            task(); // run task
        } catch (const std::exception&) {
            // pop() throws when queue closed and empty
            break;
        }
    }
}