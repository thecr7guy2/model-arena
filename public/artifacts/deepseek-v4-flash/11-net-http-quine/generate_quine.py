#!/usr/bin/env python3
"""
Generate a quine-style C++17 HTTP/1.1 server.
The resulting quine_server.cpp serves its own source code, /stats JSON, and 404.
"""

# ---------------------------------------------------------------------------
# The server source code template.  The exact string @@@QUINE_SRC@@@ marks the
# position where the format-string (the quine) will be injected.
# ---------------------------------------------------------------------------
TEMPLATE = r"""#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cstdio>

// ---- quine infrastructure ------------------------------------------------
static const char* quine_fmt = "@@@QUINE_SRC@@@";

static std::string build_self_source() {
    char buf[400000];
    int args[] = { __QUINE_ARGS__ };
    int nargs = sizeof(args) / sizeof(args[0]);
    // We need to interleave args and the %s argument (quine_fmt).
    // Build format by scanning quine_fmt and replacing %c/%s.
    std::string out;
    const char* p = quine_fmt;
    int ai = 0;
    while (*p) {
        if (*p == '%' && *(p+1) == 'c') {
            out += (char)args[ai++];
            p += 2;
        } else if (*p == '%' && *(p+1) == 's') {
            out += quine_fmt;
            p += 2;
        } else {
            out += *p;
            p++;
        }
    }
    return out;
}
// -------------------------------------------------------------------------

static std::string html_escape(const std::string& s) {
    std::string r; r.reserve(s.size() * 2);
    for (unsigned char c : s) {
        switch (c) {
            case '&': r += "&amp;"; break;
            case '<': r += "&lt;"; break;
            case '>': r += "&gt;"; break;
            case '"': r += "&quot;"; break;
            default:  r += c;
        }
    }
    return r;
}

static bool is_keyword(const std::string& w) {
    static const char* kw[] = {
        "alignas","alignof","and","and_eq","asm","auto","bitand","bitor",
        "bool","break","case","catch","char","class","compl","const",
        "constexpr","const_cast","continue","decltype","default","delete",
        "do","double","dynamic_cast","else","enum","explicit","export",
        "extern","false","float","for","friend","goto","if","inline",
        "int","long","mutable","namespace","new","noexcept","not","not_eq",
        "nullptr","operator","or","or_eq","private","protected","public",
        "register","reinterpret_cast","return","short","signed","sizeof",
        "static","static_assert","static_cast","struct","switch","template",
        "this","thread_local","throw","true","try","typedef","typeid",
        "typename","union","unsigned","using","virtual","void","volatile",
        "while","xor","xor_eq","include","define","ifdef","ifndef","endif",
        "pragma","undef","error","line","#include","#define","#ifdef",
        "#ifndef","#endif","#pragma","#undef","#error"
    };
    for (auto* k : kw) if (w == k) return true;
    return false;
}

static bool is_type(const std::string& w) {
    static const char* tp[] = {
        "size_t","ssize_t","int8_t","int16_t","int32_t","int64_t",
        "uint8_t","uint16_t","uint32_t","uint64_t","time_t","clock_t",
        "socklen_t","sa_family_t","in_addr_t","in_port_t","pid_t",
        "std","string","vector","map","unordered_map","set","pair",
        "ostream","istream","stringstream","ostringstream","istringstream",
        "chrono","thread","mutex","lock_guard","unique_ptr","shared_ptr",
        "function","bind","ref","cref"
    };
    for (auto* t : tp) if (w == t) return true;
    return false;
}

static bool is_number(const std::string& w) {
    if (w.empty()) return false;
    for (size_t i = 0; i < w.size(); i++) {
        char c = w[i];
        if (!((c >= '0' && c <= '9') || c == '.' || c == 'x' || c == 'X' ||
              c == 'u' || c == 'U' || c == 'l' || c == 'L' ||
              c == 'f' || c == 'F' || (i == 0 && c == '-')))
            return false;
    }
    return true;
}

static bool is_preproc(const std::string& w) {
    return w == "#include" || w == "#define" || w == "#ifdef" ||
           w == "#ifndef" || w == "#endif" || w == "#pragma" ||
           w == "#undef" || w == "#error" || w == "#if" || w == "#else";
}

static std::string highlight(const std::string& src) {
    std::string r;
    std::string w;
    bool in_string = false;
    bool in_char   = false;
    bool in_preproc = false;
    char str_delim = 0;
    for (size_t i = 0; i < src.size(); i++) {
        unsigned char c = src[i];
        if (in_string || in_char) {
            r += c;
            if (c == '\\' && i + 1 < src.size()) {
                r += src[++i];
            } else if (in_string && c == str_delim) {
                in_string = false;
                r += "</span>";
            } else if (in_char && c == '\'') {
                in_char = false;
                r += "</span>";
            }
            continue;
        }
        // preprocessor line
        if (in_preproc) {
            r += c;
            if (c == '\n') in_preproc = false;
            continue;
        }
        if (c == '#' && (i == 0 || src[i-1] == '\n')) {
            r += "<span class=pp>";
            in_preproc = true;
            r += c;
            continue;
        }
        if (c == '"' || c == '\'') {
            if (!w.empty()) {
                if (is_keyword(w)) {
                    r += "<span class=kw>" + w + "</span>";
                } else if (is_type(w)) {
                    r += "<span class=ty>" + w + "</span>";
                } else if (is_number(w)) {
                    r += "<span class=nm>" + w + "</span>";
                } else {
                    r += w;
                }
                w.clear();
            }
            r += "<span class=st>" + std::string(1, c);
            in_string = (c == '"');
            in_char   = (c == '\'');
            str_delim = c;
            continue;
        }
        if (c == '/' && i + 1 < src.size() && src[i+1] == '/') {
            if (!w.empty()) {
                if (is_keyword(w)) r += "<span class=kw>" + w + "</span>";
                else if (is_type(w)) r += "<span class=ty>" + w + "</span>";
                else r += w;
                w.clear();
            }
            r += "<span class=cm>//";
            i++;
            while (i + 1 < src.size() && src[i+1] != '\n') r += src[++i];
            r += "</span>";
            continue;
        }
        if (c == '/' && i + 1 < src.size() && src[i+1] == '*') {
            if (!w.empty()) {
                if (is_keyword(w)) r += "<span class=kw>" + w + "</span>";
                else if (is_type(w)) r += "<span class=ty>" + w + "</span>";
                else r += w;
                w.clear();
            }
            r += "<span class=cm>/*";
            i += 2;
            while (i < src.size()) {
                r += src[i];
                if (src[i] == '*' && i + 1 < src.size() && src[i+1] == '/') {
                    r += src[++i]; break;
                }
                i++;
            }
            r += "</span>";
            continue;
        }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            c == '_' || c == '#' || (c >= '0' && c <= '9' && !w.empty()) ||
            (c >= '0' && c <= '9' && w.empty())) {
            if (c == '#' && !w.empty()) { r += w; w.clear(); w += c; }
            else { w += c; }
            continue;
        }
        if (!w.empty()) {
            if (is_keyword(w)) {
                if (w == "#include" || w == "#define" || w == "#ifdef" ||
                    w == "#ifndef" || w == "#endif" || w == "#pragma") {
                    r += "<span class=pp>" + w + "</span>";
                } else {
                    r += "<span class=kw>" + w + "</span>";
                }
            } else if (is_type(w)) {
                r += "<span class=ty>" + w + "</span>";
            } else if (is_number(w)) {
                r += "<span class=nm>" + w + "</span>";
            } else {
                r += w;
            }
            w.clear();
        }
        r += c;
    }
    if (!w.empty()) {
        if (is_keyword(w)) r += "<span class=kw>" + w + "</span>";
        else if (is_type(w)) r += "<span class=ty>" + w + "</span>";
        else r += w;
    }
    return r;
}

static std::string page_header(const std::string& self_src,
                               long uptime, long requests) {
    std::string h;
    h += "<!DOCTYPE html><html lang=en><head><meta charset=UTF-8>";
    h += "<title>quine server</title>";
    h += "<style>";
    h += "*{margin:0;padding:0;box-sizing:border-box}";
    h += "body{background:#1e1e1e;color:#d4d4d4;font:14px/1.5 'Cascadia Code','Fira Code','JetBrains Mono','DejaVu Sans Mono',monospace}";
    h += "#bar{position:sticky;top:0;z-index:10;background:#2d2d2d;border-bottom:2px solid #569cd6;padding:8px 20px;display:flex;gap:24px;flex-wrap:wrap;align-items:center;box-shadow:0 2px 8px rgba(0,0,0,.5)}";
    h += "#bar .title{color:#569cd6;font-weight:700;font-size:16px}";
    h += "#bar .stat{color:#9cdcfe}";
    h += "#bar .stat span{color:#ce9178}";
    h += "#code{overflow-x:auto;padding:16px 20px 40px}";
    h += "#code pre{margin:0}";
    h += ".kw{color:#569cd6}.ty{color:#4ec9b0}.st{color:#ce9178}";
    h += ".cm{color:#6a9955}.nm{color:#b5cea8}.pp{color:#c586c0}";
    h += ".ln{color:#858585;user-select:none;text-align:right;padding-right:16px;display:inline-block;width:48px}";
    h += "a{color:#569cd6;text-decoration:none;margin-left:auto}";
    h += "a:hover{text-decoration:underline}";
    h += "</style></head><body>";
    h += "<div id=bar>";
    h += "<span class=title>quine server</span>";
    h += "<span class=stat>uptime <span>" + std::to_string(uptime) + "s</span></span>";
    h += "<span class=stat>requests <span>" + std::to_string(requests) + "</span></span>";
    h += "<a href=/stats>stats</a>";
    h += "</div>";
    h += "<div id=code><pre>";
    return h;
}

static std::string page_footer() {
    return "</pre></div></body></html>";
}

static std::string render_source(const std::string& self_src,
                                  long uptime, long requests) {
    std::string h = page_header(self_src, uptime, requests);
    // Add line numbers and highlighted source
    std::string line;
    size_t lnum = 0;
    for (size_t i = 0; i <= self_src.size(); i++) {
        if (i == self_src.size() || self_src[i] == '\n') {
            lnum++;
            std::string ln = std::to_string(lnum);
            while (ln.size() < 3) ln = " " + ln;
            h += "<span class=ln>" + ln + "</span>";
            h += highlight(line) + "\n";
            line.clear();
        } else {
            line += self_src[i];
        }
    }
    h += page_footer();
    return h;
}

static std::string render_stats(long uptime, long requests, long bytes) {
    std::string j;
    j += "{\"uptime_seconds\":";
    j += std::to_string(uptime);
    j += ",\"total_requests\":";
    j += std::to_string(requests);
    j += ",\"total_bytes_served\":";
    j += std::to_string(bytes);
    j += "}\n";
    return j;
}

static std::string render_404() {
    std::string h;
    h += "<!DOCTYPE html><html lang=en><head><meta charset=UTF-8>";
    h += "<title>404 - not found</title>";
    h += "<style>body{background:#1e1e1e;color:#d4d4d4;font-family:monospace;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;text-align:center}";
    h += "h1{color:#f44747;font-size:72px;margin:0}";
    h += "p{color:#9cdcfe;font-size:18px}";
    h += "pre{color:#6a9955;font-size:14px;margin-top:20px}";
    h += "a{color:#569cd6;text-decoration:none}";
    h += "a:hover{text-decoration:underline}";
    h += "</style></head><body>";
    h += "<h1>404</h1>";
    h += "<p>nothing to see here</p>";
    h += "<pre>";
    h += "    ,-""\"\"-.\n";
    h += "   /        \\\n";
    h += "  /_        _\\\n";
    h += "  //\\      /\\\\\n";
    h += "  |\\ \\    / / |\n";
    h += "  | \\ \\  / /  |\n";
    h += "  |  \\ \\/ /   |\n";
    h += "  |   \\  /    |\n";
    h += "  |    \\/     |\n";
    h += "  |          |\n";
    h += "  |          |\n";
    h += "  |          |\n";
    h += "  |          |\n";
    h += "  |    __    |\n";
    h += "  |   /  \\   |\n";
    h += "  |  /    \\  |\n";
    h += "  | /      \\ |\n";
    h += "  |/        \\|\n";
    h += "  '--....--'\n";
    h += "</pre>";
    h += "<p><a href=/>go home</a></p>";
    h += "</body></html>";
    return h;
}

static void send_response(int fd, int status, const std::string& ct,
                           const std::string& body) {
    std::string r;
    r += "HTTP/1.1 ";
    r += std::to_string(status);
    r += " ";
    switch (status) {
        case 200: r += "OK"; break;
        case 404: r += "Not Found"; break;
        default:  r += "Unknown";
    }
    r += "\r\nContent-Type: ";
    r += ct;
    r += "\r\nContent-Length: ";
    r += std::to_string(body.size());
    r += "\r\nConnection: close\r\n\r\n";
    r += body;
    ::write(fd, r.data(), r.size());
}

int main(int argc, char** argv) {
    if (argc < 2) {
        ::write(2, "Usage: quine_server <port>\n", 27);
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        ::write(2, "invalid port\n", 13);
        return 1;
    }

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { ::write(2, "socket failed\n", 14); return 1; }
    int opt = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (::bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::write(2, "bind failed\n", 12); return 1;
    }
    if (::listen(server_fd, 128) < 0) {
        ::write(2, "listen failed\n", 14); return 1;
    }

    // Build the self-source once at startup
    std::string self_source = build_self_source();

    auto start_time = std::chrono::steady_clock::now();
    long total_requests = 0;
    long total_bytes = 0;

    char buf[16384];

    while (true) {
        struct sockaddr_in cli;
        socklen_t cli_len = sizeof(cli);
        int client_fd = ::accept(server_fd, (struct sockaddr*)&cli, &cli_len);
        if (client_fd < 0) continue;

        total_requests++;
        ssize_t n = ::read(client_fd, buf, sizeof(buf) - 1);
        if (n <= 0) { ::close(client_fd); continue; }
        buf[n] = 0;

        // Parse method and path from the first line
        std::string req(buf);
        std::string method, path;
        size_t sp1 = req.find(' ');
        if (sp1 != std::string::npos) {
            method = req.substr(0, sp1);
            size_t sp2 = req.find(' ', sp1 + 1);
            if (sp2 != std::string::npos) {
                path = req.substr(sp1 + 1, sp2 - sp1 - 1);
            }
        }

        auto now = std::chrono::steady_clock::now();
        long uptime = std::chrono::duration_cast<std::chrono::seconds>(
                          now - start_time).count();

        std::string response_body;
        std::string content_type;
        int status = 200;

        if (method != "GET") {
            status = 404;
            response_body = render_404();
            content_type = "text/html";
        } else if (path == "/" || path.empty()) {
            response_body = render_source(self_source, uptime, total_requests);
            content_type = "text/html; charset=utf-8";
        } else if (path == "/stats") {
            response_body = render_stats(uptime, total_requests, total_bytes);
            content_type = "application/json";
        } else {
            status = 404;
            response_body = render_404();
            content_type = "text/html";
        }

        send_response(client_fd, status, content_type, response_body);
        total_bytes += response_body.size();
        ::close(client_fd);
    }
    ::close(server_fd);
    return 0;
}
"""

# ---------------------------------------------------------------------------
# Quine generation
# ---------------------------------------------------------------------------
def make_quine(template: str, marker: str = "@@@QUINE_SRC@@@"):
    """Turn the template into a self-reproducing C++ source file."""
    # Validate marker appears exactly once
    pos = template.find(marker)
    if pos < 0:
        raise ValueError(f"Marker {marker!r} not found in template")
    if template.find(marker, pos + 1) >= 0:
        raise ValueError(f"Marker {marker!r} appears more than once")

    prefix = template[:pos]
    suffix = template[pos + len(marker):]

    # --- Build the format string and argument list ---
    fmt_chars = []
    args = []

    def add_char(ch):
        if ch == '\n':
            fmt_chars.append('%')
            fmt_chars.append('c')
            args.append('10')
        elif ch == '"':
            fmt_chars.append('%')
            fmt_chars.append('c')
            args.append('34')
        elif ch == '\\':
            fmt_chars.append('%')
            fmt_chars.append('c')
            args.append('92')
        elif ch == '%':
            fmt_chars.append('%')
            fmt_chars.append('%')
        else:
            fmt_chars.append(ch)

    for ch in prefix:
        add_char(ch)

    # Self-reference: %s
    fmt_chars.append('%')
    fmt_chars.append('s')
    args.append('/*SELF*/')

    for ch in suffix:
        add_char(ch)

    fmt_str = ''.join(fmt_chars)

    # --- Build the argument array literal ---
    # Replace the /*SELF*/ marker with the actual C expression for self-ref
    arg_items = []
    for a in args:
        if a == '/*SELF*/':
            arg_items.append('0')  # placeholder, will be handled separately
        else:
            arg_items.append(a)

    # We need to handle %s differently. In our format, %s is ONE position
    # and its "argument value" in the args array doesn't make sense as int.
    # Instead, our runtime code scans the format string and replaces %s
    # with quine_fmt directly (not via the args array).
    # So we output the args array with all the %c arguments only.
    # The %s is handled in build_self_source() by string replacement.

    # Filter out the self-reference placeholder from args
    c_args = [a for a in args if a != '/*SELF*/']
    arg_array = ', '.join(c_args) if c_args else '0'

    # --- Build the final source ---
    result = template.replace(marker, fmt_str)
    result = result.replace("__QUINE_ARGS__", arg_array)

    return result


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if __name__ == '__main__':
    source = make_quine(TEMPLATE)
    with open('quine_server.cpp', 'w') as f:
        f.write(source)
    print("Generated quine_server.cpp")
    print(f"Format string length: {len(source.split('quine_fmt = \"')[1].split('\"')[0]) if 'quine_fmt = \"' in source else 'N/A'}")
