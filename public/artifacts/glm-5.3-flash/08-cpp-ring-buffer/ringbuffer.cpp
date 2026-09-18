// Lock-free SPSC ring buffer (C++20)
//
// Build: g++ -std=c++20 -O2 -pthread ringbuffer.cpp -o ringbuffer
//
// Memory-ordering contract (minimal, acquire/release only):
//  * Producer publishes an element by storing tail_ with release;
//    consumer acquires tail_ before reading the element.
//  * Consumer publishes a freed slot by storing head_ with release;
//    producer acquires head_ before overwriting a slot.
//  * Each thread reads its own cursor with relaxed ordering, since it is
//    the only writer of it. Cached copies are plain (non-atomic) and
//    touched exclusively by their owning thread.

#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <utility>

namespace {

constexpr std::size_t kCacheLine = 64;

struct alignas(kCacheLine) Index {
    std::atomic<std::size_t> v{0};
    char pad_[kCacheLine - sizeof(std::atomic<std::size_t>)];
};

template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(std::has_single_bit(Capacity),
                  "Capacity must be a power of two");

public:
    static constexpr std::size_t capacity() { return Capacity; }

    // Producer only.
    bool try_push(const T& value) {
        const std::size_t t = tail_.v.load(std::memory_order_relaxed);
        if (t - head_cached_ >= Capacity) {           // possibly full
            head_cached_ = head_.v.load(std::memory_order_acquire);
            if (t - head_cached_ >= Capacity)          // definitely full
                return false;
        }
        data_[t & kMask] = value;
        tail_.v.store(t + 1, std::memory_order_release);  // publish element
        return true;
    }

    // Consumer only.
    bool try_pop(T& out) {
        const std::size_t h = head_.v.load(std::memory_order_relaxed);
        if (h == tail_cached_) {                      // possibly empty
            tail_cached_ = tail_.v.load(std::memory_order_acquire);
            if (h == tail_cached_)                    // definitely empty
                return false;
        }
        out = data_[h & kMask];
        head_.v.store(h + 1, std::memory_order_release);  // publish free slot
        return true;
    }

private:
    static constexpr std::size_t kMask = Capacity - 1;

    std::array<T, Capacity> data_{};

    // --- producer-owned cache line(s) ---
    alignas(kCacheLine) Index tail_{};              // published to consumer
    alignas(kCacheLine) std::size_t head_cached_{0};  // local copy of head_

    // --- consumer-owned cache line(s) ---
    alignas(kCacheLine) Index head_{};              // published to producer
    alignas(kCacheLine) std::size_t tail_cached_{0};  // local copy of tail_
};

std::uint64_t g_sink = 0;  // prevents the optimizer from discarding work

bool correctness_test() {
    constexpr std::uint64_t kCount = 100'000'000;
    constexpr std::uint32_t kValueMask = 0x7FFFFFFF;
    SpscRingBuffer<std::uint32_t, 4096> rb;
    std::atomic<bool> ok{true};

    std::thread producer([&] {
        for (std::uint64_t i = 0; i < kCount;) {
            if (rb.try_push(static_cast<std::uint32_t>(i & kValueMask)))
                ++i;
        }
    });

    std::thread consumer([&] {
        for (std::uint64_t got = 0; got < kCount;) {
            std::uint32_t v;
            if (rb.try_pop(v)) {
                if (v != static_cast<std::uint32_t>(got & kValueMask))
                    ok.store(false, std::memory_order_relaxed);
                ++got;
            }
        }
    });

    producer.join();
    consumer.join();

    const bool passed = ok.load(std::memory_order_relaxed);
    std::printf("%s: %llu sequenced integers transferred in order\n",
                passed ? "PASS" : "FAIL",
                static_cast<unsigned long long>(kCount));
    return passed;
}

void benchmark() {
    constexpr std::uint64_t kOps = 50'000'000;
    SpscRingBuffer<std::uint64_t, 4096> rb;

    const auto start = std::chrono::steady_clock::now();

    std::thread producer([&] {
        for (std::uint64_t i = 0; i < kOps;) {
            if (rb.try_push(i)) ++i;
        }
    });
    std::thread consumer([&] {
        std::uint64_t sink = 0;
        for (std::uint64_t got = 0; got < kOps;) {
            std::uint64_t v;
            if (rb.try_pop(v)) {
                sink += v;
                ++got;
            }
        }
        g_sink = sink;
    });

    producer.join();
    consumer.join();

    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double seconds =
        std::chrono::duration<double>(elapsed).count();
    const double mops = static_cast<double>(kOps) / seconds / 1e6;

    std::printf("benchmark: %.2f M ops/s (%.3f s for %llu ops, sink=%llu)\n",
                mops, seconds,
                static_cast<unsigned long long>(kOps),
                static_cast<unsigned long long>(g_sink));
}

}  // namespace

int main() {
    if (!correctness_test())
        return 1;
    benchmark();
    return 0;
}
