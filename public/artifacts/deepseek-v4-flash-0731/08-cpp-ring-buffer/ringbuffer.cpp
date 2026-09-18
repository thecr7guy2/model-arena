#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t CAPACITY = 1u << 16; // power of two
constexpr std::size_t MASK = CAPACITY - 1;
constexpr std::size_t CACHE_LINE = 64u;

} // namespace

// Each index lives in its own alignment-padded struct so the two atomics
// never share a cache line (avoids false sharing).
struct alignas(64) ProducerIndex {
    std::atomic<std::size_t> pos{0};
};

struct alignas(64) ConsumerIndex {
    std::atomic<std::size_t> pos{0};
};

template <typename T>
class SPSCRingBuffer {
public:
    explicit SPSCRingBuffer(std::size_t capacity = CAPACITY)
        : capacity_(capacity), mask_(capacity - 1), data_(capacity) {}

    bool push(const T& value) noexcept {
        const auto write_pos = write_.pos.load(std::memory_order_relaxed);
        const auto read_pos = read_.pos.load(std::memory_order_acquire);
        if ((write_pos - read_pos) >= capacity_) {
            return false; // full
        }
        data_[write_pos & mask_] = value;
        write_.pos.store(write_pos + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& value) noexcept {
        const auto read_pos = read_.pos.load(std::memory_order_relaxed);
        const auto write_pos = write_.pos.load(std::memory_order_acquire);
        if (read_pos == write_pos) {
            return false; // empty
        }
        value = data_[read_pos & mask_];
        read_.pos.store(read_pos + 1, std::memory_order_release);
        return true;
    }

    std::size_t capacity() const noexcept { return capacity_; }

private:
    // ---- cache-line-padded producer index ----
    ProducerIndex write_;
    // ---- cache-line-padded consumer index ----
    ConsumerIndex read_;

    std::size_t capacity_;
    std::size_t mask_;
    std::vector<T> data_;
};

bool run_correctness_test() {
    constexpr std::size_t N = 100'000'000u;
    SPSCRingBuffer<int> buf;

    std::atomic<std::size_t> produced{0};
    std::atomic<std::size_t> consumed{0};

    std::thread producer([&] {
        for (std::size_t i = 0; i < N; ++i) {
            int v = static_cast<int>(i);
            while (!buf.push(v)) {
                std::this_thread::yield();
            }
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::atomic<bool> order_ok{true};
    int expected = 0;
    std::thread consumer([&] {
        for (std::size_t i = 0; i < N; ++i) {
            int v = 0;
            while (!buf.pop(v)) {
                std::this_thread::yield();
            }
            if (v != expected) {
                order_ok.store(false, std::memory_order_relaxed);
            }
            ++expected;
            consumed.fetch_add(1, std::memory_order_relaxed);
        }
    });

    producer.join();
    consumer.join();

    const bool ok = order_ok.load(std::memory_order_relaxed) &&
                    produced.load(std::memory_order_relaxed) == N &&
                    consumed.load(std::memory_order_relaxed) == N &&
                    expected == static_cast<int>(N);

    if (!ok) {
        std::cerr << "FAIL: order_ok=" << order_ok.load()
                  << " produced=" << produced.load()
                  << " consumed=" << consumed.load()
                  << " expected=" << expected << '\n';
    }
    return ok;
}

double run_benchmark(std::size_t count = 100'000'000u) {
    SPSCRingBuffer<int> buf;

    auto start = std::chrono::steady_clock::now();
    std::atomic<bool> done{false};

    std::thread producer([&] {
        for (std::size_t i = 0; i < count; ++i) {
            int v = static_cast<int>(i);
            while (!buf.push(v)) {
            }
        }
        done.store(true, std::memory_order_release);
    });

    std::size_t popped = 0;
    while (!(done.load(std::memory_order_acquire) && popped == count)) {
        int v = 0;
        if (buf.pop(v)) {
            ++popped;
        }
    }

    producer.join();
    auto end = std::chrono::steady_clock::now();

    const double seconds =
        std::chrono::duration<double>(end - start).count();
    return static_cast<double>(count) / seconds;
}

int main() {
    std::cout << "Correctness test: passing " << 100'000'000u
              << " integers through SPSC ring buffer...\n";
    if (!run_correctness_test()) {
        std::cerr << "Correctness test FAILED\n";
        return EXIT_FAILURE;
    }
    std::cout << "Correctness test PASSED (all values received in order)\n\n";

    const double ops = run_benchmark();
    std::cout << "Throughput: " << static_cast<long long>(ops)
              << " ops/sec\n";
    return EXIT_SUCCESS;
}
