// quine_server.cpp -- a tiny self-hosting HTTP/1.1 server in a single C++17 file.
//
// Routes:
//   GET /        styled page with this file's complete source, syntax highlighted
//   GET /stats   JSON: uptime seconds, total requests, total bytes served
//   anything     playful 404 page
//
// The binary is a quine: its whole text is embedded at compile time and
// stitched back together at runtime, so no .cpp file is ever read from disk.
//
// Build:  g++ -std=c++17 -O2 quine_server.cpp -o quine_server
// Run:    ./quine_server <port>

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
const Clock::time_point g_start = Clock::now();

unsigned long long g_requests = 0;
unsigned long long g_bytes_served = 0;

// ---------------------------------------------------------------------------
// The quine payload.
//
// SRC_RAW holds the full text of this file, except that the raw string literal
// carrying it is represented by the placeholder token used right below.
// source_code() finds the first placeholder and splices the literal text back
// in, reconstructing quine_server.cpp byte for byte.
// ---------------------------------------------------------------------------
static const char SRC_RAW[] = R"QSRC(// quine_server.cpp -- a tiny self-hosting HTTP/1.1 server in a single C++17 file.
//
// Routes:
//   GET /        styled page with this file's complete source, syntax highlighted
//   GET /stats   JSON: uptime seconds, total requests, total bytes served
//   anything     playful 404 page
//
// The binary is a quine: its whole text is embedded at compile time and
// stitched back together at runtime, so no .cpp file is ever read from disk.
//
// Build:  g++ -std=c++17 -O2 quine_server.cpp -o quine_server
// Run:    ./quine_server <port>

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
const Clock::time_point g_start = Clock::now();

unsigned long long g_requests = 0;
unsigned long long g_bytes_served = 0;

// ---------------------------------------------------------------------------
// The quine payload.
//
// SRC_RAW holds the full text of this file, except that the raw string literal
// carrying it is represented by the placeholder token used right below.
// source_code() finds the first placeholder and splices the literal text back
// in, reconstructing quine_server.cpp byte for byte.
// ---------------------------------------------------------------------------
static const char SRC_RAW[] = @SRC@;

const std::string& source_code() {
  static const std::string text = [] {
    const std::string token = "@SRC@";
    const std::string raw = SRC_RAW;
    const std::string::size_type at = raw.find(token);
    if (at == std::string::npos) return raw;
    return raw.substr(0, at) + "R\"QSRC(" + raw + ")QSRC\"" + raw.substr(at + token.size());
  }();
  return text;
}

std::string html_escape(const std::string& in) {
  std::string out;
  out.reserve(in.size() + in.size() / 8 + 16);
  for (unsigned char c : in) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += static_cast<char>(c); break;
    }
  }
  return out;
}

bool is_ident_start(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_ident_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// A deliberately small C++ lexer.  It is not a compiler front end; it just
// colours comments, strings, chars, numbers, preprocessor lines, keywords,
// known type names and call-like identifiers well enough to look right.
std::string highlight_cpp(const std::string& src) {
  static const std::unordered_set<std::string> keywords = {
      "alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch",
      "char", "class", "const", "constexpr", "const_cast", "continue",
      "decltype", "default", "delete", "do", "double", "dynamic_cast", "else",
      "enum", "explicit", "extern", "false", "float", "for", "friend", "goto",
      "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
      "nullptr", "operator", "private", "protected", "public", "register",
      "reinterpret_cast", "return", "short", "signed", "sizeof", "static",
      "static_assert", "static_cast", "struct", "switch", "template", "this",
      "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
      "union", "unsigned", "using", "virtual", "void", "volatile", "while"};
  static const std::unordered_set<std::string> types = {
      "std", "string", "vector", "unordered_set", "size_t", "ssize_t",
      "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
      "int32_t", "int64_t", "sockaddr_in", "sockaddr", "socklen_t", "in_port_t",
      "in_addr", "Clock", "time_point", "steady_clock"};

  struct Piece {
    std::string text;
    std::string cls;
  };

  std::vector<std::vector<Piece>> lines(1);
  auto add = [&](char c, const char* cls) {
    std::vector<Piece>& line = lines.back();
    if (!line.empty() && line.back().cls == cls) line.back().text.push_back(c);
    else line.push_back(Piece{std::string(1, c), cls});
  };

  enum State { Normal, Block, LineC, Str, Chr, RawStr };
  State state = Normal;
  std::string raw_delim;
  bool at_line_start = true;
  const std::size_t n = src.size();
  std::size_t i = 0;

  while (i < n) {
    const char c = src[i];
    if (c == '\n') { lines.emplace_back(); at_line_start = true; ++i; continue; }

    if (state == LineC) { add(c, "cmt"); ++i; continue; }

    if (state == Block) {
      if (c == '*' && i + 1 < n && src[i + 1] == '/') {
        add('*', "cmt"); add('/', "cmt"); i += 2; state = Normal;
      } else {
        add(c, "cmt"); ++i;
      }
      continue;
    }

    if (state == Str) {
      add(c, "str"); ++i;
      if (c == '\\' && i < n) { add(src[i], "str"); ++i; }
      else if (c == '"') state = Normal;
      continue;
    }

    if (state == Chr) {
      add(c, "chr"); ++i;
      if (c == '\\' && i < n) { add(src[i], "chr"); ++i; }
      else if (c == '\'') state = Normal;
      continue;
    }

    if (state == RawStr) {
      const std::string closer = ")" + raw_delim + "\"";
      if (src.compare(i, closer.size(), closer) == 0) {
        for (char d : closer) add(d, "str");
        i += closer.size();
        state = Normal;
      } else {
        add(c, "str"); ++i;
      }
      continue;
    }

    if (at_line_start && (c == ' ' || c == '\t')) { add(c, ""); ++i; continue; }

    if (at_line_start && c == '#') {
      while (i < n && src[i] != '\n') { add(src[i], "pre"); ++i; }
      continue;
    }

    if (c == '/' && i + 1 < n && src[i + 1] == '/') {
      add('/', "cmt"); add('/', "cmt"); i += 2; state = LineC;
      at_line_start = false; continue;
    }
    if (c == '/' && i + 1 < n && src[i + 1] == '*') {
      add('/', "cmt"); add('*', "cmt"); i += 2; state = Block;
      at_line_start = false; continue;
    }
    if (c == 'R' && i + 1 < n && src[i + 1] == '"' &&
        (i == 0 || !is_ident_char(src[i - 1]))) {
      const std::size_t open = src.find('(', i + 2);
      if (open != std::string::npos && open - (i + 2) <= 24) {
        raw_delim = src.substr(i + 2, open - (i + 2));
        for (std::size_t k = i; k <= open; ++k) add(src[k], "str");
        i = open + 1; state = RawStr; at_line_start = false; continue;
      }
    }
    if (c == '"') { add(c, "str"); ++i; state = Str; at_line_start = false; continue; }
    if (c == '\'') { add(c, "chr"); ++i; state = Chr; at_line_start = false; continue; }

    if (std::isdigit(static_cast<unsigned char>(c))) {
      while (i < n && (std::isalnum(static_cast<unsigned char>(src[i])) ||
                       src[i] == '.' || src[i] == '\'' || src[i] == '_')) {
        add(src[i], "num"); ++i;
      }
      at_line_start = false; continue;
    }

    if (is_ident_start(c)) {
      std::size_t j = i;
      while (j < n && is_ident_char(src[j])) ++j;
      const std::string word = src.substr(i, j - i);
      const char* cls = "";
      if (keywords.count(word)) cls = "kw";
      else if (types.count(word)) cls = "ty";
      else if (j < n && src[j] == '(') cls = "fn";
      for (char d : word) add(d, cls);
      i = j; at_line_start = false; continue;
    }

    add(c, "");
    ++i;
    if (c != ' ' && c != '\t') at_line_start = false;
  }

  std::string out;
  out.reserve(src.size() * 2);
  bool first_line = true;
  for (const std::vector<Piece>& line : lines) {
    if (!first_line) out += '\n';
    first_line = false;
    out += "<span class=\"line\">";
    for (const Piece& p : line) {
      if (p.cls.empty()) {
        out += html_escape(p.text);
      } else {
        out += "<span class=\"";
        out += p.cls;
        out += "\">";
        out += html_escape(p.text);
        out += "</span>";
      }
    }
    out += "</span>";
  }
  return out;
}

const std::string& highlighted_source() {
  static const std::string html = highlight_cpp(source_code());
  return html;
}

unsigned long long uptime_seconds() {
  const auto delta = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - g_start);
  return static_cast<unsigned long long>(delta.count());
}

std::string format_duration(unsigned long long secs) {
  char buf[48];
  std::snprintf(buf, sizeof buf, "%llu:%02llu:%02llu",
                secs / 3600, (secs / 60) % 60, secs % 60);
  return std::string(buf);
}

const char kCss[] = R"CSS(
:root { color-scheme: dark; }
* { box-sizing: border-box; }
body { margin: 0; background: #0d1117; color: #c9d1d9;
       font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, sans-serif; }
header.bar { position: sticky; top: 0; z-index: 10; display: flex; flex-wrap: wrap;
             align-items: center; gap: .65rem; padding: .65rem 1.25rem;
             background: rgba(13, 17, 23, .93); border-bottom: 1px solid #21262d;
             font-size: .9rem; }
.dot { width: .6rem; height: .6rem; border-radius: 50%; background: #3fb950;
       box-shadow: 0 0 10px #3fb950; }
header.bar strong { color: #e6edf3; letter-spacing: .02em; }
header.bar code { background: #161b22; border: 1px solid #30363d; padding: .1rem .45rem;
                  border-radius: .35rem; color: #79c0ff; font-size: .85em; }
.sep { color: #484f58; }
.spacer { flex: 1; }
a { color: #58a6ff; text-decoration: none; }
a:hover { text-decoration: underline; }
main { padding: 1.5rem 1.25rem 0; }
h1 { margin: 0 0 .35rem; font-size: 1.35rem; color: #e6edf3; }
.lede { margin: 0 0 1.25rem; color: #8b949e; max-width: 62rem; line-height: 1.5; }
pre.code { margin: 0 0 2.5rem; padding: .9rem 0; background: #161b22;
           border: 1px solid #21262d; border-radius: .6rem; overflow: auto;
           counter-reset: line;
           font: 13px/1.55 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; }
pre.code .line { display: block; white-space: pre; padding-right: 1.25rem; }
pre.code .line::before { counter-increment: line; content: counter(line);
                         display: inline-block; width: 3.4em; margin-right: 1.1em;
                         text-align: right; color: #484f58; user-select: none; }
.cmt { color: #8b949e; font-style: italic; }
.str { color: #a5d6ff; }
.chr { color: #a5d6ff; }
.kw { color: #ff7b72; }
.ty { color: #ffa657; }
.num { color: #79c0ff; }
.pre { color: #d2a8ff; }
.fn { color: #d2a8ff; }
footer { padding: 0 1.25rem 2.5rem; color: #484f58; font-size: .8rem; }
.err { max-width: 42rem; margin: 12vh auto; text-align: center; padding: 0 1.25rem; }
.err h1 { font-size: 4rem; margin: 0 0 .35rem; color: #e6edf3; }
.err .quip { font-size: 1.15rem; color: #8b949e; margin: 0 0 1.5rem; }
.err .path { font-family: ui-monospace, Menlo, Consolas, monospace; color: #ffa657;
             background: #161b22; border: 1px solid #30363d; border-radius: .4rem;
             padding: .15rem .55rem; word-break: break-all; }
.err .links { display: flex; gap: 1.25rem; justify-content: center; margin-top: 1.5rem; }
)CSS";

std::string stats_json() {
  std::string out = "{";
  out += "\"uptime_seconds\":" + std::to_string(uptime_seconds());
  out += ",\"total_requests\":" + std::to_string(g_requests);
  out += ",\"bytes_served\":" + std::to_string(g_bytes_served);
  out += "}";
  return out;
}

std::string page_index() {
  std::string html;
  html.reserve(highlighted_source().size() + 4096);
  html += "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  html += "<title>quine_server.cpp &middot; self-hosted source</title>\n<style>\n";
  html += kCss;
  html += "</style>\n</head>\n<body>\n";
  html += "<header class=\"bar\"><span class=\"dot\"></span><strong>quine_server</strong>";
  html += "<span class=\"sep\">/</span>uptime <code>" + format_duration(uptime_seconds()) + "</code>";
  html += "<span class=\"sep\">/</span>requests <code>" + std::to_string(g_requests) + "</code>";
  html += "<span class=\"spacer\"></span><a href=\"/stats\">stats.json</a></header>\n";
  html += "<main>\n<h1>quine_server.cpp</h1>\n";
  html += "<p class=\"lede\">This page is served by the program it shows. The binary carries "
          "its own complete source text and reassembles it at runtime &mdash; no .cpp file is "
          "read from disk. The duplicate-looking block in the middle is the quine payload: the "
          "file as it exists on disk, placeholder and all.</p>\n";
  html += "<pre class=\"code\">";
  html += highlighted_source();
  html += "</pre>\n</main>\n<footer>tiny HTTP/1.1 server &middot; BSD sockets &middot; C++17 "
          "&middot; no dependencies</footer>\n</body>\n</html>\n";
  return html;
}

std::string page_not_found(const std::string& path) {
  static const char* quips[] = {
      "These are not the droids you are looking for.",
      "The resource you seek is in another castle.",
      "Have you tried turning it off and on again?",
      "A wild 404 appeared.",
      "Nothing to see here. Move along.",
      "This path was last seen heading for the exit."};
  const std::size_t count = sizeof(quips) / sizeof(quips[0]);
  const std::string quip = quips[g_requests % count];

  std::string html;
  html += "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  html += "<title>404 &middot; quine_server</title>\n<style>\n";
  html += kCss;
  html += "</style>\n</head>\n<body>\n";
  html += "<header class=\"bar\"><span class=\"dot\"></span><strong>quine_server</strong>";
  html += "<span class=\"sep\">/</span>uptime <code>" + format_duration(uptime_seconds()) + "</code>";
  html += "<span class=\"sep\">/</span>requests <code>" + std::to_string(g_requests) + "</code>";
  html += "<span class=\"spacer\"></span><a href=\"/stats\">stats.json</a></header>\n";
  html += "<div class=\"err\">\n<h1>404</h1>\n";
  html += "<p class=\"quip\">" + html_escape(quip) + "</p>\n";
  html += "<p>No route matches <span class=\"path\">" + html_escape(path) + "</span></p>\n";
  html += "<p class=\"links\"><a href=\"/\">home</a><a href=\"/stats\">stats</a></p>\n";
  html += "</div>\n</body>\n</html>\n";
  return html;
}

void send_all(int fd, const char* data, std::size_t len) {
  std::size_t sent = 0;
  while (sent < len) {
    const ssize_t w = ::send(fd, data + sent, len - sent, 0);
    if (w < 0) {
      if (errno == EINTR) continue;
      return;
    }
    if (w == 0) return;
    sent += static_cast<std::size_t>(w);
  }
}

void respond(int fd, const char* status, const char* type,
             const std::string& body, bool head_only) {
  std::string head = "HTTP/1.1 ";
  head += status;
  head += "\r\nServer: quine_server/1.0\r\nContent-Type: ";
  head += type;
  head += "\r\nContent-Length: " + std::to_string(body.size());
  head += "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
  send_all(fd, head.data(), head.size());
  if (!head_only) {
    send_all(fd, body.data(), body.size());
    g_bytes_served += body.size();
  }
}

void handle_client(int fd) {
  ++g_requests;

  std::string req;
  char buf[4096];
  while (req.size() < 65536) {
    const ssize_t r = ::recv(fd, buf, sizeof buf, 0);
    if (r < 0) {
      if (errno == EINTR) continue;
      break;
    }
    if (r == 0) break;
    req.append(buf, static_cast<std::size_t>(r));
    if (req.find("\r\n\r\n") != std::string::npos) break;
  }

  std::string method;
  std::string target;
  const std::size_t a = req.find(' ');
  if (a != std::string::npos) {
    method = req.substr(0, a);
    const std::size_t b = req.find(' ', a + 1);
    if (b != std::string::npos) target = req.substr(a + 1, b - a - 1);
  }
  const std::size_t q = target.find('?');
  const std::string path = (q == std::string::npos) ? target : target.substr(0, q);
  const bool head_only = (method == "HEAD");

  if (head_only || method == "GET") {
    if (path == "/") {
      respond(fd, "200 OK", "text/html; charset=utf-8", page_index(), head_only);
      return;
    }
    if (path == "/stats") {
      respond(fd, "200 OK", "application/json; charset=utf-8", stats_json(), head_only);
      return;
    }
  }
  respond(fd, "404 Not Found", "text/html; charset=utf-8", page_not_found(path), head_only);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  const long parsed = std::strtol(argv[1], nullptr, 10);
  if (parsed <= 0 || parsed > 65535) {
    std::fprintf(stderr, "invalid port: %s\n", argv[1]);
    return 1;
  }
  const std::uint16_t port = static_cast<std::uint16_t>(parsed);

  std::signal(SIGPIPE, SIG_IGN);

  const int listener = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    std::perror("socket");
    return 1;
  }
  int one = 1;
  ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (::bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) {
    std::perror("bind");
    return 1;
  }
  if (::listen(listener, 64) < 0) {
    std::perror("listen");
    return 1;
  }

  std::printf("quine_server listening on http://0.0.0.0:%u (%zu bytes of source embedded)\n",
              static_cast<unsigned>(port), source_code().size());
  std::fflush(stdout);

  for (;;) {
    const int fd = ::accept(listener, nullptr, nullptr);
    if (fd < 0) {
      if (errno == EINTR) continue;
      std::perror("accept");
      continue;
    }
    handle_client(fd);
    ::close(fd);
  }
})QSRC";

const std::string& source_code() {
  static const std::string text = [] {
    const std::string token = "@SRC@";
    const std::string raw = SRC_RAW;
    const std::string::size_type at = raw.find(token);
    if (at == std::string::npos) return raw;
    return raw.substr(0, at) + "R\"QSRC(" + raw + ")QSRC\"" + raw.substr(at + token.size());
  }();
  return text;
}

std::string html_escape(const std::string& in) {
  std::string out;
  out.reserve(in.size() + in.size() / 8 + 16);
  for (unsigned char c : in) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += static_cast<char>(c); break;
    }
  }
  return out;
}

bool is_ident_start(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_ident_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// A deliberately small C++ lexer.  It is not a compiler front end; it just
// colours comments, strings, chars, numbers, preprocessor lines, keywords,
// known type names and call-like identifiers well enough to look right.
std::string highlight_cpp(const std::string& src) {
  static const std::unordered_set<std::string> keywords = {
      "alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch",
      "char", "class", "const", "constexpr", "const_cast", "continue",
      "decltype", "default", "delete", "do", "double", "dynamic_cast", "else",
      "enum", "explicit", "extern", "false", "float", "for", "friend", "goto",
      "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
      "nullptr", "operator", "private", "protected", "public", "register",
      "reinterpret_cast", "return", "short", "signed", "sizeof", "static",
      "static_assert", "static_cast", "struct", "switch", "template", "this",
      "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
      "union", "unsigned", "using", "virtual", "void", "volatile", "while"};
  static const std::unordered_set<std::string> types = {
      "std", "string", "vector", "unordered_set", "size_t", "ssize_t",
      "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
      "int32_t", "int64_t", "sockaddr_in", "sockaddr", "socklen_t", "in_port_t",
      "in_addr", "Clock", "time_point", "steady_clock"};

  struct Piece {
    std::string text;
    std::string cls;
  };

  std::vector<std::vector<Piece>> lines(1);
  auto add = [&](char c, const char* cls) {
    std::vector<Piece>& line = lines.back();
    if (!line.empty() && line.back().cls == cls) line.back().text.push_back(c);
    else line.push_back(Piece{std::string(1, c), cls});
  };

  enum State { Normal, Block, LineC, Str, Chr, RawStr };
  State state = Normal;
  std::string raw_delim;
  bool at_line_start = true;
  const std::size_t n = src.size();
  std::size_t i = 0;

  while (i < n) {
    const char c = src[i];
    if (c == '\n') { lines.emplace_back(); at_line_start = true; ++i; continue; }

    if (state == LineC) { add(c, "cmt"); ++i; continue; }

    if (state == Block) {
      if (c == '*' && i + 1 < n && src[i + 1] == '/') {
        add('*', "cmt"); add('/', "cmt"); i += 2; state = Normal;
      } else {
        add(c, "cmt"); ++i;
      }
      continue;
    }

    if (state == Str) {
      add(c, "str"); ++i;
      if (c == '\\' && i < n) { add(src[i], "str"); ++i; }
      else if (c == '"') state = Normal;
      continue;
    }

    if (state == Chr) {
      add(c, "chr"); ++i;
      if (c == '\\' && i < n) { add(src[i], "chr"); ++i; }
      else if (c == '\'') state = Normal;
      continue;
    }

    if (state == RawStr) {
      const std::string closer = ")" + raw_delim + "\"";
      if (src.compare(i, closer.size(), closer) == 0) {
        for (char d : closer) add(d, "str");
        i += closer.size();
        state = Normal;
      } else {
        add(c, "str"); ++i;
      }
      continue;
    }

    if (at_line_start && (c == ' ' || c == '\t')) { add(c, ""); ++i; continue; }

    if (at_line_start && c == '#') {
      while (i < n && src[i] != '\n') { add(src[i], "pre"); ++i; }
      continue;
    }

    if (c == '/' && i + 1 < n && src[i + 1] == '/') {
      add('/', "cmt"); add('/', "cmt"); i += 2; state = LineC;
      at_line_start = false; continue;
    }
    if (c == '/' && i + 1 < n && src[i + 1] == '*') {
      add('/', "cmt"); add('*', "cmt"); i += 2; state = Block;
      at_line_start = false; continue;
    }
    if (c == 'R' && i + 1 < n && src[i + 1] == '"' &&
        (i == 0 || !is_ident_char(src[i - 1]))) {
      const std::size_t open = src.find('(', i + 2);
      if (open != std::string::npos && open - (i + 2) <= 24) {
        raw_delim = src.substr(i + 2, open - (i + 2));
        for (std::size_t k = i; k <= open; ++k) add(src[k], "str");
        i = open + 1; state = RawStr; at_line_start = false; continue;
      }
    }
    if (c == '"') { add(c, "str"); ++i; state = Str; at_line_start = false; continue; }
    if (c == '\'') { add(c, "chr"); ++i; state = Chr; at_line_start = false; continue; }

    if (std::isdigit(static_cast<unsigned char>(c))) {
      while (i < n && (std::isalnum(static_cast<unsigned char>(src[i])) ||
                       src[i] == '.' || src[i] == '\'' || src[i] == '_')) {
        add(src[i], "num"); ++i;
      }
      at_line_start = false; continue;
    }

    if (is_ident_start(c)) {
      std::size_t j = i;
      while (j < n && is_ident_char(src[j])) ++j;
      const std::string word = src.substr(i, j - i);
      const char* cls = "";
      if (keywords.count(word)) cls = "kw";
      else if (types.count(word)) cls = "ty";
      else if (j < n && src[j] == '(') cls = "fn";
      for (char d : word) add(d, cls);
      i = j; at_line_start = false; continue;
    }

    add(c, "");
    ++i;
    if (c != ' ' && c != '\t') at_line_start = false;
  }

  std::string out;
  out.reserve(src.size() * 2);
  bool first_line = true;
  for (const std::vector<Piece>& line : lines) {
    if (!first_line) out += '\n';
    first_line = false;
    out += "<span class=\"line\">";
    for (const Piece& p : line) {
      if (p.cls.empty()) {
        out += html_escape(p.text);
      } else {
        out += "<span class=\"";
        out += p.cls;
        out += "\">";
        out += html_escape(p.text);
        out += "</span>";
      }
    }
    out += "</span>";
  }
  return out;
}

const std::string& highlighted_source() {
  static const std::string html = highlight_cpp(source_code());
  return html;
}

unsigned long long uptime_seconds() {
  const auto delta = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - g_start);
  return static_cast<unsigned long long>(delta.count());
}

std::string format_duration(unsigned long long secs) {
  char buf[48];
  std::snprintf(buf, sizeof buf, "%llu:%02llu:%02llu",
                secs / 3600, (secs / 60) % 60, secs % 60);
  return std::string(buf);
}

const char kCss[] = R"CSS(
:root { color-scheme: dark; }
* { box-sizing: border-box; }
body { margin: 0; background: #0d1117; color: #c9d1d9;
       font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, sans-serif; }
header.bar { position: sticky; top: 0; z-index: 10; display: flex; flex-wrap: wrap;
             align-items: center; gap: .65rem; padding: .65rem 1.25rem;
             background: rgba(13, 17, 23, .93); border-bottom: 1px solid #21262d;
             font-size: .9rem; }
.dot { width: .6rem; height: .6rem; border-radius: 50%; background: #3fb950;
       box-shadow: 0 0 10px #3fb950; }
header.bar strong { color: #e6edf3; letter-spacing: .02em; }
header.bar code { background: #161b22; border: 1px solid #30363d; padding: .1rem .45rem;
                  border-radius: .35rem; color: #79c0ff; font-size: .85em; }
.sep { color: #484f58; }
.spacer { flex: 1; }
a { color: #58a6ff; text-decoration: none; }
a:hover { text-decoration: underline; }
main { padding: 1.5rem 1.25rem 0; }
h1 { margin: 0 0 .35rem; font-size: 1.35rem; color: #e6edf3; }
.lede { margin: 0 0 1.25rem; color: #8b949e; max-width: 62rem; line-height: 1.5; }
pre.code { margin: 0 0 2.5rem; padding: .9rem 0; background: #161b22;
           border: 1px solid #21262d; border-radius: .6rem; overflow: auto;
           counter-reset: line;
           font: 13px/1.55 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; }
pre.code .line { display: block; white-space: pre; padding-right: 1.25rem; }
pre.code .line::before { counter-increment: line; content: counter(line);
                         display: inline-block; width: 3.4em; margin-right: 1.1em;
                         text-align: right; color: #484f58; user-select: none; }
.cmt { color: #8b949e; font-style: italic; }
.str { color: #a5d6ff; }
.chr { color: #a5d6ff; }
.kw { color: #ff7b72; }
.ty { color: #ffa657; }
.num { color: #79c0ff; }
.pre { color: #d2a8ff; }
.fn { color: #d2a8ff; }
footer { padding: 0 1.25rem 2.5rem; color: #484f58; font-size: .8rem; }
.err { max-width: 42rem; margin: 12vh auto; text-align: center; padding: 0 1.25rem; }
.err h1 { font-size: 4rem; margin: 0 0 .35rem; color: #e6edf3; }
.err .quip { font-size: 1.15rem; color: #8b949e; margin: 0 0 1.5rem; }
.err .path { font-family: ui-monospace, Menlo, Consolas, monospace; color: #ffa657;
             background: #161b22; border: 1px solid #30363d; border-radius: .4rem;
             padding: .15rem .55rem; word-break: break-all; }
.err .links { display: flex; gap: 1.25rem; justify-content: center; margin-top: 1.5rem; }
)CSS";

std::string stats_json() {
  std::string out = "{";
  out += "\"uptime_seconds\":" + std::to_string(uptime_seconds());
  out += ",\"total_requests\":" + std::to_string(g_requests);
  out += ",\"bytes_served\":" + std::to_string(g_bytes_served);
  out += "}";
  return out;
}

std::string page_index() {
  std::string html;
  html.reserve(highlighted_source().size() + 4096);
  html += "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  html += "<title>quine_server.cpp &middot; self-hosted source</title>\n<style>\n";
  html += kCss;
  html += "</style>\n</head>\n<body>\n";
  html += "<header class=\"bar\"><span class=\"dot\"></span><strong>quine_server</strong>";
  html += "<span class=\"sep\">/</span>uptime <code>" + format_duration(uptime_seconds()) + "</code>";
  html += "<span class=\"sep\">/</span>requests <code>" + std::to_string(g_requests) + "</code>";
  html += "<span class=\"spacer\"></span><a href=\"/stats\">stats.json</a></header>\n";
  html += "<main>\n<h1>quine_server.cpp</h1>\n";
  html += "<p class=\"lede\">This page is served by the program it shows. The binary carries "
          "its own complete source text and reassembles it at runtime &mdash; no .cpp file is "
          "read from disk. The duplicate-looking block in the middle is the quine payload: the "
          "file as it exists on disk, placeholder and all.</p>\n";
  html += "<pre class=\"code\">";
  html += highlighted_source();
  html += "</pre>\n</main>\n<footer>tiny HTTP/1.1 server &middot; BSD sockets &middot; C++17 "
          "&middot; no dependencies</footer>\n</body>\n</html>\n";
  return html;
}

std::string page_not_found(const std::string& path) {
  static const char* quips[] = {
      "These are not the droids you are looking for.",
      "The resource you seek is in another castle.",
      "Have you tried turning it off and on again?",
      "A wild 404 appeared.",
      "Nothing to see here. Move along.",
      "This path was last seen heading for the exit."};
  const std::size_t count = sizeof(quips) / sizeof(quips[0]);
  const std::string quip = quips[g_requests % count];

  std::string html;
  html += "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  html += "<title>404 &middot; quine_server</title>\n<style>\n";
  html += kCss;
  html += "</style>\n</head>\n<body>\n";
  html += "<header class=\"bar\"><span class=\"dot\"></span><strong>quine_server</strong>";
  html += "<span class=\"sep\">/</span>uptime <code>" + format_duration(uptime_seconds()) + "</code>";
  html += "<span class=\"sep\">/</span>requests <code>" + std::to_string(g_requests) + "</code>";
  html += "<span class=\"spacer\"></span><a href=\"/stats\">stats.json</a></header>\n";
  html += "<div class=\"err\">\n<h1>404</h1>\n";
  html += "<p class=\"quip\">" + html_escape(quip) + "</p>\n";
  html += "<p>No route matches <span class=\"path\">" + html_escape(path) + "</span></p>\n";
  html += "<p class=\"links\"><a href=\"/\">home</a><a href=\"/stats\">stats</a></p>\n";
  html += "</div>\n</body>\n</html>\n";
  return html;
}

void send_all(int fd, const char* data, std::size_t len) {
  std::size_t sent = 0;
  while (sent < len) {
    const ssize_t w = ::send(fd, data + sent, len - sent, 0);
    if (w < 0) {
      if (errno == EINTR) continue;
      return;
    }
    if (w == 0) return;
    sent += static_cast<std::size_t>(w);
  }
}

void respond(int fd, const char* status, const char* type,
             const std::string& body, bool head_only) {
  std::string head = "HTTP/1.1 ";
  head += status;
  head += "\r\nServer: quine_server/1.0\r\nContent-Type: ";
  head += type;
  head += "\r\nContent-Length: " + std::to_string(body.size());
  head += "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
  send_all(fd, head.data(), head.size());
  if (!head_only) {
    send_all(fd, body.data(), body.size());
    g_bytes_served += body.size();
  }
}

void handle_client(int fd) {
  ++g_requests;

  std::string req;
  char buf[4096];
  while (req.size() < 65536) {
    const ssize_t r = ::recv(fd, buf, sizeof buf, 0);
    if (r < 0) {
      if (errno == EINTR) continue;
      break;
    }
    if (r == 0) break;
    req.append(buf, static_cast<std::size_t>(r));
    if (req.find("\r\n\r\n") != std::string::npos) break;
  }

  std::string method;
  std::string target;
  const std::size_t a = req.find(' ');
  if (a != std::string::npos) {
    method = req.substr(0, a);
    const std::size_t b = req.find(' ', a + 1);
    if (b != std::string::npos) target = req.substr(a + 1, b - a - 1);
  }
  const std::size_t q = target.find('?');
  const std::string path = (q == std::string::npos) ? target : target.substr(0, q);
  const bool head_only = (method == "HEAD");

  if (head_only || method == "GET") {
    if (path == "/") {
      respond(fd, "200 OK", "text/html; charset=utf-8", page_index(), head_only);
      return;
    }
    if (path == "/stats") {
      respond(fd, "200 OK", "application/json; charset=utf-8", stats_json(), head_only);
      return;
    }
  }
  respond(fd, "404 Not Found", "text/html; charset=utf-8", page_not_found(path), head_only);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return 1;
  }
  const long parsed = std::strtol(argv[1], nullptr, 10);
  if (parsed <= 0 || parsed > 65535) {
    std::fprintf(stderr, "invalid port: %s\n", argv[1]);
    return 1;
  }
  const std::uint16_t port = static_cast<std::uint16_t>(parsed);

  std::signal(SIGPIPE, SIG_IGN);

  const int listener = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    std::perror("socket");
    return 1;
  }
  int one = 1;
  ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (::bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) {
    std::perror("bind");
    return 1;
  }
  if (::listen(listener, 64) < 0) {
    std::perror("listen");
    return 1;
  }

  std::printf("quine_server listening on http://0.0.0.0:%u (%zu bytes of source embedded)\n",
              static_cast<unsigned>(port), source_code().size());
  std::fflush(stdout);

  for (;;) {
    const int fd = ::accept(listener, nullptr, nullptr);
    if (fd < 0) {
      if (errno == EINTR) continue;
      std::perror("accept");
      continue;
    }
    handle_client(fd);
    ::close(fd);
  }
}