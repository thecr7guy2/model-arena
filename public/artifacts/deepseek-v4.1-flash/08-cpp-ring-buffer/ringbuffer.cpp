#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <thread>

namespace {

constexpr std::size_t kCacheLine = 64;

#if defined(__x86_64__) || defined(__i386__)
inline void cpu_relax() noexcept { __builtin_ia32_pause(); }
#elif defined(__aarch64__)
inline void cpu_relax() noexcept { asm volatile("yield" ::: "memory"); }
#else
inline void cpu_relax() noexcept { std::atomic_signal_fence(std::memory_order_relaxed); }
#endif

template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity >= 2, "capacity must be at least 2");
    static_assert(std::has_single_bit(Capacity), "capacity must be a power of two");

public:
    SpscRingBuffer() noexcept = default;
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    [[nodiscard]] bool try_push(const T& value) noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = head + 1;

        if (next - cached_tail_ > Capacity) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (next - cached_tail_ > Capacity) {
                return false;
            }
        }

        buffer_[head & kMask] = value;
        head_.store(next, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool try_pop(T& value) noexcept {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == cached_head_) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (tail == cached_head_) {
                return false;
            }
        }

        value = buffer_[tail & kMask];
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

private:
    static constexpr std::size_t kMask = Capacity - 1;

    alignas(kCacheLine) std::atomic<std::size_t> head_{0};
    alignas(kCacheLine) std::atomic<std::size_t> tail_{0};
    alignas(kCacheLine) std::size_t cached_tail_{0};
    alignas(kCacheLine) std::size_t cached_head_{0};
    alignas(kCacheLine) std::array<T, Capacity> buffer_{};
};

template <std::size_t Capacity>
bool correctness_test(std::uint64_t count) {
    SpscRingBuffer<std::uint64_t, Capacity> ring;
    std::atomic<bool> producer_done{false};

    std::thread producer([&ring, &producer_done, count] {
        for (std::uint64_t i = 0; i < count; ++i) {
            while (!ring.try_push(i)) {
                cpu_relax();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    std::uint64_t expected = 0;
    bool order_ok = true;
    std::uint64_t mismatch_index = 0;
    std::uint64_t mismatch_actual = 0;

    auto check = [&](std::uint64_t value) {
        if (order_ok && value != expected) {
            order_ok = false;
            mismatch_index = expected;
            mismatch_actual = value;
        }
        ++expected;
    };

    for (;;) {
        std::uint64_t value = 0;
        if (ring.try_pop(value)) {
            check(value);
            continue;
        }
        if (!producer_done.load(std::memory_order_acquire)) {
            cpu_relax();
            continue;
        }
        if (ring.try_pop(value)) {
            check(value);
            continue;
        }
        break;
    }

    producer.join();

    if (!order_ok) {
        std::printf("  first mismatch at index %" PRIu64 ": expected %" PRIu64 ", got %" PRIu64 "\n",
                    mismatch_index, mismatch_index, mismatch_actual);
    }
    if (expected != count) {
        std::printf("  received %" PRIu64 " of %" PRIu64 " values\n", expected, count);
    }
    return order_ok && expected == count;
}

struct TransferStats {
    double seconds;
    std::uint64_t checksum;
};

template <std::size_t Capacity>
TransferStats run_transfer(std::uint64_t count) {
    SpscRingBuffer<std::uint64_t, Capacity> ring;

    const auto start = std::chrono::steady_clock::now();

    std::thread producer([&ring, count] {
        for (std::uint64_t i = 0; i < count; ++i) {
            while (!ring.try_push(i)) {
                cpu_relax();
            }
        }
    });

    std::uint64_t received = 0;
    std::uint64_t checksum = 0;
    while (received < count) {
        std::uint64_t value = 0;
        if (ring.try_pop(value)) {
            checksum += value;
            ++received;
        } else {
            cpu_relax();
        }
    }

    producer.join();
    const auto end = std::chrono::steady_clock::now();
    return {std::chrono::duration<double>(end - start).count(), checksum};
}

}  // namespace

int main() {
    constexpr std::uint64_t kCorrectnessCount = 100'000'000;
    constexpr std::uint64_t kBenchmarkCount = 100'000'000;
    constexpr std::uint64_t kWarmupCount = 2'000'000;
    constexpr std::size_t kCorrectnessCapacity = 1024;
    constexpr std::size_t kBenchmarkCapacity = 65536;

    std::printf("Correctness: passing %" PRIu64 " sequenced integers, capacity %zu\n",
                kCorrectnessCount, kCorrectnessCapacity);

    const auto t0 = std::chrono::steady_clock::now();
    const bool passed = correctness_test<kCorrectnessCapacity>(kCorrectnessCount);
    const auto t1 = std::chrono::steady_clock::now();
    const double correctness_seconds = std::chrono::duration<double>(t1 - t0).count();

    if (!passed) {
        std::printf("Correctness: FAIL\n");
        return 1;
    }
    std::printf("Correctness: PASS (order and completeness verified, %.3f s)\n", correctness_seconds);

    (void)run_transfer<kBenchmarkCapacity>(kWarmupCount);

    const TransferStats stats = run_transfer<kBenchmarkCapacity>(kBenchmarkCount);
    const double elements_per_second = static_cast<double>(kBenchmarkCount) / stats.seconds;
    const double operations_per_second = 2.0 * elements_per_second;
    const std::uint64_t expected_checksum = kBenchmarkCount * (kBenchmarkCount - 1) / 2;

    std::printf("Benchmark: %" PRIu64 " elements, capacity %zu, %.3f s\n",
                kBenchmarkCount, kBenchmarkCapacity, stats.seconds);
    std::printf("Throughput: %.2f M elements/s (%.2f M operations/s, push+pop)\n",
                elements_per_second / 1e6, operations_per_second / 1e6);

    if (stats.checksum != expected_checksum) {
        std::printf("Benchmark checksum mismatch: got %" PRIu64 ", expected %" PRIu64 "\n",
                    stats.checksum, expected_checksum);
        return 1;
    }
    std::printf("Benchmark checksum: %" PRIu64 " (verified)\n", stats.checksum);
    return 0;
}