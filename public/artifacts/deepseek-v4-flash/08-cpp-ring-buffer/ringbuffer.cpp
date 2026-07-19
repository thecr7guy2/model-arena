#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

static constexpr size_t kCacheLineSize = 64;

template <typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two");

    // Producer write index — written by producer, read by consumer
    alignas(kCacheLineSize) std::atomic<size_t> tail_{0};

    char pad0_[kCacheLineSize - sizeof(std::atomic<size_t>)];

    // Consumer read index — written by consumer, read by producer
    alignas(kCacheLineSize) std::atomic<size_t> head_{0};

    char pad1_[kCacheLineSize - sizeof(std::atomic<size_t>)];

    // Payload storage, on its own cache line
    alignas(kCacheLineSize) T buffer_[Capacity];

    static constexpr size_t kMask = Capacity - 1;

public:
    RingBuffer() = default;

    // Called by producer thread. Returns false when full.
    bool push(const T& value) {
        size_t tail = tail_.load(std::memory_order::relaxed);
        size_t head = head_.load(std::memory_order::acquire);
        if (tail - head >= Capacity)
            return false;
        buffer_[tail & kMask] = value;
        tail_.store(tail + 1, std::memory_order::release);
        return true;
    }

    // Called by consumer thread. Returns false when empty.
    bool pop(T& value) {
        size_t head = head_.load(std::memory_order::relaxed);
        size_t tail = tail_.load(std::memory_order::acquire);
        if (head == tail)
            return false;
        value = buffer_[head & kMask];
        head_.store(head + 1, std::memory_order::release);
        return true;
    }
};

// -----------------------------------------------------------------------
// Correctness test — 100 M integers, verifies order and completeness
// -----------------------------------------------------------------------
static constexpr size_t kTestCount = 100'000'000;
static constexpr size_t kBufferCapacity = 1024;

static void correctness_test() {
    RingBuffer<size_t, kBufferCapacity> ring;

    std::thread producer([&ring] {
        for (size_t i = 0; i < kTestCount;) {
            if (ring.push(i))
                ++i;
        }
    });

    std::thread consumer([&ring] {
        size_t expected = 0;
        size_t value;
        for (size_t received = 0; received < kTestCount;) {
            if (ring.pop(value)) {
                if (value != expected) {
                    std::cerr << "FAIL: expected " << expected << " got "
                              << value << '\n';
                    std::exit(1);
                }
                ++expected;
                ++received;
            }
        }
    });

    producer.join();
    consumer.join();
    std::cout << "Correctness test passed: " << kTestCount
              << " integers transmitted in order.\n";
}

// -----------------------------------------------------------------------
// Throughput benchmark
// -----------------------------------------------------------------------
static void throughput_benchmark() {
    RingBuffer<size_t, kBufferCapacity> ring;

    auto start = std::chrono::steady_clock::now();

    std::thread producer([&ring] {
        for (size_t i = 0; i < kTestCount;) {
            if (ring.push(i))
                ++i;
        }
    });

    std::thread consumer([&ring] {
        size_t value;
        for (size_t received = 0; received < kTestCount;) {
            if (ring.pop(value))
                ++received;
        }
    });

    producer.join();
    consumer.join();

    auto end = std::chrono::steady_clock::now();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // Each iteration = 1 push + 1 pop = 2 operations
    double ops_per_sec = static_cast<double>(kTestCount) * 2.0 / (ms / 1000.0);
    std::cout << "Throughput: ~" << static_cast<size_t>(ops_per_sec)
              << " ops/sec  (" << kTestCount << " push+pop pairs in " << ms
              << " ms)\n";
}

// -----------------------------------------------------------------------
int main() {
    correctness_test();
    throughput_benchmark();
    return 0;
}
