# BUGS.md — netcache.cpp security review findings

Bug reports below. Line numbers refer to the source as pasted in the review.

---

## 1. `parse_frame` — OOB read when `buflen < 4` (line ~14)

- **What's wrong:** `std::memcpy(&len, buf, 4)` unconditionally reads 4 bytes from `buf` before any length check; if `buflen < 4` the read is out of bounds (UB), and `len` holds garbage.
- **Scenario:** A short/garbage TCP segment of 1–3 bytes is handed to `parse_frame`; it reads past the buffer instead of rejecting the frame.
- **Fix:** `if (buflen < 4 || len + 4 > buflen) return {};` before the `memcpy` (and read `len` only after the check).

## 2. `parse_frame` — integer overflow in `len + 4` bypasses bounds check (line ~16)

- **What's wrong:** `len` is `uint32_t`, so `len + 4` can wrap to a small number (or 0). The oversized length then passes `len + 4 > buflen`, and `std::string(buf+4, len)` attempts a ~4 GB allocation and an OOB read.
- **Scenario:** Attacker sends `len = 0xFFFFFFFC`; `len + 4 == 0` passes the check, and the code constructs a 4 GB string from a tiny frame, crashing or hanging the proxy.
- **Fix:** `if (buflen < 4 || len > buflen - 4) return {};` (compute against `buflen - 4`, no overflow).

## 3. `Conn` — copied into `std::vector` causes double `delete[]`/double `close` (struct + `g_conns`, lines ~22–29)

- **What's wrong:** `Conn` declares a destructor but no copy/move handling; its implicit copy copies the raw `buf` and `fd` pointers, and `std::vector<Conn>` copies on reallocation/push_back. Two `Conn` objects then share one heap buffer and one fd, so the buffer is `delete[]`ed twice and the fd `close()`d twice — double-free/use-after-close.
- **Scenario:** `g_conns.push_back(Conn(fd))` later triggers vector reallocation; the old element's destructor frees `buf` and closes `fd`, then the vector's destructor frees the same `buf` again — heap corruption / crash.
- **Fix:** Delete copy ctor/assignment and make `Conn` move-only (or store `std::unique_ptr<Conn>` / heap-allocated `Conn*` in the vector).

## 4. `g_conns` — shared global modified/read from multiple threads without synchronization (line ~30)

- **What's wrong:** The connection list is a plain global `std::vector` with no mutex; concurrent workers doing `push_back`/removal race with readers (and with vector reallocation in bug 3), a data race (UB).
- **Scenario:** Two accepted connections push into `g_conns` simultaneously → torn vector state, iterator invalidation, possible crash.
- **Fix:** Guard all access to `g_conns` with a `std::mutex` (or a concurrent container / per-connection ownership).

## 5. `g_stats` / `account` — unsynchronized concurrent writes (and read by flusher) (lines ~33–41)

- **What's wrong:** `bytes_in += n` and `frames++` are plain `uint64_t` operations on a shared global from every worker thread while the flusher thread reads them — a data race (UB); lost updates and torn reads.
- **Scenario:** Many parallel connections call `account()`; counters undercount or the flusher reads inconsistent values, and the read/write races are UB on the architecture.
- **Fix:** Make the counters `std::atomic<uint64_t>` (or protect with a mutex), e.g. `g_stats.bytes_in.fetch_add(n); g_stats.frames.fetch_add(1);`.

## 6. `checksum16` — off-by-one reads `words[words.size()]` out of bounds (line ~46)

- **What's wrong:** Loop condition `i <= words.size()` iterates one past the end; the final iteration reads `words[words.size()]`, an out-of-bounds read (UB), and corrupts the checksum.
- **Scenario:** Any callsite with at least one word reads past the vector; with ASan it aborts, otherwise the checksum is silently wrong and packets are dropped.
- **Fix:** Change the loop to `for (size_t i = 0; i < words.size(); ++i)`.

## 7. `peer_name` — returns dangling pointer to destroyed local string (line ~52)

- **What's wrong:** `name` is a local `std::string`; returning `name.c_str()` yields a pointer whose storage is freed when the function returns — use-after-free.
- **Scenario:** Caller logs `std::cout << peer_name(fd);`; the string already died at return, so the call reads freed memory (and `name` has no storage for `std::cout` to print correctly).
- **Fix:** Return `std::string` by value (or take a caller-supplied buffer), e.g. `std::string peer_name(int fd) { return "peer-" + std::to_string(fd); }`.

## 8. `start_flusher` — lambda captures local `interval_ms` by reference (line ~59)

- **What's wrong:** The detached thread's lambda captures `interval_ms` with `[&]`, but `interval_ms` is a local of `start_flusher`; after the function returns the reference dangles — UB, and the sleep interval is garbage.
- **Scenario:** `start_flusher()` is called and returns; the detached thread reads the destroyed local `interval_ms` on its next loop iteration → undefined behavior.
- **Fix:** Capture by value, e.g. `[interval_ms]`, and prefer `std::this_thread::sleep_for(std::chrono::milliseconds{interval_ms})`.

## 9. `read_exact` — `recv` returning 0 (EOF/peer close) causes an infinite loop (line ~70)

- **What's wrong:** On `r == 0` (client closed the connection), `got += 0` never advances, so `while (got < n)` spins forever on a closed socket — busy-loop Hang (and it is never even detected as an error).
- **Scenario:** Client connects, sends a partial frame, and closes; the worker calling `read_exact` for the remaining bytes loops forever pegging a CPU.
- **Fix:** Treat EOF as failure: `if (r <= 0) return false;`.
