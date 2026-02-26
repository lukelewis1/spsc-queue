#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <class T>

class BlockingQueue {

public:
    BlockingQueue() : capacity_(0) {}

    explicit BlockingQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) throw std::invalid_argument("capacity must be > 0");
    }

    // Throws if queue is closed
    void push(T value) {
        //ensures no other threads can modify queue at same time
        std::unique_lock<std::mutex> ul(mutex_);

        //producer waits as long as queue is full
        //releases mutex so other threads can access the queue i.e. consumer
        not_full_.wait(ul, [this] {
            return closed_ || queue_.size() < capacity_;
        });

        if (closed_) {
            throw std::runtime_error("push() on closed BlockingQueue");
        }

        //add item to queue
        queue_.push(std::move(value));
        ul.unlock();
        //wake up blocked consumer thread to consume
        not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> ul(mutex_);

        //block consumer while queue is empty
        not_empty_.wait(ul, [this] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty()) {
            throw std::runtime_error("pop() on closed + empty BlockingQueue");
        }

        T value = std::move(queue_.front());
        queue_.pop();

        ul.unlock();
        //wake up blocked producer thread after queue is not full
        not_full_.notify_one();

        return value;
    }

    void close() {
        std::lock_guard<std::mutex> l(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    size_t size() {
        std::lock_guard<std::mutex> lg(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;

    bool closed_ = false;

    std::size_t capacity_;
};
