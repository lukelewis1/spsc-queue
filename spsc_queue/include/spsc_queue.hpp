//
// Created by Luke Lewis on 24/2/2026.
//

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <atomic>
#include <memory>
#include <utility>
#include <cstddef>
#include <stdexcept>

template<typename T, typename Alloc = std::allocator<T>>
class SPSCQueue : private Alloc {
public:
    explicit SPSCQueue(std::size_t capacity, const Alloc& alloc = Alloc{})
    : Alloc(alloc), capacity_{capacity}, ring_{std::allocator_traits<Alloc>::allocate(*this, capacity_)} {
        if (capacity_ == 0) throw std::invalid_argument("SPSCQueue capacity must be > 0");
    }

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    // Empties queue then cleanly destructs
    ~SPSCQueue() {
        T tmp;
        while (pop(tmp)) {}

        std::allocator_traits<Alloc>::deallocate(*this, ring_, capacity_);
    }

    // Constructs element into next ring slot, returns false if full
    bool push(const T& value) {
        const std::size_t head = head_.load(std::memory_order_relaxed); // reads own cursor — no need for synchronisation

        // Checks if queue is full using cached tail — if full, THEN does acquire-load from tail_ and checks again
        if (full(head, cachedTail_)) {
            cachedTail_ = tail_.load(std::memory_order_acquire); // Performs acquire-load
            if (full(head, cachedTail_)) {
                return false;
            }
        }

        // Computes queue index using modulo, then constructs object at the index
        auto* idx = &ring_[head % capacity_];
        std::allocator_traits<Alloc>::construct(*this, idx, value);
        head_.store(head + 1, std::memory_order_release); // Publish updated head for consumer
        return true;
    }

    // Removes given element from ring slot, returns false if empty
    bool pop(T& value) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed); // reads own cursor — no need for synchronisation

        // Checks if queue is empty using cached head — if full, THEN does acquire-load from head_ and checks again
        if (empty(cachedHead_, tail)) {
            cachedHead_ = head_.load(std::memory_order_acquire);
            if (empty(cachedHead_, tail)) {
                return false;
            }
        }

        // Computes queue index, moves item out, then destroys
        auto* idx = &ring_[tail % capacity_];
        value = std::move(*idx);
        std::allocator_traits<Alloc>::destroy(*this, idx);

        tail_.store(tail + 1, std::memory_order_release); // Publish updated tail for producer
        return true;
    }

private:

    // Checks if both cursors equal — no items
    bool empty(std::size_t head, std::size_t tail) const {
        return head == tail;
    }

    // Checks if distance equals capacity — ring is full
    bool full(std::size_t head, std::size_t tail) const {
        return (head - tail) == capacity_;
    }
private:

    // align to cache line boundaries to reduce false sharing
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_ {0};

    alignas(64) std::size_t cachedHead_{0};
    alignas(64) std::size_t cachedTail_ {0};

    std::size_t capacity_;
    T* ring_;
};



#endif //SPSCQUEUE_H
