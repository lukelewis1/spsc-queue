# Concurrency Lab

A C++20 project exploring practical concurrency, memory ordering, and performance-critical systems design.

This repository documents a progression of implementations used to deepen understanding of multithreading and synchronization:

1. A thread-safe blocking queue
2. A thread pool built on top of that queue
3. A lock-free Single-Producer Single-Consumer (SPSC) queue
4. A throughput benchmark

The goal of this project was to learn about concurrency primitives.

---

# Learning Progression

## 1. Thread-Safe Queue (Mutex + Condition Variable)

The starting point was implementing a classic blocking queue using:

- `std::mutex`
- `std::condition_variable`
- RAII locking
- Clean shutdown semantics

---

## 2. Thread Pool

Building on the blocking queue, a thread pool was implemented to manage worker threads and task execution.

Key concepts explored:

- Worker lifecycle management
- Graceful shutdown procedures
- Avoiding false sharing (`alignas(64)`)
- Task submission and ownership transfer
- Move semantics and callable forwarding
- Exception safety in worker threads

---

## 3. SPSC Queue

After understanding lock-based synchronization, the next step was implementing a bounded lock-free queue.

The queue was an implementation of the SPSC Lock-free FIFO designed by Charles Frasch: 
- [Single Producer Single Consumer Lock-free FIFO From the Ground Up - Charles Frasch - CppCon 2023](https://www.youtube.com/watch?v=K3P_Lmq6pw0)

The SPSC Queue is a fixed-capacity wait-free queue optimized for one producer thread and one consumer thread.  
The implementation avoids dynamic allocation during steady-state operation and uses explicit acquire/release semantics to enforce correct cross-thread visibility.

The queue is implemented as a circular buffer (ring) with atomic head and tail indices.

### Design Highlights

- Bounded ring buffer
- Wait-free producer and consumer
- `std::atomic<size_t>` head/tail indices
- Release-store for publishing
- Acquire-load for synchronization
- `alignas(64)` to mitigate false sharing
- Manual object lifetime management via allocator + `construct` / `destroy`

---

# Throughput Benchmark

A microbenchmark measures steady-state performance of the SPSC queue under Release builds.

Configuration:

# Performance

### Benchmark Configuration

- 1 producer thread
- 1 consumer thread
- N = 10,000,000 operations
- Capacity = 1024
- Release build (`-O3 -DNDEBUG`)
- Apple Silicon / Clang (C++20)

### Results (Multiple Runs)

| Run | Throughput (M ops/s) |
|-----|----------------------|
| 1   | 116.49               |
| 2   | 138.29               |
| 3   | 126.66               |
| 4   | 154.39               |
| 5   | 125.95               |
| 6   | 99.87                |
| 7   | 127.21               |

**Average:** ~126.7 M ops/s  
**Peak:** 154.4 M ops/s

Checksum verified for all runs:
50000005000000
---

# Queue API

| Method | Description |
|--------|------------|
| `SPSCQueue(size_t capacity, const Alloc& alloc = Alloc{})` | Constructs a bounded queue with specified capacity. |
| `bool push(const T& value)` | Inserts an element if space is available. |
| `bool pop(T& value)` | Removes the next element if available. |
| `size_t capacity() const` | Returns queue capacity. |
| `size_t size() const` | Returns approximate size (non-linearizable). |

---

# Summary
Overall, this project taught me a lot about concurrency — from implementing a thread-safe blocking queue, to building a thread pool, then finally a lock-free SPSC ring buffer. I began with almost zero knowledge on concurrency and had to use a variety of resources online, and used ChatGPT to fill in the gaps.  

