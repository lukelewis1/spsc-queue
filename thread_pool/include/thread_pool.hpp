#pragma once

#include <functional>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <utility>


#include "blocking_queue.hpp"

class ThreadPool {

public:

    using Task = std::function<void()>;

    // constructs thread pool of size n_threads threads
    explicit ThreadPool(size_t n_threads);

    // safely and cleanly destructs thread pool
    ~ThreadPool();

    // submits a task to pool
    template<typename F>
    void submit(F&& f) {
        // if stopped, reject this task
        if (stop_.load(std::memory_order_relaxed)) {
            throw std::runtime_error("submit() on stopped ThreadPool");
        }

        tasks_.push(std::function<void()>(std::forward<F>(f)));
    }

private:

    void worker_loop(size_t worker_id);

    BlockingQueue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
};