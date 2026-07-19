#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

static time_t g_start = 0;
static int g_requests = 0;
static long long g_bytes = 0;

extern const char g_src[];
static std::string html_escape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '&') r += "&amp;";
        else r += c;
    }
    return r;
}

static std::string syntax_hl(const char* code) {
    std::stringstream ss;
    ss << "<pre style='background:#1e1e2e;color:#cdd6f4;font-family:Consolas,monospace;font-size:13px;line-height:1.6;padding:1.5rem;overflow:auto;border-radius:6px'>";
    size_t i = 0;
    std::string s(code);
    while (i < s.size()) {
        char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i+1] == '/') {
            size_t e = s.find('\n', i);
            if (e == std::string::npos) e = s.size();
            ss << "<span style='color:#6c7086'>" << html_escape(s.substr(i, e - i)) << "</span>";
            i = e;
        } else if (c == '#') {
            size_t e = s.find('\n', i);
            if (e == std::string::npos) e = s.size();
            ss << "<span style='color:#fab387'>" << html_escape(s.substr(i, e - i)) << "</span>";
            i = e;
        } else if (c == '"') {
            size_t e = i + 1;
            while (e < s.size() && !(s[e] == '"' && s[e-1] != '\\')) e++;
            std::string tok = s.substr(i, e - i + 1);
            ss << "<span style='color:#a6e3a1'>" << html_escape(tok) << "</span>";
            i = e + 1;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            size_t e = i + 1;
            while (e < s.size() && ((s[e] >= 'a' && s[e] <= 'z') || (s[e] >= 'A' && s[e] <= 'Z') ||
                       (s[e] >= '0' && s[e] <= '9') || s[e] == '_')) e++;
            std::string tok = s.substr(i, e - i);
            static const char* kws[] = {
                "auto","break","case","char","const","continue","default","do","double","else",
                "enum","extern","float","for","goto","if","int","long","register","return",
                "short","signed","sizeof","static","struct","switch","typedef","union","unsigned",
                "void","volatile","while","class","public","private","protected","virtual","override",
                "final","namespace","using","template","typename","try","catch","throw","new","delete",
                "nullptr","true","false","bool","std","string","cout","cin","cerr","endl","vector",
                "map","set","pair","make_pair","to_string","stoi","stol","stod","cerr<<","cout<<",
                "socket","bind","listen","accept","send","recv","connect","htons","htonl","ntohs","ntohl",
                "AF_INET","SOCK_STREAM","SOL_SOCKET","SO_REUSEADDR","INADDR_ANY","sockaddr_in","socklen_t"
            };
            bool is_kw = false;
            for (auto kw : kws) { if (tok == kw) { is_kw = true; break; } }
            if (is_kw) ss << "<span style='color:#cba6f7'>" << tok << "</span>";
            else ss << tok;
            i = e;
        } else {
            ss << html_escape(std::string(1, c));
            ++i;
        }
    }
    ss << "</pre>";
    return ss.str();
}

static std::string make_page() {
    std::string body = syntax_hl(g_src);
    std::stringstream ss;
    ss << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>C++17 Quine HTTP Server</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',system-ui,sans-serif;background:#11111b;color:#cdd6f4;min-height:100vh;display:flex;flex-direction:column}
header{background:#181825;padding:1.2rem 2rem;border-bottom:1px solid #313244;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:1rem}
h1{color:#89b4fa;font-size:1.35rem}
.st{display:flex;gap:1.5rem;font-size:0.88rem;flex-wrap:wrap}
.r{display:flex;align-items:center;gap:0.4rem}
.l{color:#f38ba8;font-weight:600}
main{padding:1.5rem 2rem;flex:1}
footer{background:#181825;padding:0.7rem 2rem;font-size:0.78rem;color:#6c7086;border-top:1px solid #313244}
</style>
</head>
<body>
<header>
<h1>C++17 HTTP Quine Server</h1>
<div class="st">
<div class="r"><span class="l">Uptime:</span><span id="u">0s</span></div>
<div class="r"><span class="l">Requests:</span>)" << g_requests << R"(</div>
<div class="r"><span class="l">Bytes:</span>)" << g_bytes << R"(</div>
</div>
</header>
<main>
)" << body << R"(
</main>
<footer>g++ -std=c++17 -O2 quine_server.cpp -o quine_server</footer>
<script>
let s=)" << (time(nullptr) - g_start) << R"(;
function f(n){if(n>=86400)return Math.floor(n/86400)+"d "+Math.floor((n%86400)/3600)+"h "+Math.floor((n%3600)/60)+"m"+(n%60)+"s";if(n>=3600)return Math.floor(n/3600)+"h "+Math.floor((n%3600)/60)+"m"+(n%60)+"s";if(n>=60)return Math.floor(n/60)+"m"+(n%60)+"s";return n+"s"}
document.getElementById('u').textContent=f(s);
setInterval(()=>{s++;document.getElementById('u').textContent=f(s)},1000);
</script>
</body>
</html>)";
    return ss.str();
}

static std::string make_stats() {
    int up = static_cast<int>(time(nullptr) - g_start);
    std::stringstream ss;
    ss << "{";
    ss << "\"uptime_seconds\":" << up << ",";
    ss << "\"total_requests\":" << g_requests << ",";
    ss << "\"total_bytes_served\":" << g_bytes;
    ss << "}";
    return ss.str();
}

static std::string make_404() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>404 - Not Found</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;background:#11111b;font-family:'Segoe UI',system-ui,sans-serif;color:#cdd6f4;text-align:center;padding:2rem}
.n{font-size:7rem;font-weight:700;color:#f38ba8;text-shadow:0 0 50px rgba(243,139,168,.4);animation:g 2s ease-in-out infinite}
@keyframes g{0%,100%{text-shadow:0 0 20px rgba(243,139,168,.3)}50%{text-shadow:0 0 70px rgba(243,139,168,.7)}}
.m{font-size:1.4rem;margin:1rem 0;color:#9399b2}
.h{font-size:.88rem;color:#585b70;margin-top:1.5rem;line-height:1.8}
a{color:#89b4fa;text-decoration:none}a:hover{text-decoration:underline}
</style>
</head>
<body>
<div class="n">404</div>
<div class="m">This path leads nowhere</div>
<div class="h">Try <a href="/">/</a> for the source code<br>or <a href="/stats">/stats</a> for JSON stats</div>
</body>
</html>)";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <port>\n"; return 1; }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) { std::cerr << "Invalid port\n"; return 1; }

    g_start = time(nullptr);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) { perror("bind"); close(fd); return 1; }
    if (listen(fd, 10) < 0) { perror("listen"); close(fd); return 1; }
    std::cout << "Listening on port " << port << "\n";

    char buf[8192];
    while (true) {
        struct sockaddr_in ca; socklen_t cl = sizeof(ca);
        int cd = accept(fd, (struct sockaddr*)&ca, &cl);
        if (cd < 0) { perror("accept"); continue; }

        memset(buf, 0, sizeof(buf));
        ssize_t rn = recv(cd, buf, sizeof(buf) - 1, 0);
        if (rn > 0) {
            buf[rn] = '\0';
            std::string req(buf);
            std::string path;
            size_t sp1 = req.find(' ');
            size_t sp2 = req.find(' ', sp1 + 1);
            if (sp1 != std::string::npos && sp2 != std::string::npos) {
                path = req.substr(sp1 + 1, sp2 - sp1 - 1);
            }

            ++g_requests;
            std::string body, status, ctype;

            if (path == "/" || path == "/index.html") {
                status = "200 OK"; ctype = "text/html; charset=utf-8";
                body = make_page();
            } else if (path == "/stats") {
                status = "200 OK"; ctype = "application/json";
                body = make_stats();
            } else {
                status = "404 Not Found"; ctype = "text/html; charset=utf-8";
                body = make_404();
            }

            std::ostringstream h;
            h << "HTTP/1.1 " << status << "\r\n";
            h << "Content-Type: " << ctype << "\r\n";
            h << "Content-Length: " << body.size() << "\r\n";
            h << "Connection: close\r\n";
            h << "\r\n";
            std::string hdr = h.str();
            g_bytes += static_cast<long long>(hdr.size() + body.size());
            send(cd, hdr.c_str(), hdr.size(), 0);
            send(cd, body.c_str(), body.size(), 0);
        }
        close(cd);
    }
    close(fd);
    return 0;
}

const char g_src[] = R"CODE(#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

static time_t g_start = 0;
static int g_requests = 0;
static long long g_bytes = 0;

static std::string html_escape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '&') r += "&amp;";
        else r += c;
    }
    return r;
}

static std::string syntax_hl(const char* code) {
    std::stringstream ss;
    ss << "<pre style='background:#1e1e2e;color:#cdd6f4;font-family:Consolas,monospace;font-size:13px;line-height:1.6;padding:1.5rem;overflow:auto;border-radius:6px'>";
    size_t i = 0;
    std::string s(code);
    while (i < s.size()) {
        char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i+1] == '/') {
            size_t e = s.find('\n', i);
            if (e == std::string::npos) e = s.size();
            ss << "<span style='color:#6c7086'>" << html_escape(s.substr(i, e - i)) << "</span>";
            i = e;
        } else if (c == '#') {
            size_t e = s.find('\n', i);
            if (e == std::string::npos) e = s.size();
            ss << "<span style='color:#fab387'>" << html_escape(s.substr(i, e - i)) << "</span>";
            i = e;
        } else if (c == '"') {
            size_t e = i + 1;
            while (e < s.size() && !(s[e] == '"' && s[e-1] != '\\')) e++;
            std::string tok = s.substr(i, e - i + 1);
            ss << "<span style='color:#a6e3a1'>" << html_escape(tok) << "</span>";
            i = e + 1;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            size_t e = i + 1;
            while (e < s.size() && ((s[e] >= 'a' && s[e] <= 'z') || (s[e] >= 'A' && s[e] <= 'Z') ||
                       (s[e] >= '0' && s[e] <= '9') || s[e] == '_')) e++;
            std::string tok = s.substr(i, e - i);
            static const char* kws[] = {
                "auto","break","case","char","const","continue","default","do","double","else",
                "enum","extern","float","for","goto","if","int","long","register","return",
                "short","signed","sizeof","static","struct","switch","typedef","union","unsigned",
                "void","volatile","while","class","public","private","protected","virtual","override",
                "final","namespace","using","template","typename","try","catch","throw","new","delete",
                "nullptr","true","false","bool","std","string","cout","cin","cerr","endl","vector",
                "map","set","pair","make_pair","to_string","stoi","stol","stod","cerr<<","cout<<",
                "socket","bind","listen","accept","send","recv","connect","htons","htonl","ntohs","ntohl",
                "AF_INET","SOCK_STREAM","SOL_SOCKET","SO_REUSEADDR","INADDR_ANY","sockaddr_in","socklen_t"
            };
            bool is_kw = false;
            for (auto kw : kws) { if (tok == kw) { is_kw = true; break; } }
            if (is_kw) ss << "<span style='color:#cba6f7'>" << tok << "</span>";
            else ss << tok;
            i = e;
        } else {
            ss << html_escape(std::string(1, c));
            ++i;
        }
    }
    ss << "</pre>";
    return ss.str();
}

static std::string make_page() {
    std::string body = syntax_hl(g_src);
    std::stringstream ss;
    ss << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>C++17 Quine HTTP Server</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',system-ui,sans-serif;background:#11111b;color:#cdd6f4;min-height:100vh;display:flex;flex-direction:column}
header{background:#181825;padding:1.2rem 2rem;border-bottom:1px solid #313244;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:1rem}
h1{color:#89b4fa;font-size:1.35rem}
.st{display:flex;gap:1.5rem;font-size:0.88rem;flex-wrap:wrap}
.r{display:flex;align-items:center;gap:0.4rem}
.l{color:#f38ba8;font-weight:600}
main{padding:1.5rem 2rem;flex:1}
footer{background:#181825;padding:0.7rem 2rem;font-size:0.78rem;color:#6c7086;border-top:1px solid #313244}
</style>
</head>
<body>
<header>
<h1>C++17 HTTP Quine Server</h1>
<div class="st">
<div class="r"><span class="l">Uptime:</span><span id="u">0s</span></div>
<div class="r"><span class="l">Requests:</span>)" << g_requests << R"(</div>
<div class="r"><span class="l">Bytes:</span>)" << g_bytes << R"(</div>
</div>
</header>
<main>
)" << body << R"(
</main>
<footer>g++ -std=c++17 -O2 quine_server.cpp -o quine_server</footer>
<script>
let s=)" << (time(nullptr) - g_start) << R"(;
function f(n){if(n>=86400)return Math.floor(n/86400)+"d "+Math.floor((n%86400)/3600)+"h "+Math.floor((n%3600)/60)+"m"+(n%60)+"s";if(n>=3600)return Math.floor(n/3600)+"h "+Math.floor((n%3600)/60)+"m"+(n%60)+"s";if(n>=60)return Math.floor(n/60)+"m"+(n%60)+"s";return n+"s"}
document.getElementById('u').textContent=f(s);
setInterval(()=>{s++;document.getElementById('u').textContent=f(s)},1000);
</script>
</body>
</html>)";
    return ss.str();
}

static std::string make_stats() {
    int up = static_cast<int>(time(nullptr) - g_start);
    std::stringstream ss;
    ss << "{";
    ss << "\"uptime_seconds\":" << up << ",";
    ss << "\"total_requests\":" << g_requests << ",";
    ss << "\"total_bytes_served\":" << g_bytes;
    ss << "}";
    return ss.str();
}

static std::string make_404() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>404 - Not Found</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;background:#11111b;font-family:'Segoe UI',system-ui,sans-serif;color:#cdd6f4;text-align:center;padding:2rem}
.n{font-size:7rem;font-weight:700;color:#f38ba8;text-shadow:0 0 50px rgba(243,139,168,.4);animation:g 2s ease-in-out infinite}
@keyframes g{0%,100%{text-shadow:0 0 20px rgba(243,139,168,.3)}50%{text-shadow:0 0 70px rgba(243,139,168,.7)}}
.m{font-size:1.4rem;margin:1rem 0;color:#9399b2}
.h{font-size:.88rem;color:#585b70;margin-top:1.5rem;line-height:1.8}
a{color:#89b4fa;text-decoration:none}a:hover{text-decoration:underline}
</style>
</head>
<body>
<div class="n">404</div>
<div class="m">This path leads nowhere</div>
<div class="h">Try <a href="/">/</a> for the source code<br>or <a href="/stats">/stats</a> for JSON stats</div>
</body>
</html>)";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <port>\n"; return 1; }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) { std::cerr << "Invalid port\n"; return 1; }

    g_start = time(nullptr);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) { perror("bind"); close(fd); return 1; }
    if (listen(fd, 10) < 0) { perror("listen"); close(fd); return 1; }
    std::cout << "Listening on port " << port << "\n";

    char buf[8192];
    while (true) {
        struct sockaddr_in ca; socklen_t cl = sizeof(ca);
        int cd = accept(fd, (struct sockaddr*)&ca, &cl);
        if (cd < 0) { perror("accept"); continue; }

        memset(buf, 0, sizeof(buf));
        ssize_t rn = recv(cd, buf, sizeof(buf) - 1, 0);
        if (rn > 0) {
            buf[rn] = '\0';
            std::string req(buf);
            std::string path;
            size_t sp1 = req.find(' ');
            size_t sp2 = req.find(' ', sp1 + 1);
            if (sp1 != std::string::npos && sp2 != std::string::npos) {
                path = req.substr(sp1 + 1, sp2 - sp1 - 1);
            }

            ++g_requests;
            std::string body, status, ctype;

            if (path == "/" || path == "/index.html") {
                status = "200 OK"; ctype = "text/html; charset=utf-8";
                body = make_page();
            } else if (path == "/stats") {
                status = "200 OK"; ctype = "application/json";
                body = make_stats();
            } else {
                status = "404 Not Found"; ctype = "text/html; charset=utf-8";
                body = make_404();
            }

            std::ostringstream h;
            h << "HTTP/1.1 " << status << "\r\n";
            h << "Content-Type: " << ctype << "\r\n";
            h << "Content-Length: " << body.size() << "\r\n";
            h << "Connection: close\r\n";
            h << "\r\n";
            std::string hdr = h.str();
            g_bytes += static_cast<long long>(hdr.size() + body.size());
            send(cd, hdr.c_str(), hdr.size(), 0);
            send(cd, body.c_str(), body.size(), 0);
        }
        close(cd);
    }
    close(fd);
    return 0;
}
)CODE";