# BUGS.md — Security review of `netcache.cpp`

Line numbers refer to the listing reviewed. Issues are ordered by severity.

---

## Critical

### 1. `parse_frame` (line 15) — out-of-bounds read when the buffer is shorter than the header

**What:** `std::memcpy(&len, buf, 4)` unconditionally reads 4 bytes, but `buflen < 4` (partial/`unfinished header from a stream) is never checked. This is a heap over-read of up to 3 bytes; the bytes leak into `len` and then drive further processing.

**Scenario:** A peer sends a TCP segment containing only 2 bytes (`\x00\x05`). The caller forwards the partial read buffer to `parse_frame(buf, 2)`. `memcpy` reads 2 bytes past the end of the allocation — ASAN heap-buffer-overflow, potentially a crash, and attacker-influenced `len`.

**Fix:** Add `if (buflen < 4) return {};` before the `memcpy`.

---

### 2. `parse_frame` (line 17) — integer overflow in the length check defeats bounds checking

**What:** `len + 4 > buflen` is computed in 32-bit unsigned arithmetic. For attacker-controlled values near `UINT32_MAX` it wraps: e.g. `len = 0xFFFFFFFD` gives `len + 4 == 1`, so the check against `buflen` passes. `std::string(buf + 4, len)` then constructs a string of ~4 GiB from a tiny buffer: massive out-of-bounds read (heap disclosure / segfault) plus an allocation-DoS.

**Scenario:** Peer sends header `FD FF FF FF` followed by 4 payload bytes. Caller has `buflen == 8`; the check `1 > 8` is false, so it proceeds to read ~4294967293 bytes past the buffer and/or throws `std::bad_alloc`/OOM-kills the proxy.

**Fix:** Check without addition: `if (buflen < 4 || len > buflen - 4) return {};` (the first clause also fixes bug 1).

---

### 3. `Conn` (lines 22–29) — copyable owning RAII type causes double-free, double-close, and dangling pointers

**What:** `Conn` owns `fd` and a heap `buf`, but the class has a user-declared destructor and no copy/move operations. The implicit copy constructor shallow-copies both members; the implicit move constructor is suppressed by the destructor. Copying therefore produces two owners of the same `fd` and `buf`, each of which runs `close(fd); delete[] buf;`.

**Scenario:** `std::vector<Conn> g_conns; g_conns.emplace_back(fd1); g_conns.emplace_back(fd2);` triggers vector reallocation. The first element is copied, then the original is destroyed: `buf` is `delete[]`d (new copy now dangles — use-after-free) and `fd1` is closed. The new copy is destroyed later and closes `fd1` a second time; if the kernel reused `fd1` for an unrelated connection, that connection is silently killed.

**Fix:** Delete copy operations and implement ownership-transferring moves (`Conn(const Conn&) = delete; Conn& operator=(const Conn&) = delete; Conn(Conn&&) noexcept; Conn& operator=(Conn&&) noexcept;`), or store `std::unique_ptr<char[]>`/`std::unique_ptr<Conn>` and let the compiler handle ownership.

---

### 4. `peer_name` (lines 50–52) — returns a dangling pointer to a destroyed local `std::string`

**What:** `name` is a local `std::string`; `name.c_str()` points into its internal buffer, which is freed when the function returns. Every use of the returned pointer is a use-after-free.

**Scenario:** `const char* p = peer_name(fd); log("%s", p);` — the log reads freed heap memory: garbage output, leak of whatever reused the chunk, or a crash under ASAN/with hardened allocators.

**Fix:** Change the return type to `std::string` and return `name;` (or `strdup` with documented ownership).

---

### 5. `account`/`g_stats` with `start_flusher` (lines 32–38, 61–62) — data races on shared statistics

**What:** `g_stats.bytes_in` and `g_stats.frames` are plain `uint64_t` fields incremented from every worker thread without synchronization, while the flusher thread reads them concurrently. Unsynchronized read/write of the same object is undefined behavior (not merely "lost updates"); 64-bit loads/stores can also tear on 32-bit targets. Additionally the two fields are read at different times, so the printed pair can be inconsistent.

**Scenario:** Two workers call `account()` simultaneously: both read the same old `bytes_in`, one update is lost. The flusher may print torn or stale values, and the compiler may legally reorder/cache the fields. Under TSan this is reported as a race; on some architectures it can read a half-written value.

**Fix:** Make both fields `std::atomic<uint64_t>` and use `fetch_add(..., std::memory_order_relaxed)`; if a consistent `(bytes, frames)` snapshot is required, guard with a mutex or use a seqlock.

---

### 6. `g_conns` (lines 29, and its consumers) — `std::vector<Conn>` shared across threads without synchronization

**What:** The global vector is presumably mutated and iterated by worker threads with no mutex. Concurrent `push_back`/`erase` while another thread iterates is a data race and invalidates iterators/references; combined with bug 3, reallocation frees `fd`/`buf` still in use by another thread.

**Scenario:** Worker A is scanning `g_conns` to write to each peer; worker B accepts a new connection and calls `push_back`. The reallocation moves the elements and destroys the old storage while A holds a reference to an element — A then reads/writes freed memory and may write to a recycled fd.

**Fix:** Protect all access with a `std::mutex`, store `std::unique_ptr<Conn>` so reallocation never moves ownership, and copy a snapshot of the list under the lock before iterating.

---

## High

### 7. `checksum16` (line 43) — off-by-one loop bound reads past the end of the vector

**What:** The loop condition is `i <= words.size()`, so the last iteration reads `words[words.size()]` — one element past the end. For an empty vector, the very first iteration reads `words[0]`. Both are out-of-bounds `operator[]` (UB), and the bogus word silently corrupts the checksum.

**Scenario:** `checksum16({0x1234, 0x5678})` reads a third, nonexistent element (stack/heap garbage or ASAN heap-buffer-overflow), producing a checksum that no peer can validate. `checksum16({})` reads out of bounds immediately.

**Fix:** Use `for (size_t i = 0; i < words.size(); ++i)`.

---

### 8. `read_exact` (lines 72–74) — `recv` returning 0 (EOF) is treated as progress → infinite busy loop

**What:** On orderly connection shutdown `recv` returns 0. The code only rejects `r < 0`, so `got += 0` and the `while (got < n)` loop spins forever with no progress, consuming 100% CPU.

**Scenario:** A peer sends 3 bytes of a 4-byte header and then closes. `read_exact` never returns, the worker thread spins at full CPU, and the connection object is never released — a trivial remote DoS.

**Fix:** `if (r <= 0) return false;` (0 means EOF, i.e. fewer than `n` bytes can ever arrive).

---

### 9. `start_flusher` (lines 57–60) — detached thread captures a stack local by reference

**What:** `[&]` captures `interval_ms` by reference. The lambda outlives `start_flusher`'s stack frame because the thread is detached and loops forever, so every iteration reads a dangling stack slot.

**Scenario:** `start_flusher()` returns; the detached thread resumes and evaluates `interval_ms * 1000` against reused stack memory. The value is arbitrary — if negative, `usleep` receives a huge unsigned argument and the thread may sleep for days or forever; otherwise it is UB and can crash.

**Fix:** Capture by value: `std::thread t([interval_ms] { ... });` (and give the thread a proper stop mechanism).

---

### 10. `start_flusher` (line 65) — detached infinite thread has no shutdown/lifetime management

**What:** `t.detach()` abandons the thread. It runs forever and touches `g_stats` and `std::cout` even after `main` returns and static objects are destroyed.

**Scenario:** The process exits while the flusher is between `usleep` and the `std::cout` chain; the stream/statics are already destroyed, so the detached thread crashes the process during shutdown (or after `main`, racing static destruction).

**Fix:** Keep the thread joinable (`std::jthread`) with an atomic stop flag and join it during shutdown; never detach an unbounded global worker.

---

## Minor / hardening

### 11. `read_exact` (line 73) — `EINTR` is treated as a fatal error

**What:** A signal interrupt makes `recv` return -1 with `errno == EINTR`; the code reports failure and tears down a perfectly healthy connection, possibly mid-frame.

**Scenario:** A profiling timer or stop signal arrives while a worker is blocked in `recv`; the connection is dropped and the peer sees a truncated frame.

**Fix:** `if (r < 0 && errno == EINTR) continue;` before the error return (`#include <cerrno>`).

---

### 12. `checksum16` (lines 41–46) — 32-bit accumulator overflows on large inputs, dropping carries

**What:** The RFC1071 fold `while (sum >> 16)` assumes carries accumulate in a wider-than-16-bit accumulator, but `sum` is only 32 bits. For payloads over ~128 KiB (more than 65536 words), the accumulator itself wraps and the carry out of bit 31 is lost, producing a checksum that differs from the RFC result.

**Scenario:** Checksum a 1 MiB frame of `0xFFFF` words: the true sum is ~3.4×10^10 > 2^32, so the computed 16-bit value is wrong and the peer rejects every large frame.

**Fix:** Use a `uint64_t sum` (or fold the high 16 bits into `sum` every N iterations).

---

### 13. `checksum16` (lines 41–46) — host-endianness assumption breaks interoperability

**What:** RFC1071 sums 16-bit words in network byte order. If callers build `words` directly from wire bytes (`memcpy` into `uint16_t`), the sum is byte-order dependent: it is wrong on little-endian hosts relative to the RFC/other architectures.

**Scenario:** An x86 proxy computes a checksum over raw wire words; a big-endian backend computes/validates the same frame and the checksums disagree.

**Fix:** Normalize on ingest/emit (`sum += ntohs(words[i]);` and `htons(...)` on the result), or document and enforce that `words` is already in network order.

---

### 14. `parse_frame` (line 17) — error and valid zero-length frame are indistinguishable

**What:** A malformed/truncated buffer and a valid `[0x00000000]` frame both return `""`. Callers cannot tell "incomplete/error" from "empty payload".

**Scenario:** The protocol uses zero-length frames as keepalives; `parse_frame` returns `""`, the caller interprets it as an error/no-frame and stalls or drops the connection.

**Fix:** Return an explicit status (`enum`/`bool` plus out-param, or `std::optional<std::string>` where empty-but-present is distinct).

---

### 15. `Conn` constructor (line 23) — fd leaked if `new char[8192]` throws

**What:** Members are initialized as `fd(fd), buf(new char[8192])`. If the allocation throws `std::bad_alloc`, the constructor never completes, the destructor does not run, and the already-accepted descriptor `fd` is never closed.

**Scenario:** Memory pressure causes the allocation to throw inside an accept loop; each failure leaks one descriptor until the process hits `EMFILE` and can no longer accept connections.

**Fix:** Hold `fd` in an RAII wrapper (or wrap the allocation in `try { ... } catch (...) { close(fd); throw; }`), so the descriptor is released on any construction failure.