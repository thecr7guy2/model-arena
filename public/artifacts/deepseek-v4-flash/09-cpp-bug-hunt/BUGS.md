# Bug Report — netcache.cpp

---

### Bug 1 – `parse_frame` (line ~9): no bounds check before `memcpy`

**What:** `std::memcpy(&len, buf, 4)` reads 4 bytes from `buf` without confirming `buflen >= 4`.

**Scenario:** A partial or empty datagram arrives with `buflen < 4`. The memcpy reads past the buffer boundary — undefined behaviour (crash or information leak).

**Fix:**  
```cpp
if (buflen < 4) return {};
```

---

### Bug 2 – `parse_frame` (line ~12): integer overflow in length guard

**What:** `len + 4` uses 32‑bit unsigned arithmetic. If `len == 0xFFFFFFFF`, the addition wraps to 3, so `3 > buflen` is false even for modest buffers, and the length check is bypassed.

**Scenario:** An attacker sends a frame header with `len = 0xFFFFFFFF`. The overflow passes the guard, and `std::string(buf + 4, 4294967295)` is constructed, either throwing `std::bad_alloc` or performing an out‑of‑bounds read of the heap.

**Fix:**  
```cpp
if (static_cast<uint64_t>(len) + 4 > buflen) return {};
```

---

### Bug 3 – `checksum16` (line ~30): off‑by‑one loop bound

**What:** The loop runs while `i <= words.size()` instead of `i < words.size()`. When `i == words.size()`, `words[i]` indexes one past the last element — undefined behaviour.

**Scenario:** Any call with a non‑empty vector indexes out of bounds. If the vector is empty the loop still executes once (`i = 0`), also OOB.

**Fix:**  
```cpp
for (size_t i = 0; i < words.size(); ++i)
```

---

### Bug 4 – `peer_name` (line ~36): dangling pointer returned

**What:** Returns `name.c_str()` where `name` is a local `std::string`. The string is destroyed when the function returns, so the caller holds a dangling `const char*`.

**Scenario:** Any caller that uses the returned pointer (e.g., `printf("%s", peer_name(fd))`) reads freed stack memory.

**Fix:** Return `std::string` by value instead of `const char*`.

---

### Bug 5 – `start_flusher` (line ~42): lambda captures local by reference

**What:** `[&]` captures `interval_ms` by reference, but `interval_ms` is a local variable that goes out of scope when `start_flusher` returns. The detached thread reads a dangling reference on every iteration.

**Scenario:** Shortly after `start_flusher()` returns, the thread reads a destroyed stack slot — UB (garbage interval or crash).

**Fix:** Capture by value: `[interval_ms]` or `[=]`.

---

### Bug 6 – `g_stats` / `account` (line ~24): data race on shared non‑atomic struct

**What:** `g_stats` is modified by every worker thread without any synchronisation. Two threads calling `account()` concurrently can lose increments (nonatomic read‑modify‑write).

**Scenario:** Under concurrent traffic, reported `bytes_in` and `frames` are permanently lower than the true counts. On weakly‑ordered architectures the struct fields may be torn.

**Fix:** Replace `uint64_t` with `std::atomic<uint64_t>` (or guard with a mutex).

---

### Bug 7 – `Conn` copy semantics + `std::vector<Conn>` (lines 15–20): double‑free / double‑close

**What:** `Conn` has user‑defined destructor but no copy/move special members. Storing `Conn` in a `std::vector` triggers copies when the vector resizes; the copied object shares the same `buf` pointer and `fd`, so both destructors `delete[] buf` (double‑free) and `close(fd)` (double‑close).

**Scenario:** `g_conns.push_back(Conn(some_fd))` — on the next `push_back` the vector may reallocate, copy‑construct new elements, and destroy the old ones. Heap corruption or arbitrary FD reuse follows.

**Fix:** Delete copy operations and use `std::vector<std::unique_ptr<Conn>>` (or `std::list<Conn>`).

---

### Bug 8 – `read_exact` (line ~56): `recv` returning 0 is not handled

**What:** `recv` returns 0 when the peer has closed the connection. The current code only checks `r < 0`; `r == 0` is silently added to `got`, causing an infinite loop.

**Scenario:** The client disconnects while 1000 bytes are expected and 500 have been received. `recv` returns 0 forever, the loop never exits, and the thread hangs.

**Fix:**  
```cpp
if (r <= 0) return false;
```
