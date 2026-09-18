# BUGS.md — `netcache.cpp` security review findings

Findings ordered roughly by severity. Line numbers refer to the reviewed file.
Bugs 1–5 and 8–9 were reproduced empirically (ASan/UBSan/TSan + socketpair test).

---

### 1. `parse_frame()` (line 15) — out-of-bounds read: 4-byte header `memcpy` before any length check
**What:** `std::memcpy(&len, buf, 4)` executes unconditionally. If the caller passes a buffer shorter than 4 bytes (partial header arrival), 4 bytes are always read regardless of `buflen`.

**Scenario (reproduced):** `parse_frame(buf, 2)` with a 2-byte stack buffer → ASan reports `stack-buffer-overflow`, READ of size 4 in `parse_frame` at the `memcpy`.

**Fix (one line):** Add `if (buflen < 4) return {};` before the `memcpy`.

---

### 2. `parse_frame()` (line 17) — 32-bit overflow in `len + 4` defeats the bounds check
**What:** `len + 4` is computed in 32-bit unsigned arithmetic (`uint32_t + int`), so it wraps modulo 2³². For attacker-supplied lengths ≥ 0xFFFFFFFC the sum wraps to 0–3 and the bounds check `len + 4 > buflen` passes.

**Scenario (reproduced):** Attacker sends header `0xFFFFFFFF` against a 100-byte buffer: `0xFFFFFFFF + 4 == 3`, check passes, then line 18 attempts `std::string(buf+4, 4294967295)` — a ~4 GiB read past a 96-byte payload and a multi-GiB allocation: heap over-read/segfault and memory-exhaustion DoS.

**Fix (one line):** Replace the check with `if (buflen < 4 || len > buflen - 4) return {};` (64-bit-safe, no overflow).

---

### 3. `struct Conn` (lines 22–27) — Rule-of-Three violation: default copy leads to double-free / double-close
**What:** `Conn` owns a raw `char* buf` and an `fd`, has a user-defined destructor, but implicit copy constructor/assignment. Copies share the same `buf` pointer and `fd`. `std::vector<Conn> g_conns` copies elements on reallocation/resize, and those temporaries are destroyed.

**Scenario (reproduced):** `g_conns.emplace_back(...)` twice → vector reallocates, moves/copies elements, destroys the originals → ASan: `attempting double-free` on `delete[] buf` in `~Conn()`. Additionally, each copy's destructor calls `close(fd)`; after a double close, the descriptor number may already have been reused by another accepted connection, so the proxy silently closes an unrelated live socket.

**Fix (one line):** Delete the copy operations (`Conn(const Conn&) = delete; Conn& operator=(const Conn&) = delete;`) and store `std::vector<char>`/`std::unique_ptr<char[]>` (or define proper copy/move ctor/assignment).

---

### 4. `account()` (lines 35–38) — unsynchronized `+=` on shared globals from every worker thread
**What:** `g_stats.bytes_in += n` and `g_stats.frames++` are non-atomic read-modify-write on a plain `uint64_t`, called concurrently ("from every worker thread"). This is a data race → undefined behavior: lost updates and, for readers, potentially torn 64-bit values.

**Scenario (reproduced):** 4 threads × 100k `account(8)` calls under TSan → `ThreadSanitizer: data race ... in account()`; observed counter totals are systematically lower than expected.

**Fix (one line):** Declare members of `Stats` as `std::atomic<uint64_t>` (or guard `account` with a `std::mutex`).

---

### 5. `checksum16()` (lines 43–44) — off-by-one loop reads one element past the end
**What:** `for (size_t i = 0; i <= words.size(); ++i)` should be `i < words.size()`; at `i == words.size()` it reads `words[word.size()]` — out of bounds.

**Scenario (reproduced):** `checksum16({})` on an empty vector reads `words[0]` of an empty vector → ASan: `SEGV on unknown address 0x000000000000` (load of null pointer). On non-empty vectors it silently folds adjacent heap memory into the checksum, producing wrong checksums nondeterministically.

**Fix (one line):** Change the loop condition to `i < words.size()`.

---

### 6. `peer_name()` (lines 50–53) — returns dangling pointer to a destroyed local `std::string`
**What:** `return name.c_str();` hands out a pointer into the internal buffer of `name`, which is destroyed when the function returns. Every use of the result is use-after-free (unbounded read).

**Scenario:** `const char* n = peer_name(fd); std::cout << n;` — with small strings the buffer sits in the (now-invalid) caller stack frame (SSO); with longer names it is freed heap memory. Anything that touches the stack afterwards (another call, interrupt handling) corrupts the printed/logged peer name — garbage names in logs at best, unbounded read at worst.

**Fix (one line):** Return `std::string` by value (`std::string peer_name(int fd) { return "peer-" + std::to_string(fd); }`).

---

### 7. `start_flusher()` (lines 57–58) — detached thread captures local `interval_ms` by reference
**What:** The lambda captures `interval_ms` (a local of `start_flusher`) by reference, then the thread is detached and outlives the function. After `start_flusher()` returns, `usleep(interval_ms * 1000)` reads a reference into a dead stack frame.

**Scenario (reproduced):** TSan run flags a `data race ... Location is stack of main thread` — the flusher reads the reference while `main` writes that stack area. Once the slot is reused (any subsequent call in the frame above), the sleep interval becomes arbitrary garbage: flusher may spin at full speed hammering stdout/g_stats, or sleep e.g. seconds.

**Fix (one line):** Capture by value: `std::thread t([interval_ms] { ... })` (and ideally pass a constant or make it `static constexpr`).

---

### 8. `start_flusher()` (lines 61–62) — flusher reads `g_stats` without synchronization (torn/inconsistent reads)
**What:** Even ignoring Bug 4, the flusher thread reads `g_stats.bytes_in` and `g_stats.frames` as plain `uint64_t` while workers mutate them — a data race and UB; on 32-bit targets this tears the 64-bit read, and the two fields are never mutually consistent.

**Scenario:** Flush prints `bytes=` from a torn low/high half mix (e.g., nonsense magnitude like `18446744073709551615`) precisely while traffic peaks — misleading monitoring on the edge proxy.

**Fix (one line):** Read the atomics of Bug 4 (relaxed load per field) or snapshot both under one mutex lock.

*(Related: the detached thread never terminates and touches `g_stats`/`std::cout` during static destruction at exit — joinable thread with a stop flag avoids shutdown UB.)*

---

### 9. `read_exact()` (lines 69–75) — infinite loop on `recv() == 0` (peer EOF); EINTR/EAGAIN misclassified
**What:** `recv` returning `0` means orderly shutdown, but the loop only checks `r < 0`, so `got` never advances and the loop spins forever burning one core per stuck connection. Also, transient errors (`errno == EINTR`, and `EAGAIN/EWOULDBLOCK` on non-blocking sockets) return `false`/abort rather than retry, so a signaled read drops the connection mid-frame.

**Scenario (reproduced):** Client connects and closes the socket (FIN) before sending the announced frame length → server-side `read_exact` spins at 100% CPU indefinitely (test had to be killed by timeout); repeated across a botnet of such clients this is a trivial CPU-exhaustion DoS.

**Fix (one line):** After `recv`, `if (r == 0) return false;` and loop again on `errno == EINTR` (and on EAGAIN, return false/report would-block distinctly).

---

### 10. Global `g_conns` (line 29) — shared connection registry with no owner or synchronization
**What:** `g_conns` is a plain global `std::vector` implied to be touched from worker threads (as the `Stats` comment indicates cross-thread use) while `Conn` is non-thread-safe by design (see Bug 3). Concurrent `push_back`/erase from multiple threads races the vector internals (corrupt `begin/end` pointers, crashes); even serialized mutation invalidates any `Conn*`/iterator another thread holds.

**Scenario:** Two threads accept simultaneously and both `g_conns.push_back(...)` → interleaved `size`/capacity updates under reallocation → corrupted vector, crash under load; or a worker holding `&g_conns[0]` dereferences it after another thread's insert moved the buffer.

**Fix (one line):** Protect every access with a mutex, or replace with per-thread/per-event-loop registries (plus `std::deque<Conn>`-style stable storage or indirection via `std::unique_ptr`).

---

## Summary table

| # | Location | Class | Root cause |
|---|----------|-------|-----------|
| 1 | `parse_frame` L15 | Memory safety | `memcpy` before length validation |
| 2 | `parse_frame` L17 | Memory safety / UB | 32-bit `len + 4` overflow |
| 3 | `Conn` L22–27 | Memory safety / fd leak | Rule-of-three violation, double free/close |
| 4 | `account` L35–38 | Concurrency / UB | Non-atomic RMW on shared state |
| 5 | `checksum16` L43 | Memory safety | `i <= size()` off-by-one OOB read |
| 6 | `peer_name` L50–53 | Lifetime | Dangling `c_str()` of local |
| 7 | `start_flusher` L57–58 | Lifetime / UB | By-reference capture of dead local in detached thread |
| 8 | `start_flusher` L61–62 | Concurrency | Unsynchronized/torn reads of `g_stats` |
| 9 | `read_exact` L69–75 | Protocol / DoS | `recv()==0` spins forever; EINTR/EAGAIN mishandled |
| 10 | `g_conns` L29 | Concurrency | Unsynced shared container of non-copyable-semantics objects |
