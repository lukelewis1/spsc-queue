//
// Created by Luke Lewis on 26/2/2026.
//
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>

#include "../include/spsc_queue.hpp"

using Clock = std::chrono::steady_clock;

int main() {
    constexpr std::size_t N   = 10'000'000; // messages
    constexpr std::size_t CAP = 1024;       // queue capacity

    SPSCQueue<std::uint64_t> q(CAP);

    std::atomic<bool> start{false};
    std::uint64_t checksum = 0;

    std::thread producer([&] {
        while (!start.load(std::memory_order_acquire)) {}
        for (std::size_t i = 0; i < N; ++i) {
            while (!q.push(static_cast<std::uint64_t>(i))) {}
        }
    });

    std::thread consumer([&] {
        while (!start.load(std::memory_order_acquire)) {}
        std::uint64_t v = 0;
        for (std::size_t i = 0; i < N; ++i) {
            while (!q.pop(v)) {}
            checksum += (v + 1); // stop compiler “helping”
        }
    });

    auto t0 = Clock::now();
    start.store(true, std::memory_order_release);

    producer.join();
    consumer.join();
    auto t1 = Clock::now();

    const double secs = std::chrono::duration<double>(t1 - t0).count();
    const double ops_per_sec = static_cast<double>(N) / secs;

    std::cout << "N=" << (N / 1e6) <<"M" << " CAP=" << CAP << "\n";
    std::cout << "Throughput: " << (ops_per_sec / 1e6) << " M ops/s\n";
    std::cout << "Checksum: " << checksum << "\n";
}