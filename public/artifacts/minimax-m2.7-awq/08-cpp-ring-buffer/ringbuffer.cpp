#include <atomic>
#include <thread>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <optional>

template<typename T>
class spsc_ring_buffer {
    static constexpr size_t CACHE_LINE = 64;
    
    struct padded_atomic : std::atomic<size_t> {
        char padding[CACHE_LINE - sizeof(size_t)];
    };
    
    padded_atomic head_;
    padded_atomic tail_;
    size_t capacity_;
    size_t mask_;
    std::unique_ptr<T[]> buffer_;
    
public:
    explicit spsc_ring_buffer(size_t capacity) : capacity_(capacity), mask_(capacity - 1) {
        buffer_ = std::make_unique<T[]>(capacity + 1);
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }
    
    void push(T item) {
        while (true) {
            size_t head = head_.load(std::memory_order_acquire);
            size_t tail = tail_.load(std::memory_order_relaxed);
            if ((tail - head) < capacity_) {
                size_t idx = tail & mask_;
                buffer_[idx] = std::move(item);
                tail_.store(tail + 1, std::memory_order_release);
                break;
            }
            std::this_thread::yield();
        }
    }
    
    std::optional<T> pop(std::atomic<bool>* done = nullptr) {
        while (true) {
            if (done && done->load(std::memory_order_acquire) && 
                tail_.load(std::memory_order_acquire) == head_.load(std::memory_order_relaxed)) {
                return std::nullopt;
            }
            
            size_t tail = tail_.load(std::memory_order_acquire);
            size_t head = head_.load(std::memory_order_relaxed);
            
            if (head < tail) {
                size_t idx = head & mask_;
                T item = std::move(buffer_[idx]);
                head_.store(head + 1, std::memory_order_release);
                return item;
            }
            
            std::this_thread::yield();
        }
    }
};

void test_correctness(size_t count) {
    const size_t capacity = 1 << 20;
    spsc_ring_buffer<uint64_t> rb(capacity);
    
    std::atomic<bool> producer_done{false};
    std::atomic<size_t> consumed{0};
    
    std::thread consumer([&]() {
        size_t expected = 0;
        while (expected < count) {
            auto val = rb.pop(&producer_done);
            if (!val) break;
            if (*val != expected) {
                std::cerr << "ERROR: expected " << expected << ", got " << *val << "\n";
                return;
            }
            expected++;
            consumed.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    std::thread producer([&]() {
        for (size_t i = 0; i < count; ++i) {
            rb.push(static_cast<uint64_t>(i));
        }
        producer_done.store(true, std::memory_order_release);
    });
    
    producer.join();
    consumer.join();
    
    if (consumed.load() == count) {
        std::cout << "Correctness test PASSED: " << count << " integers transferred in order\n";
    } else {
        std::cerr << "Correctness test FAILED: expected " << count << ", got " << consumed.load() << "\n";
    }
}

void benchmark_throughput(size_t iterations) {
    const size_t capacity = 1 << 20;
    spsc_ring_buffer<uint64_t> rb(capacity);
    
    for (size_t i = 0; i < capacity / 2; ++i) {
        rb.push(static_cast<uint64_t>(i));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        rb.push(static_cast<uint64_t>(i));
        rb.pop();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count();
    double ops_per_sec = iterations / (elapsed_ns / 1e9);
    
    std::cout << "Throughput benchmark: " 
              << std::fixed << std::setprecision(0) 
              << ops_per_sec << " ops/sec\n";
}

int main() {
    const size_t COUNT = 100'000'000;
    
    std::cout << "Running correctness test with " << COUNT << " integers...\n";
    test_correctness(COUNT);
    
    std::cout << "\nRunning throughput benchmark...\n";
    benchmark_throughput(10'000'000);
    
    return 0;
}