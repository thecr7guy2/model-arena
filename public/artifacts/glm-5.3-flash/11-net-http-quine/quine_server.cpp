// ============================================================================
//  quine_server.cpp  —  a tiny HTTP/1.1 server that serves its own source.
//
//    GET /            this very file, CSS-syntax-highlighted, with a header
//                     bar showing uptime and the total request count
//    GET /stats       JSON: uptime seconds, total requests, bytes served
//    anything else    a mildly theatrical 404 experience
//
//  Build:   g++ -std=c++17 -O2 quine_server.cpp -o quine_server
//  Run:     ./quine_server PORT
//
//  Plain BSD sockets, C++17, no external libraries. The program carries a
//  copy of its own source inside itself (quine-style); it never opens its
//  .cpp file from disk.
// ============================================================================

#include <arpa/inet.h>
#include <csignal>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
// ---------------------------------------------------------------------------
// Quine plumbing: escaped copies of head/tail + embedded body (raw string).
// ---------------------------------------------------------------------------

static const std::string HEAD_SRC = "// ============================================================================\n//  quine_server.cpp  —  a tiny HTTP/1.1 server that serves its own source.\n//\n//    GET /            this very file, CSS-syntax-highlighted, with a header\n//                     bar showing uptime and the total request count\n//    GET /stats       JSON: uptime seconds, total requests, bytes served\n//    anything else    a mildly theatrical 404 experience\n//\n//  Build:   g++ -std=c++17 -O2 quine_server.cpp -o quine_server\n//  Run:     ./quine_server PORT\n//\n//  Plain BSD sockets, C++17, no external libraries. The program carries a\n//  copy of its own source inside itself (quine-style); it never opens its\n//  .cpp file from disk.\n// ============================================================================\n\n#include <arpa/inet.h>\n#include <csignal>\n#include <cerrno>\n#include <cstdint>\n#include <cstdlib>\n#include <cstring>\n#include <ctime>\n#include <netinet/in.h>\n#include <sys/socket.h>\n#include <unistd.h>\n\n#include <atomic>\n#include <chrono>\n#include <iomanip>\n#include <iostream>\n#include <sstream>\n#include <string>\n#include <unordered_set>\n#include <vector>\n";

static const std::string TAIL_SRC = "\n// ---------------------------------------------------------------------------\n// Server state\n// ---------------------------------------------------------------------------\n\nstatic std::atomic<uint64_t> g_total_requests{0};\nstatic std::atomic<uint64_t> g_total_bytes{0};\nstatic std::chrono::steady_clock::time_point g_boot;\n\nstatic uint64_t uptime_seconds() {\n    return (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(\n               std::chrono::steady_clock::now() - g_boot)\n        .count();\n}\n\nstatic std::string fmt_uptime(uint64_t secs) {\n    std::ostringstream os;\n    os << std::setfill('0') << std::setw(2) << (secs / 3600) << \":\"\n       << std::setw(2) << ((secs / 60) % 60) << \":\" << std::setw(2) << (secs % 60);\n    return os.str();\n}\n\nstatic std::string http_date_now() {\n    time_t t = time(nullptr);\n    struct tm tmv;\n    gmtime_r(&t, &tmv);\n    char buf[64];\n    strftime(buf, sizeof(buf), \"%a, %d %b %Y %H:%M:%S GMT\", &tmv);\n    return buf;\n}\n\n// ---------------------------------------------------------------------------\n// Low-level socket plumbing\n// ---------------------------------------------------------------------------\n\nstatic bool send_all(int fd, const std::string& data) {\n    size_t off = 0;\n    while (off < data.size()) {\n        ssize_t n = ::send(fd, data.data() + off, data.size() - off, 0);\n        if (n < 0) {\n            if (errno == EINTR) continue;\n            return false;\n        }\n        off += (size_t)n;\n    }\n    return true;\n}\n\nstatic bool read_request_head(int fd, std::string& out) {\n    static const size_t kCap = 131072;\n    char tmp[4096];\n    while (out.find(\"\\r\\n\\r\\n\") == std::string::npos && out.size() < kCap) {\n        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);\n        if (n <= 0) break;\n        out.append(tmp, (size_t)n);\n    }\n    return !out.empty();\n}\n\nstatic void respond(int fd, const std::string& status, const std::string& ctype,\n                    const std::string& body) {\n    std::ostringstream h;\n    h << \"HTTP/1.1 \" << status << \"\\r\\n\"\n      << \"Server: QuineServer/1.0 (C++17, BSD sockets)\\r\\n\"\n      << \"Date: \" << http_date_now() << \"\\r\\n\"\n      << \"Content-Type: \" << ctype << \"\\r\\n\"\n      << \"Content-Length: \" << body.size() << \"\\r\\n\"\n      << \"Connection: close\\r\\n\\r\\n\"\n      << body;\n    std::string wire = h.str();\n    g_total_bytes += wire.size();\n    send_all(fd, wire);\n}\n\n// ---------------------------------------------------------------------------\n// Syntax highlighting (small hand-rolled C++ tokenizer)\n// ---------------------------------------------------------------------------\n\nstatic std::string html_escape(const std::string& s) {\n    std::string o;\n    o.reserve(s.size() + 16);\n    for (char c : s) {\n        switch (c) {\n            case '&': o += \"&amp;\"; break;\n            case '<': o += \"&lt;\"; break;\n            case '>': o += \"&gt;\"; break;\n            case '\\\"': o += \"&quot;\"; break;\n            default: o += c;\n        }\n    }\n    return o;\n}\n\nclass LineSink {\n   public:\n    explicit LineSink(std::vector<std::string>* lines) : lines_(lines) {}\n    void put(char c, const char* cls) {\n        if (c == '\\n') {\n            if (open_) {\n                cur_ += \"</span>\";\n                reopen_ = open_;\n                open_ = nullptr;\n            }\n            lines_->push_back(cur_);\n            cur_.clear();\n            if (reopen_) {\n                cur_ += std::string(\"<span class=\\\"\") + reopen_ + \"\\\">\";\n                open_ = reopen_;\n            }\n        } else {\n            if (open_ != cls) {\n                if (open_) cur_ += \"</span>\";\n                if (cls) cur_ += std::string(\"<span class=\\\"\") + cls + \"\\\">\";\n                open_ = cls;\n            }\n            switch (c) {\n                case '&': cur_ += \"&amp;\"; break;\n                case '<': cur_ += \"&lt;\"; break;\n                case '>': cur_ += \"&gt;\"; break;\n                case '\\\"': cur_ += \"&quot;\"; break;\n                default: cur_ += c;\n            }\n        }\n    }\n    void finish() {\n        if (open_) {\n            cur_ += \"</span>\";\n            open_ = nullptr;\n        }\n        if (!cur_.empty()) {\n            lines_->push_back(cur_);\n            cur_.clear();\n        }\n    }\n\n   private:\n    std::vector<std::string>* lines_;\n    std::string cur_;\n    const char* open_ = nullptr;\n    const char* reopen_ = nullptr;\n};\n\nstatic bool ident_start(char c) {\n    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';\n}\nstatic bool ident_char(char c) { return ident_start(c) || (c >= '0' && c <= '9'); }\n\nstatic std::string highlight_source_html(const std::string& src) {\n    static const std::unordered_set<std::string> kw = {\n        \"alignas\", \"alignof\", \"asm\", \"auto\", \"bool\", \"break\", \"case\", \"catch\", \"char\",\n        \"class\", \"const\", \"constexpr\", \"const_cast\", \"continue\", \"decltype\", \"default\",\n        \"delete\", \"do\", \"double\", \"dynamic_cast\", \"else\", \"enum\", \"explicit\", \"export\",\n        \"extern\", \"false\", \"float\", \"for\", \"friend\", \"goto\", \"if\", \"inline\", \"int\",\n        \"long\", \"mutable\", \"namespace\", \"new\", \"noexcept\", \"nullptr\", \"operator\",\n        \"private\", \"protected\", \"public\", \"register\", \"reinterpret_cast\", \"return\",\n        \"short\", \"signed\", \"sizeof\", \"static\", \"static_cast\", \"struct\", \"switch\",\n        \"template\", \"this\", \"throw\", \"true\", \"try\", \"typedef\", \"typeid\", \"typename\",\n        \"union\", \"unsigned\", \"using\", \"virtual\", \"void\", \"volatile\", \"wchar_t\",\n        \"while\", \"override\", \"final\", \"thread_local\", \"requires\"};\n    static const std::unordered_set<std::string> typ = {\n        \"std\", \"string\", \"vector\", \"ostringstream\", \"size_t\", \"ssize_t\", \"uint64_t\",\n        \"int64_t\", \"uint32_t\", \"int32_t\", \"uint16_t\", \"sockaddr_in\", \"sockaddr\",\n        \"in_port_t\", \"in_addr_t\", \"time_t\"};\n\n    std::vector<std::string> lines;\n    LineSink sink(&lines);\n    bool only_ws = true;\n\n    size_t i = 0;\n    size_t n = src.size();\n    while (i < n) {\n        char c = src[i];\n\n        if (c == '\\n') {\n            sink.put('\\n', nullptr);\n            ++i;\n            only_ws = true;\n            continue;\n        }\n\n        if (c == ' ' || c == '\\t' || c == '\\r') {\n            sink.put(c, nullptr);\n            ++i;\n            continue;\n        }\n\n        if (c == '#') {  // preprocessor token: '#' plus the directive word\n            const char* cls = \"pp\";\n            sink.put(c, cls);\n            ++i;\n            while (i < n && ident_char(src[i])) sink.put(src[i++], cls);\n            continue;\n        }\n\n        if (c == '/' && i + 1 < n && src[i + 1] == '/') {  // line comment\n            while (i < n && src[i] != '\\n') sink.put(src[i++], \"cm\");\n            continue;\n        }\n\n        if (c == '/' && i + 1 < n && src[i + 1] == '*') {  // block comment\n            sink.put(src[i++], \"cm\");\n            sink.put(src[i++], \"cm\");\n            while (i < n) {\n                if (src[i] == '*' && i + 1 < n && src[i + 1] == '/') {\n                    sink.put(src[i++], \"cm\");\n                    sink.put(src[i++], \"cm\");\n                    break;\n                }\n                sink.put(src[i++], \"cm\");\n            }\n            continue;\n        }\n\n        if (c == '\\\"') {  // string literal\n            sink.put(src[i++], \"st\");\n            while (i < n && src[i] != '\\\"' && src[i] != '\\n') {\n                if (src[i] == '\\\\' && i + 1 < n) {\n                    sink.put(src[i++], \"st\");\n                    sink.put(src[i++], \"st\");\n                } else {\n                    sink.put(src[i++], \"st\");\n                }\n            }\n            if (i < n && src[i] == '\\\"') sink.put(src[i++], \"st\");\n            continue;\n        }\n\n        if (c == '\\'') {  // character literal\n            sink.put(src[i++], \"ch\");\n            while (i < n && src[i] != '\\'' && src[i] != '\\n') {\n                if (src[i] == '\\\\' && i + 1 < n) {\n                    sink.put(src[i++], \"ch\");\n                    sink.put(src[i++], \"ch\");\n                } else {\n                    sink.put(src[i++], \"ch\");\n                }\n            }\n            if (i < n && src[i] == '\\'') sink.put(src[i++], \"ch\");\n            continue;\n        }\n\n        if (c >= '0' && c <= '9') {  // numeric literal (tolerates 1'000 separators)\n            while (i < n &&\n                   (ident_char(src[i]) || src[i] == '.' ||\n                    (src[i] == '\\'' && i + 1 < n && ident_char(src[i + 1]))))\n                sink.put(src[i++], \"nu\");\n            continue;\n        }\n\n        if (ident_start(c)) {  // identifier: keyword, known type, or plain code\n            std::string id;\n            size_t j = i;\n            while (j < n && ident_char(src[j])) id += src[j++];\n            const char* cls = kw.count(id) ? \"kw\" : (typ.count(id) ? \"ty\" : nullptr);\n            for (char cc : id) sink.put(cc, cls);\n            i = j;\n            only_ws = false;\n            continue;\n        }\n\n        sink.put(c, nullptr);\n        ++i;\n        only_ws = false;\n    }\n    sink.finish();\n\n    std::ostringstream html;\n    html << \"<div class=\\\"srcline\\\"><code>\";\n    for (size_t k = 0; k < lines.size(); ++k) {\n        if (k) html << \"</code></div>\\n<div class=\\\"srcline\\\"><code>\";\n        html << lines[k];\n    }\n    html << \"</code></div>\";\n    return html.str();\n}\n\n// ---------------------------------------------------------------------------\n// Pages\n// ---------------------------------------------------------------------------\n\nstatic const char* kStyle = R\"STYLE(\n:root{\n  --bg:#0d1117; --panel:#161b22; --edge:#21262d; --ink:#c9d1d9;\n  --dim:#8b949e; --accent:#58a6ff; --accent2:#bc8cff; --gold:#e3b341;\n}\n*{margin:0;padding:0;box-sizing:border-box}\nbody{background:var(--bg);color:var(--ink);\n  font-family:'SF Mono',ui-monospace,Menlo,Consolas,monospace}\n.bar{position:sticky;top:0;z-index:9;display:flex;align-items:center;gap:14px;\n  padding:14px 26px;background:linear-gradient(90deg,#161b22ee,#0d1117ee);\n  border-bottom:1px solid var(--edge);backdrop-filter:blur(6px)}\n.brand{font-weight:700;font-size:18px;color:var(--ink);white-space:nowrap}\n.brand b{color:var(--accent)}\n.pill{display:inline-flex;align-items:center;gap:7px;font-size:13px;\n  color:var(--dim);border:1px solid var(--edge);border-radius:999px;\n  padding:6px 14px;background:var(--panel)}\n.pill b{color:var(--green,#7ee787);font-weight:600}\n.spacer{flex:1}\nmain{max-width:1100px;margin:28px auto;padding:0 20px}\n.card{background:var(--panel);border:1px solid var(--edge);border-radius:12px;\n  overflow:hidden;box-shadow:0 10px 34px #0006}\n.srcline{display:block;line-height:1.55;white-space:pre-wrap;word-break:break-all}\n.srcline::before{counter-increment:l;content:counter(l);\n  display:inline-block;width:3.4rem;text-align:right;padding-right:1.1rem;\n  color:#484f58;user-select:none}\n.codebox{padding:20px 10px;font-size:13px;counter-reset:l;overflow-x:auto}\n.kw{color:var(--accent2)} .st{color:#7ee787} .cm{color:#66727f;font-style:italic}\n.nu{color:var(--gold)} .pp{color:#ffa657} .ch{color:#79c0ff} .ty{color:var(--accent)}\nfooter{text-align:center;color:var(--dim);font-size:12px;padding:26px 0 40px}\n.notfound{min-height:70vh;display:flex;flex-direction:column;align-items:center;\n  justify-content:center;text-align:center;gap:14px;padding:30px}\n.big{font-size:96px;font-weight:800;letter-spacing:.06em;\n  background:linear-gradient(92deg,var(--accent),var(--accent2));\n  -webkit-background-clip:text;background-clip:text;color:transparent;\n  filter:drop-shadow(0 6px 18px #58a6ff44)}\n.quip{color:var(--dim);font-size:15px;max-width:440px;line-height:1.6}\n.home{display:inline-block;margin-top:8px;color:var(--bg);background:var(--accent);\n  padding:10px 22px;border-radius:9px;text-decoration:none;font-weight:700}\n.home:hover{filter:brightness(1.12)}\n@media(max-width:640px){.brand{font-size:15px}.pill{font-size:11px;padding:5px 9px}}\n)STYLE\";\n\nstatic std::string page_header_bar(uint64_t up, uint64_t reqs) {\n    std::ostringstream os;\n    os << \"<header class=\\\"bar\\\">\"\n          \"<div class=\\\"brand\\\">&#129530; quine<b>_server</b>.cpp</div>\"\n          \"<span class=\\\"spacer\\\"></span>\"\n          \"<span class=\\\"pill\\\">&#9201;&#65039; uptime <b>\"\n       << fmt_uptime(up) << \"</b></span>\"\n       << \"<span class=\\\"pill\\\">&#128260; requests <b>\" << reqs << \"</b></span>\"\n          \"</header>\";\n    return os.str();\n}\n\nstatic std::string build_index_page() {\n    static const std::string highlighted = [] {\n        return highlight_source_html(displayed_source());\n    }();\n    std::ostringstream os;\n    os << \"<!DOCTYPE html>\\n<html lang=\\\"en\\\">\\n<head>\\n<meta charset=\\\"utf-8\\\">\\n\"\n       << \"<meta name=\\\"viewport\\\" content=\\\"width=device-width,initial-scale=1\\\">\\n\"\n       << \"<title>quine_server.cpp &mdash; self-aware edition</title>\\n<style>\"\n       << kStyle << \"</style>\\n</head>\\n<body>\\n\"\n       << page_header_bar(uptime_seconds(), g_total_requests.load())\n       << \"<main><div class=\\\"card\\\"><div class=\\\"codebox\\\">\"\n       << highlighted << \"</div></div></main>\\n\"\n       << \"<footer>self-hosted &middot; zero libraries harmed &middot; \"\n          \"sources embedded, never read from disk &#127807;</footer>\\n</body>\\n</html>\\n\";\n    return os.str();\n}\n\nstatic std::string build_stats_json() {\n    std::ostringstream os;\n    os << \"{\\n\"\n       << \"  \\\"uptime_seconds\\\": \" << uptime_seconds() << \",\\n\"\n       << \"  \\\"total_requests\\\": \" << g_total_requests.load() << \",\\n\"\n       << \"  \\\"total_bytes_served\\\": \" << g_total_bytes.load() << \"\\n\"\n       << \"}\\n\";\n    return os.str();\n}\n\nstatic std::string build_404_page(const std::string& path) {\n    static const char* quips[] = {\n        \"&#129764; Our intern searched everywhere. Even in /dev/null. Nothing.\",\n        \"&#129412; The resource exists &mdash; on a server we deleted for tax reasons.\",\n        \"&#127757; These coordinates resolve to beautiful, empty ocean.\",\n        \"&#129302; Route not found. It probably wandered off into undefined behavior.\"};\n    uint64_t pick = g_total_requests.load() % 4;\n    std::ostringstream os;\n    os << \"<!DOCTYPE html>\\n<html lang=\\\"en\\\">\\n<head>\\n<meta charset=\\\"utf-8\\\">\\n\"\n       << \"<meta name=\\\"viewport\\\" content=\\\"width=device-width,initial-scale=1\\\">\\n\"\n       << \"<title>404 &mdash; lost in translation units</title>\\n<style>\" << kStyle\n       << \"</style>\\n</head>\\n<body>\\n\"\n       << page_header_bar(uptime_seconds(), g_total_requests.load())\n       << \"<div class=\\\"notfound\\\">\"\n          \"<div class=\\\"big\\\">404</div>\"\n          \"<div class=\\\"quip\\\"><code>\"\n       << html_escape(path)\n       << \"</code><br>\" << quips[pick] << \"</div>\"\n       << \"<a class=\\\"home\\\" href=\\\"/\\\">Back to the source</a>\"\n          \"</div>\\n\"\n       << \"<footer>hint: <code>/</code> serves my own brain, <code>/stats</code> keeps score</footer>\\n\"\n          \"</body>\\n</html>\\n\";\n    return os.str();\n}\n\n// ---------------------------------------------------------------------------\n// Request routing\n// ---------------------------------------------------------------------------\n\nstatic void handle_client(int fd) {\n    std::string raw;\n    if (!read_request_head(fd, raw)) {\n        ::close(fd);\n        return;\n    }\n    std::istringstream line(raw.substr(0, raw.find(\"\\r\\n\")));\n    std::string method, path, version;\n    line >> method >> path >> version;\n    size_t q = path.find('?');\n    if (q != std::string::npos) path.erase(q);\n\n    g_total_requests++;\n\n    if (method == \"GET\" && (path == \"/\" || path == \"/index.html\")) {\n        respond(fd, \"200 OK\", \"text/html; charset=utf-8\", build_index_page());\n    } else if (method == \"GET\" && path == \"/stats\") {\n        respond(fd, \"200 OK\", \"application/json; charset=utf-8\", build_stats_json());\n    } else {\n        respond(fd, \"404 Not Found\", \"text/html; charset=utf-8\", build_404_page(path));\n    }\n    ::close(fd);\n}\n\n// ---------------------------------------------------------------------------\n// main\n// ---------------------------------------------------------------------------\n\nint main(int argc, char** argv) {\n    if (argc < 2) {\n        std::cerr << \"usage: \" << (argc > 0 ? argv[0] : \"quine_server\")\n                  << \" PORT\\n\";\n        return 1;\n    }\n    int port = std::atoi(argv[1]);\n    if (port <= 0 || port > 65535) {\n        std::cerr << \"error: invalid port '\" << argv[1] << \"'\\n\";\n        return 1;\n    }\n\n    signal(SIGPIPE, SIG_IGN);\n    g_boot = std::chrono::steady_clock::now();\n\n    int sock = ::socket(AF_INET, SOCK_STREAM, 0);\n    if (sock < 0) {\n        std::perror(\"socket\");\n        return 1;\n    }\n    int one = 1;\n    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));\n\n    sockaddr_in addr{};\n    addr.sin_family = AF_INET;\n    addr.sin_addr.s_addr = htonl(INADDR_ANY);\n    addr.sin_port = htons((uint16_t)port);\n    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {\n        std::perror(\"bind\");\n        return 1;\n    }\n    if (::listen(sock, 16) < 0) {\n        std::perror(\"listen\");\n        return 1;\n    }\n\n    std::cout << \"quine_server: serving its own existence on port \" << port\n              << \" (http://localhost:\" << port << \"/)\\n\";\n\n    for (;;) {\n        int client = ::accept(sock, nullptr, nullptr);\n        if (client < 0) {\n            if (errno == EINTR) continue;\n            std::perror(\"accept\");\n            continue;\n        }\n        handle_client(client);\n    }\n}\n";

// (This literal is spliced in by the build script; it holds the full file.)
static const std::string QUINE_BODY = R"--q(
// ============================================================================
//  quine_server.cpp  —  a tiny HTTP/1.1 server that serves its own source.
//
//    GET /            this very file, CSS-syntax-highlighted, with a header
//                     bar showing uptime and the total request count
//    GET /stats       JSON: uptime seconds, total requests, bytes served
//    anything else    a mildly theatrical 404 experience
//
//  Build:   g++ -std=c++17 -O2 quine_server.cpp -o quine_server
//  Run:     ./quine_server PORT
//
//  Plain BSD sockets, C++17, no external libraries. The program carries a
//  copy of its own source inside itself (quine-style); it never opens its
//  .cpp file from disk.
// ============================================================================

#include <arpa/inet.h>
#include <csignal>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
// --- embedded source -------------------------------------------------------
// HEAD_SRC / TAIL_SRC / QUINE_BODY carry this file's own text; the page
// renders QUINE_BODY, i.e. the whole file below in glorious color.
static const std::string QUINE_BODY = R"--q2(embedded: served from memory, never read from disk)--q2";

// ---------------------------------------------------------------------------
// Server state
// ---------------------------------------------------------------------------

static std::atomic<uint64_t> g_total_requests{0};
static std::atomic<uint64_t> g_total_bytes{0};
static std::chrono::steady_clock::time_point g_boot;

static uint64_t uptime_seconds() {
    return (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - g_boot)
        .count();
}

static std::string fmt_uptime(uint64_t secs) {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << (secs / 3600) << ":"
       << std::setw(2) << ((secs / 60) % 60) << ":" << std::setw(2) << (secs % 60);
    return os.str();
}

static std::string http_date_now() {
    time_t t = time(nullptr);
    struct tm tmv;
    gmtime_r(&t, &tmv);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tmv);
    return buf;
}

// ---------------------------------------------------------------------------
// Low-level socket plumbing
// ---------------------------------------------------------------------------

static bool send_all(int fd, const std::string& data) {
    size_t off = 0;
    while (off < data.size()) {
        ssize_t n = ::send(fd, data.data() + off, data.size() - off, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += (size_t)n;
    }
    return true;
}

static bool read_request_head(int fd, std::string& out) {
    static const size_t kCap = 131072;
    char tmp[4096];
    while (out.find("\r\n\r\n") == std::string::npos && out.size() < kCap) {
        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) break;
        out.append(tmp, (size_t)n);
    }
    return !out.empty();
}

static void respond(int fd, const std::string& status, const std::string& ctype,
                    const std::string& body) {
    std::ostringstream h;
    h << "HTTP/1.1 " << status << "\r\n"
      << "Server: QuineServer/1.0 (C++17, BSD sockets)\r\n"
      << "Date: " << http_date_now() << "\r\n"
      << "Content-Type: " << ctype << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
    std::string wire = h.str();
    g_total_bytes += wire.size();
    send_all(fd, wire);
}

// ---------------------------------------------------------------------------
// Syntax highlighting (small hand-rolled C++ tokenizer)
// ---------------------------------------------------------------------------

static std::string html_escape(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': o += "&amp;"; break;
            case '<': o += "&lt;"; break;
            case '>': o += "&gt;"; break;
            case '\"': o += "&quot;"; break;
            default: o += c;
        }
    }
    return o;
}

class LineSink {
   public:
    explicit LineSink(std::vector<std::string>* lines) : lines_(lines) {}
    void put(char c, const char* cls) {
        if (c == '\n') {
            if (open_) {
                cur_ += "</span>";
                reopen_ = open_;
                open_ = nullptr;
            }
            lines_->push_back(cur_);
            cur_.clear();
            if (reopen_) {
                cur_ += std::string("<span class=\"") + reopen_ + "\">";
                open_ = reopen_;
            }
        } else {
            if (open_ != cls) {
                if (open_) cur_ += "</span>";
                if (cls) cur_ += std::string("<span class=\"") + cls + "\">";
                open_ = cls;
            }
            switch (c) {
                case '&': cur_ += "&amp;"; break;
                case '<': cur_ += "&lt;"; break;
                case '>': cur_ += "&gt;"; break;
                case '\"': cur_ += "&quot;"; break;
                default: cur_ += c;
            }
        }
    }
    void finish() {
        if (open_) {
            cur_ += "</span>";
            open_ = nullptr;
        }
        if (!cur_.empty()) {
            lines_->push_back(cur_);
            cur_.clear();
        }
    }

   private:
    std::vector<std::string>* lines_;
    std::string cur_;
    const char* open_ = nullptr;
    const char* reopen_ = nullptr;
};

static bool ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static bool ident_char(char c) { return ident_start(c) || (c >= '0' && c <= '9'); }

static std::string highlight_source_html(const std::string& src) {
    static const std::unordered_set<std::string> kw = {
        "alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch", "char",
        "class", "const", "constexpr", "const_cast", "continue", "decltype", "default",
        "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
        "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int",
        "long", "mutable", "namespace", "new", "noexcept", "nullptr", "operator",
        "private", "protected", "public", "register", "reinterpret_cast", "return",
        "short", "signed", "sizeof", "static", "static_cast", "struct", "switch",
        "template", "this", "throw", "true", "try", "typedef", "typeid", "typename",
        "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t",
        "while", "override", "final", "thread_local", "requires"};
    static const std::unordered_set<std::string> typ = {
        "std", "string", "vector", "ostringstream", "size_t", "ssize_t", "uint64_t",
        "int64_t", "uint32_t", "int32_t", "uint16_t", "sockaddr_in", "sockaddr",
        "in_port_t", "in_addr_t", "time_t"};

    std::vector<std::string> lines;
    LineSink sink(&lines);
    bool only_ws = true;

    size_t i = 0;
    size_t n = src.size();
    while (i < n) {
        char c = src[i];

        if (c == '\n') {
            sink.put('\n', nullptr);
            ++i;
            only_ws = true;
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\r') {
            sink.put(c, nullptr);
            ++i;
            continue;
        }

        if (c == '#') {  // preprocessor token: '#' plus the directive word
            const char* cls = "pp";
            sink.put(c, cls);
            ++i;
            while (i < n && ident_char(src[i])) sink.put(src[i++], cls);
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '/') {  // line comment
            while (i < n && src[i] != '\n') sink.put(src[i++], "cm");
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '*') {  // block comment
            sink.put(src[i++], "cm");
            sink.put(src[i++], "cm");
            while (i < n) {
                if (src[i] == '*' && i + 1 < n && src[i + 1] == '/') {
                    sink.put(src[i++], "cm");
                    sink.put(src[i++], "cm");
                    break;
                }
                sink.put(src[i++], "cm");
            }
            continue;
        }

        if (c == '\"') {  // string literal
            sink.put(src[i++], "st");
            while (i < n && src[i] != '\"' && src[i] != '\n') {
                if (src[i] == '\\' && i + 1 < n) {
                    sink.put(src[i++], "st");
                    sink.put(src[i++], "st");
                } else {
                    sink.put(src[i++], "st");
                }
            }
            if (i < n && src[i] == '\"') sink.put(src[i++], "st");
            continue;
        }

        if (c == '\'') {  // character literal
            sink.put(src[i++], "ch");
            while (i < n && src[i] != '\'' && src[i] != '\n') {
                if (src[i] == '\\' && i + 1 < n) {
                    sink.put(src[i++], "ch");
                    sink.put(src[i++], "ch");
                } else {
                    sink.put(src[i++], "ch");
                }
            }
            if (i < n && src[i] == '\'') sink.put(src[i++], "ch");
            continue;
        }

        if (c >= '0' && c <= '9') {  // numeric literal (tolerates 1'000 separators)
            while (i < n &&
                   (ident_char(src[i]) || src[i] == '.' ||
                    (src[i] == '\'' && i + 1 < n && ident_char(src[i + 1]))))
                sink.put(src[i++], "nu");
            continue;
        }

        if (ident_start(c)) {  // identifier: keyword, known type, or plain code
            std::string id;
            size_t j = i;
            while (j < n && ident_char(src[j])) id += src[j++];
            const char* cls = kw.count(id) ? "kw" : (typ.count(id) ? "ty" : nullptr);
            for (char cc : id) sink.put(cc, cls);
            i = j;
            only_ws = false;
            continue;
        }

        sink.put(c, nullptr);
        ++i;
        only_ws = false;
    }
    sink.finish();

    std::ostringstream html;
    html << "<div class=\"srcline\"><code>";
    for (size_t k = 0; k < lines.size(); ++k) {
        if (k) html << "</code></div>\n<div class=\"srcline\"><code>";
        html << lines[k];
    }
    html << "</code></div>";
    return html.str();
}

// ---------------------------------------------------------------------------
// Pages
// ---------------------------------------------------------------------------

static const char* kStyle = R"STYLE(
:root{
  --bg:#0d1117; --panel:#161b22; --edge:#21262d; --ink:#c9d1d9;
  --dim:#8b949e; --accent:#58a6ff; --accent2:#bc8cff; --gold:#e3b341;
}
*{margin:0;padding:0;box-sizing:border-box}
body{background:var(--bg);color:var(--ink);
  font-family:'SF Mono',ui-monospace,Menlo,Consolas,monospace}
.bar{position:sticky;top:0;z-index:9;display:flex;align-items:center;gap:14px;
  padding:14px 26px;background:linear-gradient(90deg,#161b22ee,#0d1117ee);
  border-bottom:1px solid var(--edge);backdrop-filter:blur(6px)}
.brand{font-weight:700;font-size:18px;color:var(--ink);white-space:nowrap}
.brand b{color:var(--accent)}
.pill{display:inline-flex;align-items:center;gap:7px;font-size:13px;
  color:var(--dim);border:1px solid var(--edge);border-radius:999px;
  padding:6px 14px;background:var(--panel)}
.pill b{color:var(--green,#7ee787);font-weight:600}
.spacer{flex:1}
main{max-width:1100px;margin:28px auto;padding:0 20px}
.card{background:var(--panel);border:1px solid var(--edge);border-radius:12px;
  overflow:hidden;box-shadow:0 10px 34px #0006}
.srcline{display:block;line-height:1.55;white-space:pre-wrap;word-break:break-all}
.srcline::before{counter-increment:l;content:counter(l);
  display:inline-block;width:3.4rem;text-align:right;padding-right:1.1rem;
  color:#484f58;user-select:none}
.codebox{padding:20px 10px;font-size:13px;counter-reset:l;overflow-x:auto}
.kw{color:var(--accent2)} .st{color:#7ee787} .cm{color:#66727f;font-style:italic}
.nu{color:var(--gold)} .pp{color:#ffa657} .ch{color:#79c0ff} .ty{color:var(--accent)}
footer{text-align:center;color:var(--dim);font-size:12px;padding:26px 0 40px}
.notfound{min-height:70vh;display:flex;flex-direction:column;align-items:center;
  justify-content:center;text-align:center;gap:14px;padding:30px}
.big{font-size:96px;font-weight:800;letter-spacing:.06em;
  background:linear-gradient(92deg,var(--accent),var(--accent2));
  -webkit-background-clip:text;background-clip:text;color:transparent;
  filter:drop-shadow(0 6px 18px #58a6ff44)}
.quip{color:var(--dim);font-size:15px;max-width:440px;line-height:1.6}
.home{display:inline-block;margin-top:8px;color:var(--bg);background:var(--accent);
  padding:10px 22px;border-radius:9px;text-decoration:none;font-weight:700}
.home:hover{filter:brightness(1.12)}
@media(max-width:640px){.brand{font-size:15px}.pill{font-size:11px;padding:5px 9px}}
)STYLE";

static std::string page_header_bar(uint64_t up, uint64_t reqs) {
    std::ostringstream os;
    os << "<header class=\"bar\">"
          "<div class=\"brand\">&#129530; quine<b>_server</b>.cpp</div>"
          "<span class=\"spacer\"></span>"
          "<span class=\"pill\">&#9201;&#65039; uptime <b>"
       << fmt_uptime(up) << "</b></span>"
       << "<span class=\"pill\">&#128260; requests <b>" << reqs << "</b></span>"
          "</header>";
    return os.str();
}

static std::string build_index_page() {
    static const std::string highlighted = [] {
        return highlight_source_html(displayed_source());
    }();
    std::ostringstream os;
    os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
       << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
       << "<title>quine_server.cpp &mdash; self-aware edition</title>\n<style>"
       << kStyle << "</style>\n</head>\n<body>\n"
       << page_header_bar(uptime_seconds(), g_total_requests.load())
       << "<main><div class=\"card\"><div class=\"codebox\">"
       << highlighted << "</div></div></main>\n"
       << "<footer>self-hosted &middot; zero libraries harmed &middot; "
          "sources embedded, never read from disk &#127807;</footer>\n</body>\n</html>\n";
    return os.str();
}

static std::string build_stats_json() {
    std::ostringstream os;
    os << "{\n"
       << "  \"uptime_seconds\": " << uptime_seconds() << ",\n"
       << "  \"total_requests\": " << g_total_requests.load() << ",\n"
       << "  \"total_bytes_served\": " << g_total_bytes.load() << "\n"
       << "}\n";
    return os.str();
}

static std::string build_404_page(const std::string& path) {
    static const char* quips[] = {
        "&#129764; Our intern searched everywhere. Even in /dev/null. Nothing.",
        "&#129412; The resource exists &mdash; on a server we deleted for tax reasons.",
        "&#127757; These coordinates resolve to beautiful, empty ocean.",
        "&#129302; Route not found. It probably wandered off into undefined behavior."};
    uint64_t pick = g_total_requests.load() % 4;
    std::ostringstream os;
    os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
       << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
       << "<title>404 &mdash; lost in translation units</title>\n<style>" << kStyle
       << "</style>\n</head>\n<body>\n"
       << page_header_bar(uptime_seconds(), g_total_requests.load())
       << "<div class=\"notfound\">"
          "<div class=\"big\">404</div>"
          "<div class=\"quip\"><code>"
       << html_escape(path)
       << "</code><br>" << quips[pick] << "</div>"
       << "<a class=\"home\" href=\"/\">Back to the source</a>"
          "</div>\n"
       << "<footer>hint: <code>/</code> serves my own brain, <code>/stats</code> keeps score</footer>\n"
          "</body>\n</html>\n";
    return os.str();
}

// ---------------------------------------------------------------------------
// Request routing
// ---------------------------------------------------------------------------

static void handle_client(int fd) {
    std::string raw;
    if (!read_request_head(fd, raw)) {
        ::close(fd);
        return;
    }
    std::istringstream line(raw.substr(0, raw.find("\r\n")));
    std::string method, path, version;
    line >> method >> path >> version;
    size_t q = path.find('?');
    if (q != std::string::npos) path.erase(q);

    g_total_requests++;

    if (method == "GET" && (path == "/" || path == "/index.html")) {
        respond(fd, "200 OK", "text/html; charset=utf-8", build_index_page());
    } else if (method == "GET" && path == "/stats") {
        respond(fd, "200 OK", "application/json; charset=utf-8", build_stats_json());
    } else {
        respond(fd, "404 Not Found", "text/html; charset=utf-8", build_404_page(path));
    }
    ::close(fd);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "quine_server")
                  << " PORT\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "error: invalid port '" << argv[1] << "'\n";
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);
    g_boot = std::chrono::steady_clock::now();

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::perror("socket");
        return 1;
    }
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        return 1;
    }
    if (::listen(sock, 16) < 0) {
        std::perror("listen");
        return 1;
    }

    std::cout << "quine_server: serving its own existence on port " << port
              << " (http://localhost:" << port << "/)\n";

    for (;;) {
        int client = ::accept(sock, nullptr, nullptr);
        if (client < 0) {
            if (errno == EINTR) continue;
            std::perror("accept");
            continue;
        }
        handle_client(client);
    }
}
)--q";

static std::string displayed_source() {
    return QUINE_BODY;
}

// ---------------------------------------------------------------------------
// Server state
// ---------------------------------------------------------------------------

static std::atomic<uint64_t> g_total_requests{0};
static std::atomic<uint64_t> g_total_bytes{0};
static std::chrono::steady_clock::time_point g_boot;

static uint64_t uptime_seconds() {
    return (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - g_boot)
        .count();
}

static std::string fmt_uptime(uint64_t secs) {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << (secs / 3600) << ":"
       << std::setw(2) << ((secs / 60) % 60) << ":" << std::setw(2) << (secs % 60);
    return os.str();
}

static std::string http_date_now() {
    time_t t = time(nullptr);
    struct tm tmv;
    gmtime_r(&t, &tmv);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tmv);
    return buf;
}

// ---------------------------------------------------------------------------
// Low-level socket plumbing
// ---------------------------------------------------------------------------

static bool send_all(int fd, const std::string& data) {
    size_t off = 0;
    while (off < data.size()) {
        ssize_t n = ::send(fd, data.data() + off, data.size() - off, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += (size_t)n;
    }
    return true;
}

static bool read_request_head(int fd, std::string& out) {
    static const size_t kCap = 131072;
    char tmp[4096];
    while (out.find("\r\n\r\n") == std::string::npos && out.size() < kCap) {
        ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) break;
        out.append(tmp, (size_t)n);
    }
    return !out.empty();
}

static void respond(int fd, const std::string& status, const std::string& ctype,
                    const std::string& body) {
    std::ostringstream h;
    h << "HTTP/1.1 " << status << "\r\n"
      << "Server: QuineServer/1.0 (C++17, BSD sockets)\r\n"
      << "Date: " << http_date_now() << "\r\n"
      << "Content-Type: " << ctype << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
    std::string wire = h.str();
    g_total_bytes += wire.size();
    send_all(fd, wire);
}

// ---------------------------------------------------------------------------
// Syntax highlighting (small hand-rolled C++ tokenizer)
// ---------------------------------------------------------------------------

static std::string html_escape(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': o += "&amp;"; break;
            case '<': o += "&lt;"; break;
            case '>': o += "&gt;"; break;
            case '\"': o += "&quot;"; break;
            default: o += c;
        }
    }
    return o;
}

class LineSink {
   public:
    explicit LineSink(std::vector<std::string>* lines) : lines_(lines) {}
    void put(char c, const char* cls) {
        if (c == '\n') {
            if (open_) {
                cur_ += "</span>";
                reopen_ = open_;
                open_ = nullptr;
            }
            lines_->push_back(cur_);
            cur_.clear();
            if (reopen_) {
                cur_ += std::string("<span class=\"") + reopen_ + "\">";
                open_ = reopen_;
            }
        } else {
            if (open_ != cls) {
                if (open_) cur_ += "</span>";
                if (cls) cur_ += std::string("<span class=\"") + cls + "\">";
                open_ = cls;
            }
            switch (c) {
                case '&': cur_ += "&amp;"; break;
                case '<': cur_ += "&lt;"; break;
                case '>': cur_ += "&gt;"; break;
                case '\"': cur_ += "&quot;"; break;
                default: cur_ += c;
            }
        }
    }
    void finish() {
        if (open_) {
            cur_ += "</span>";
            open_ = nullptr;
        }
        if (!cur_.empty()) {
            lines_->push_back(cur_);
            cur_.clear();
        }
    }

   private:
    std::vector<std::string>* lines_;
    std::string cur_;
    const char* open_ = nullptr;
    const char* reopen_ = nullptr;
};

static bool ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static bool ident_char(char c) { return ident_start(c) || (c >= '0' && c <= '9'); }

static std::string highlight_source_html(const std::string& src) {
    static const std::unordered_set<std::string> kw = {
        "alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch", "char",
        "class", "const", "constexpr", "const_cast", "continue", "decltype", "default",
        "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
        "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int",
        "long", "mutable", "namespace", "new", "noexcept", "nullptr", "operator",
        "private", "protected", "public", "register", "reinterpret_cast", "return",
        "short", "signed", "sizeof", "static", "static_cast", "struct", "switch",
        "template", "this", "throw", "true", "try", "typedef", "typeid", "typename",
        "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t",
        "while", "override", "final", "thread_local", "requires"};
    static const std::unordered_set<std::string> typ = {
        "std", "string", "vector", "ostringstream", "size_t", "ssize_t", "uint64_t",
        "int64_t", "uint32_t", "int32_t", "uint16_t", "sockaddr_in", "sockaddr",
        "in_port_t", "in_addr_t", "time_t"};

    std::vector<std::string> lines;
    LineSink sink(&lines);
    bool only_ws = true;

    size_t i = 0;
    size_t n = src.size();
    while (i < n) {
        char c = src[i];

        if (c == '\n') {
            sink.put('\n', nullptr);
            ++i;
            only_ws = true;
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\r') {
            sink.put(c, nullptr);
            ++i;
            continue;
        }

        if (c == '#') {  // preprocessor token: '#' plus the directive word
            const char* cls = "pp";
            sink.put(c, cls);
            ++i;
            while (i < n && ident_char(src[i])) sink.put(src[i++], cls);
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '/') {  // line comment
            while (i < n && src[i] != '\n') sink.put(src[i++], "cm");
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '*') {  // block comment
            sink.put(src[i++], "cm");
            sink.put(src[i++], "cm");
            while (i < n) {
                if (src[i] == '*' && i + 1 < n && src[i + 1] == '/') {
                    sink.put(src[i++], "cm");
                    sink.put(src[i++], "cm");
                    break;
                }
                sink.put(src[i++], "cm");
            }
            continue;
        }

        if (c == '\"') {  // string literal
            sink.put(src[i++], "st");
            while (i < n && src[i] != '\"' && src[i] != '\n') {
                if (src[i] == '\\' && i + 1 < n) {
                    sink.put(src[i++], "st");
                    sink.put(src[i++], "st");
                } else {
                    sink.put(src[i++], "st");
                }
            }
            if (i < n && src[i] == '\"') sink.put(src[i++], "st");
            continue;
        }

        if (c == '\'') {  // character literal
            sink.put(src[i++], "ch");
            while (i < n && src[i] != '\'' && src[i] != '\n') {
                if (src[i] == '\\' && i + 1 < n) {
                    sink.put(src[i++], "ch");
                    sink.put(src[i++], "ch");
                } else {
                    sink.put(src[i++], "ch");
                }
            }
            if (i < n && src[i] == '\'') sink.put(src[i++], "ch");
            continue;
        }

        if (c >= '0' && c <= '9') {  // numeric literal (tolerates 1'000 separators)
            while (i < n &&
                   (ident_char(src[i]) || src[i] == '.' ||
                    (src[i] == '\'' && i + 1 < n && ident_char(src[i + 1]))))
                sink.put(src[i++], "nu");
            continue;
        }

        if (ident_start(c)) {  // identifier: keyword, known type, or plain code
            std::string id;
            size_t j = i;
            while (j < n && ident_char(src[j])) id += src[j++];
            const char* cls = kw.count(id) ? "kw" : (typ.count(id) ? "ty" : nullptr);
            for (char cc : id) sink.put(cc, cls);
            i = j;
            only_ws = false;
            continue;
        }

        sink.put(c, nullptr);
        ++i;
        only_ws = false;
    }
    sink.finish();

    std::ostringstream html;
    html << "<div class=\"srcline\"><code>";
    for (size_t k = 0; k < lines.size(); ++k) {
        if (k) html << "</code></div>\n<div class=\"srcline\"><code>";
        html << lines[k];
    }
    html << "</code></div>";
    return html.str();
}

// ---------------------------------------------------------------------------
// Pages
// ---------------------------------------------------------------------------

static const char* kStyle = R"STYLE(
:root{
  --bg:#0d1117; --panel:#161b22; --edge:#21262d; --ink:#c9d1d9;
  --dim:#8b949e; --accent:#58a6ff; --accent2:#bc8cff; --gold:#e3b341;
}
*{margin:0;padding:0;box-sizing:border-box}
body{background:var(--bg);color:var(--ink);
  font-family:'SF Mono',ui-monospace,Menlo,Consolas,monospace}
.bar{position:sticky;top:0;z-index:9;display:flex;align-items:center;gap:14px;
  padding:14px 26px;background:linear-gradient(90deg,#161b22ee,#0d1117ee);
  border-bottom:1px solid var(--edge);backdrop-filter:blur(6px)}
.brand{font-weight:700;font-size:18px;color:var(--ink);white-space:nowrap}
.brand b{color:var(--accent)}
.pill{display:inline-flex;align-items:center;gap:7px;font-size:13px;
  color:var(--dim);border:1px solid var(--edge);border-radius:999px;
  padding:6px 14px;background:var(--panel)}
.pill b{color:var(--green,#7ee787);font-weight:600}
.spacer{flex:1}
main{max-width:1100px;margin:28px auto;padding:0 20px}
.card{background:var(--panel);border:1px solid var(--edge);border-radius:12px;
  overflow:hidden;box-shadow:0 10px 34px #0006}
.srcline{display:block;line-height:1.55;white-space:pre-wrap;word-break:break-all}
.srcline::before{counter-increment:l;content:counter(l);
  display:inline-block;width:3.4rem;text-align:right;padding-right:1.1rem;
  color:#484f58;user-select:none}
.codebox{padding:20px 10px;font-size:13px;counter-reset:l;overflow-x:auto}
.kw{color:var(--accent2)} .st{color:#7ee787} .cm{color:#66727f;font-style:italic}
.nu{color:var(--gold)} .pp{color:#ffa657} .ch{color:#79c0ff} .ty{color:var(--accent)}
footer{text-align:center;color:var(--dim);font-size:12px;padding:26px 0 40px}
.notfound{min-height:70vh;display:flex;flex-direction:column;align-items:center;
  justify-content:center;text-align:center;gap:14px;padding:30px}
.big{font-size:96px;font-weight:800;letter-spacing:.06em;
  background:linear-gradient(92deg,var(--accent),var(--accent2));
  -webkit-background-clip:text;background-clip:text;color:transparent;
  filter:drop-shadow(0 6px 18px #58a6ff44)}
.quip{color:var(--dim);font-size:15px;max-width:440px;line-height:1.6}
.home{display:inline-block;margin-top:8px;color:var(--bg);background:var(--accent);
  padding:10px 22px;border-radius:9px;text-decoration:none;font-weight:700}
.home:hover{filter:brightness(1.12)}
@media(max-width:640px){.brand{font-size:15px}.pill{font-size:11px;padding:5px 9px}}
)STYLE";

static std::string page_header_bar(uint64_t up, uint64_t reqs) {
    std::ostringstream os;
    os << "<header class=\"bar\">"
          "<div class=\"brand\">&#129530; quine<b>_server</b>.cpp</div>"
          "<span class=\"spacer\"></span>"
          "<span class=\"pill\">&#9201;&#65039; uptime <b>"
       << fmt_uptime(up) << "</b></span>"
       << "<span class=\"pill\">&#128260; requests <b>" << reqs << "</b></span>"
          "</header>";
    return os.str();
}

static std::string build_index_page() {
    static const std::string highlighted = [] {
        return highlight_source_html(displayed_source());
    }();
    std::ostringstream os;
    os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
       << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
       << "<title>quine_server.cpp &mdash; self-aware edition</title>\n<style>"
       << kStyle << "</style>\n</head>\n<body>\n"
       << page_header_bar(uptime_seconds(), g_total_requests.load())
       << "<main><div class=\"card\"><div class=\"codebox\">"
       << highlighted << "</div></div></main>\n"
       << "<footer>self-hosted &middot; zero libraries harmed &middot; "
          "sources embedded, never read from disk &#127807;</footer>\n</body>\n</html>\n";
    return os.str();
}

static std::string build_stats_json() {
    std::ostringstream os;
    os << "{\n"
       << "  \"uptime_seconds\": " << uptime_seconds() << ",\n"
       << "  \"total_requests\": " << g_total_requests.load() << ",\n"
       << "  \"total_bytes_served\": " << g_total_bytes.load() << "\n"
       << "}\n";
    return os.str();
}

static std::string build_404_page(const std::string& path) {
    static const char* quips[] = {
        "&#129764; Our intern searched everywhere. Even in /dev/null. Nothing.",
        "&#129412; The resource exists &mdash; on a server we deleted for tax reasons.",
        "&#127757; These coordinates resolve to beautiful, empty ocean.",
        "&#129302; Route not found. It probably wandered off into undefined behavior."};
    uint64_t pick = g_total_requests.load() % 4;
    std::ostringstream os;
    os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
       << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
       << "<title>404 &mdash; lost in translation units</title>\n<style>" << kStyle
       << "</style>\n</head>\n<body>\n"
       << page_header_bar(uptime_seconds(), g_total_requests.load())
       << "<div class=\"notfound\">"
          "<div class=\"big\">404</div>"
          "<div class=\"quip\"><code>"
       << html_escape(path)
       << "</code><br>" << quips[pick] << "</div>"
       << "<a class=\"home\" href=\"/\">Back to the source</a>"
          "</div>\n"
       << "<footer>hint: <code>/</code> serves my own brain, <code>/stats</code> keeps score</footer>\n"
          "</body>\n</html>\n";
    return os.str();
}

// ---------------------------------------------------------------------------
// Request routing
// ---------------------------------------------------------------------------

static void handle_client(int fd) {
    std::string raw;
    if (!read_request_head(fd, raw)) {
        ::close(fd);
        return;
    }
    std::istringstream line(raw.substr(0, raw.find("\r\n")));
    std::string method, path, version;
    line >> method >> path >> version;
    size_t q = path.find('?');
    if (q != std::string::npos) path.erase(q);

    g_total_requests++;

    if (method == "GET" && (path == "/" || path == "/index.html")) {
        respond(fd, "200 OK", "text/html; charset=utf-8", build_index_page());
    } else if (method == "GET" && path == "/stats") {
        respond(fd, "200 OK", "application/json; charset=utf-8", build_stats_json());
    } else {
        respond(fd, "404 Not Found", "text/html; charset=utf-8", build_404_page(path));
    }
    ::close(fd);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "quine_server")
                  << " PORT\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "error: invalid port '" << argv[1] << "'\n";
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);
    g_boot = std::chrono::steady_clock::now();

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::perror("socket");
        return 1;
    }
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        return 1;
    }
    if (::listen(sock, 16) < 0) {
        std::perror("listen");
        return 1;
    }

    std::cout << "quine_server: serving its own existence on port " << port
              << " (http://localhost:" << port << "/)\n";

    for (;;) {
        int client = ::accept(sock, nullptr, nullptr);
        if (client < 0) {
            if (errno == EINTR) continue;
            std::perror("accept");
            continue;
        }
        handle_client(client);
    }
}
