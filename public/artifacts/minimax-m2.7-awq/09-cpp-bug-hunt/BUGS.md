# Bug Reports — netcache.cpp

## Bug 1: Buffer over-read in `parse_frame`
**Location:** `parse_frame`, line 16 — `std::memcpy(&len, buf, 4);`

**What is wrong:** The `memcpy` reads 4 bytes from `buf` before checking that `buflen >= 4`. If `buflen < 4`, this is an out-of-bounds read.

**Concrete scenario:** Calling `parse_frame(buf, 2)` with any buffer of length 2 reads 2 bytes past the valid buffer.

**Fix:** `if (buflen < 4) return {};` before the memcpy.

---

## Bug 2: Integer overflow in bounds check in `parse_frame`
**Location:** `parse_frame`, line 18 — `if (len + 4 > buflen)`

**What is wrong:** `len` is a `uint32_t` read from the wire. If `len` is `UINT32_MAX`, adding 4 overflows.

**Concrete scenario:** A malicious peer sends length field `0xFFFFFFFD` (4 billion). `len + 4` wraps to a small value, bypassing the bounds check. The subsequent `std::string` constructor reads gigabytes past the buffer.

**Fix:** `if (len > buflen - 4) return {};`

---

## Bug 3: Data race in `account`
**Location:** `account`, lines 37–38

**What is wrong:** `g_stats.bytes_in` and `g_stats.frames` are64-bit integers written by multiple threads without synchronization. Operations like `g_stats.bytes_in += n` are not atomic and constitute a data race (undefined behavior on most architectures, torn reads/writes on others).

**Concrete scenario:** Two threads call `account(100)` simultaneously on a 32-bit system; the increment may be lost due to torn reads/writes.

**Fix:** `std::atomic<uint64_t>` for both members, or a mutex around the updates.

---

## Bug 4: Off-by-one out-of-bounds access in `checksum16`
**Location:** `checksum16`, line 46 — `for (size_t i = 0; i <= words.size(); ++i)`

**What is wrong:** The loop condition is `<=` instead of `<`. When `i == words.size()`, `words[i]` accesses one element past the end of the vector (undefined behavior).

**Concrete scenario:** `checksum16({0x1234})` — the loop iterates with `i=0` and `i=1`; the second iteration reads `words[1]` which is out of bounds.

**Fix:** `for (size_t i = 0; i < words.size(); ++i)`

---

## Bug 5: Dangling pointer return in `peer_name`
**Location:** `peer_name`, line 56 — `return name.c_str();`

**What is wrong:** `name` is a local `std::string` that is destroyed when the function returns. The returned pointer points to freed memory. Any caller using this pointer invokes undefined behavior (typically a crash or garbage string).

**Concrete scenario:**
```cpp
const char* p = peer_name(fd);  // pointer to freed memory
std::cout << p;                  // use-after-free
```

**Fix:** Return `std::string` by value instead: `std::string peer_name(int fd)`.

---

## Bug 6: Use-after-free in `start_flusher` (detached thread with dangling reference)
**Location:** `start_flusher`, line 60 — capture of `interval_ms` by reference in detached thread

**What is wrong:** `interval_ms` is a local stack variable. The thread is detached immediately after creation. When `start_flusher` returns, `interval_ms` goes out of scope, but the detached thread continues running indefinitely, reading `usleep(interval_ms * 1000)` from freed stack memory.

**Concrete scenario:** Calling `start_flusher()` causes a detached background thread that will eventually read garbage from the stack whenever it wakes up.

**Fix:** Capture by value: `[interval_ms]` instead of `[&]`, or make `interval_ms` a global/static.

---

## Bug 7: `Conn` destructor closes invalid fd
**Location:** `Conn` destructor, line 29 — `close(fd);`

**What is wrong:** The constructor accepts any `int fd` without validating it. If `fd` is negative or a stale file descriptor (already closed), `close(fd)` has undefined behavior (on Linux it returns -1 and sets `errno=EBADF`; on other systems it may corrupt resources).

**Concrete scenario:** Constructing `Conn(-1)` and destroying it calls `close(-1)`, which is invalid.

**Fix:** Validate `fd >= 0` in the constructor, or check `fd >= 0` before calling `close` in the destructor.