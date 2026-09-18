#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static long gStart = 0;
static long gReq = 0;
static long gBytes = 0;

static std::string expandSrc() {
    const char *s = "#include <cstdio>%2$c#include <cctype>%2$c#include <cstring>%2$c#include <string>%2$c#include <vector>%2$c#include <ctime>%2$c#include <cstdlib>%2$c#include <unistd.h>%2$c#include <arpa/inet.h>%2$c#include <netinet/in.h>%2$c#include <sys/socket.h>%2$c%2$cstatic long gStart = 0;%2$cstatic long gReq = 0;%2$cstatic long gBytes = 0;%2$c%2$cstatic std::string expandSrc() {%2$c    const char *s = %3$c%1$s%3$c;%2$c    long need = std::snprintf(0, 0, s, s, 10, 34);%2$c    std::string out((size_t)need, (char)0);%2$c    std::snprintf(&out[0], (size_t)need + 1, s, s, 10, 34);%2$c    return out;%2$c}%2$c%2$cstatic void eol(std::string& t) {%2$c    t += (char)13;%2$c    t += (char)10;%2$c}%2$c%2$cstatic const char* KWS[] = {%2$c    %3$calignas%3$c, %3$calignof%3$c, %3$cand%3$c, %3$casm%3$c, %3$cauto%3$c, %3$cbitand%3$c, %3$cbitor%3$c, %3$cbool%3$c,%2$c    %3$cbreak%3$c, %3$ccase%3$c, %3$ccatch%3$c, %3$cchar%3$c, %3$cclass%3$c, %3$ccompl%3$c, %3$cconcept%3$c, %3$cconst%3$c,%2$c    %3$cconstexpr%3$c, %3$cconst_cast%3$c, %3$ccontinue%3$c, %3$cdecltype%3$c, %3$cdefault%3$c, %3$cdelete%3$c,%2$c    %3$cdo%3$c, %3$cdouble%3$c, %3$cdynamic_cast%3$c, %3$celse%3$c, %3$cenum%3$c, %3$cexplicit%3$c, %3$cexport%3$c,%2$c    %3$cextern%3$c, %3$cfalse%3$c, %3$cfloat%3$c, %3$cfor%3$c, %3$cfriend%3$c, %3$cgoto%3$c, %3$cif%3$c, %3$cinline%3$c,%2$c    %3$cint%3$c, %3$clong%3$c, %3$cmutable%3$c, %3$cnamespace%3$c, %3$cnew%3$c, %3$cnoexcept%3$c, %3$cnot%3$c,%2$c    %3$cnullptr%3$c, %3$coperator%3$c, %3$cor%3$c, %3$cprivate%3$c, %3$cprotected%3$c, %3$cpublic%3$c, %3$cregister%3$c,%2$c    %3$creinterpret_cast%3$c, %3$creturn%3$c, %3$cshort%3$c, %3$csigned%3$c, %3$csizeof%3$c, %3$cstatic%3$c,%2$c    %3$cstatic_cast%3$c, %3$cstruct%3$c, %3$cswitch%3$c, %3$ctemplate%3$c, %3$cthis%3$c, %3$cthrow%3$c, %3$ctrue%3$c,%2$c    %3$ctry%3$c, %3$ctypedef%3$c, %3$ctypeid%3$c, %3$ctypename%3$c, %3$cunion%3$c, %3$cunsigned%3$c, %3$cusing%3$c,%2$c    %3$cvirtual%3$c, %3$cvoid%3$c, %3$cvolatile%3$c, %3$cwhile%3$c, %3$cxor%3$c, %3$cnot_eq%3$c, %3$cor_eq%3$c,%2$c    %3$cstd%3$c, %3$cstring%3$c, %3$cvector%3$c, %3$csize_t%3$c, %3$cssize_t%3$c, %3$cmain%3$c, %3$cinclude%3$c, %3$cNULL%3$c%2$c};%2$cstatic const int kwN = (int)(sizeof(KWS) / sizeof(KWS[0]));%2$c%2$cstatic bool isKw(const std::string& w) {%2$c    for (int i = 0; i < kwN; ++i) {%2$c        if (w == KWS[i]) return true;%2$c    }%2$c    return false;%2$c}%2$c%2$cstatic std::string esc(const std::string& t) {%2$c    std::string o;%2$c    for (size_t i = 0; i < t.size(); ++i) {%2$c        char c = t[i];%2$c        if (c == '&') o += %3$c&amp;%3$c;%2$c        else if (c == '<') o += %3$c&lt;%3$c;%2$c        else if (c == '>') o += %3$c&gt;%3$c;%2$c        else o += c;%2$c    }%2$c    return o;%2$c}%2$c%2$cstatic std::string hl(const std::string& line) {%2$c    std::string out;%2$c    size_t i = 0;%2$c    size_t n = line.size();%2$c    while (i < n) {%2$c        char c = line[i];%2$c        if (c == '/' && i + 1 < n && line[i + 1] == '/') {%2$c            out += %3$c<span class=c>%3$c + esc(line.substr(i)) + %3$c</span>%3$c;%2$c            break;%2$c        } else if (c == '%3$c') {%2$c            size_t j = i + 1;%2$c            while (j < n && line[j] != '%3$c') ++j;%2$c            if (j < n) ++j;%2$c            out += %3$c<span class=s>%3$c + esc(line.substr(i, j - i)) + %3$c</span>%3$c;%2$c            i = j;%2$c        } else if (c == 39) {%2$c            size_t j = i + 1;%2$c            while (j < n && line[j] != 39) ++j;%2$c            if (j < n) ++j;%2$c            out += %3$c<span class=s>%3$c + esc(line.substr(i, j - i)) + %3$c</span>%3$c;%2$c            i = j;%2$c        } else if (c == '#') {%2$c            out += %3$c<span class=p>%3$c + esc(line.substr(i)) + %3$c</span>%3$c;%2$c            break;%2$c        } else if (std::isalpha((unsigned char)c) || c == '_') {%2$c            size_t j = i;%2$c            while (j < n) {%2$c                unsigned char d = (unsigned char)line[j];%2$c                if (std::isalnum(d) || line[j] == '_') ++j; else break;%2$c            }%2$c            std::string w = line.substr(i, j - i);%2$c            if (isKw(w)) out += %3$c<span class=k>%3$c + w + %3$c</span>%3$c;%2$c            else out += w;%2$c            i = j;%2$c        } else if (std::isdigit((unsigned char)c)) {%2$c            size_t j = i;%2$c            while (j < n) {%2$c                unsigned char d = (unsigned char)line[j];%2$c                if (std::isxdigit(d) || line[j] == '.' || line[j] == 'x') ++j; else break;%2$c            }%2$c            out += %3$c<span class=n>%3$c + line.substr(i, j - i) + %3$c</span>%3$c;%2$c            i = j;%2$c        } else {%2$c            if (c == '<') out += %3$c&lt;%3$c;%2$c            else if (c == '>') out += %3$c&gt;%3$c;%2$c            else if (c == '&') out += %3$c&amp;%3$c;%2$c            else out += c;%2$c            ++i;%2$c        }%2$c    }%2$c    return out;%2$c}%2$c%2$cstatic std::string hlsrc(const std::string& src) {%2$c    std::string out;%2$c    size_t start = 0;%2$c    long num = 1;%2$c    while (start <= src.size()) {%2$c        size_t e = src.find((char)10, start);%2$c        std::string line;%2$c        if (e == std::string::npos) {%2$c            line = src.substr(start);%2$c            start = src.size() + 1;%2$c        } else {%2$c            line = src.substr(start, e - start);%2$c            start = e + 1;%2$c        }%2$c        out += %3$c<div class=L><span class=ln>%3$c;%2$c        out += std::to_string(num);%2$c        out += %3$c</span>%3$c;%2$c        out += hl(line);%2$c        out += %3$c</div>%3$c;%2$c        ++num;%2$c    }%2$c    return out;%2$c}%2$c%2$cstatic std::string ut() {%2$c    return std::to_string((long)time(0) - gStart);%2$c}%2$c%2$cstatic void rep(std::string& t, const std::string& a, const std::string& b) {%2$c    size_t p = 0;%2$c    for (;;) {%2$c        p = t.find(a, p);%2$c        if (p == std::string::npos) return;%2$c        t.replace(p, a.size(), b);%2$c        p += b.size();%2$c    }%2$c}%2$c%2$cstatic std::string q2(const char* s) {%2$c    std::string t;%2$c    t += (char)34;%2$c    t += s;%2$c    t += (char)34;%2$c    return t;%2$c}%2$c%2$cstatic std::string pageRoot() {%2$c    std::string src = expandSrc();%2$c    std::string body =%2$c        %3$c<!DOCTYPE html>%3$c%2$c        %3$c<html><head><meta charset=utf-8><title>quine_server</title><style>%3$c%2$c        %3$cbody{margin:0;background:#0d1117;color:#c9d1d9;%3$c%2$c        %3$cfont-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;}%3$c%2$c        %3$c.bar{position:sticky;top:0;background:#161b22;%3$c%2$c        %3$cborder-bottom:1px solid #30363d;padding:14px 26px;%3$c%2$c        %3$cdisplay:flex;gap:26px;align-items:baseline;flex-wrap:wrap;}%3$c%2$c        %3$c.t{color:#58a6ff;font-weight:bold;font-size:19px;}%3$c%2$c        %3$c.b{color:#8b949e;font-size:13px;}%3$c%2$c        %3$cmain{padding:26px;margin:0 auto;max-width:1080px;}%3$c%2$c        %3$c.L{white-space:pre-wrap;overflow-wrap:anywhere;line-height:1.5;}%3$c%2$c        %3$c.ln{color:#4b5563;display:inline-block;width:3em;%3$c%2$c        %3$ctext-align:right;margin-right:1em;user-select:none;}%3$c%2$c        %3$c.k{color:#ff7b72}.s{color:#a5d6ff}.c{color:#8b949e}%3$c%2$c        %3$c.n{color:#79c0ff}.p{color:#d2a8ff}%3$c%2$c        %3$c</style></head><body>%3$c%2$c        %3$c<header class=bar>%3$c%2$c        %3$c<span class=t>quine_server.cpp</span>%3$c%2$c        %3$c<span class=b>this page IS its own source - served from memory</span>%3$c%2$c        %3$c<span class=b>uptime UPTIME_TAG s</span>%3$c%2$c        %3$c<span class=b>requests REQ_TAG</span>%3$c%2$c        %3$c<span class=b>bytes BYTE_TAG</span>%3$c%2$c        %3$c</header><main>HILITE_TAG</main></body></html>%3$c;%2$c    rep(body, %3$cUPTIME_TAG%3$c, ut());%2$c    rep(body, %3$cREQ_TAG%3$c, std::to_string(gReq));%2$c    rep(body, %3$cBYTE_TAG%3$c, std::to_string(gBytes));%2$c    rep(body, %3$cHILITE_TAG%3$c, hlsrc(src));%2$c    return body;%2$c}%2$c%2$cstatic std::string pageStats() {%2$c    std::string j;%2$c    j += (char)123;%2$c    j += q2(%3$cuptime%3$c);%2$c    j += (char)58;%2$c    j += ut();%2$c    j += (char)44;%2$c    j += q2(%3$crequests%3$c);%2$c    j += (char)58;%2$c    j += std::to_string(gReq);%2$c    j += (char)44;%2$c    j += q2(%3$cbytes%3$c);%2$c    j += (char)58;%2$c    j += std::to_string(gBytes);%2$c    j += (char)125;%2$c    return j;%2$c}%2$c%2$cstatic std::string page404(const std::string& path) {%2$c    std::string body = R%3$cHTML(%2$c<!DOCTYPE html><html><head><meta charset=utf-8><title>404</title><style>%2$cbody{margin:0;background:#0d1117;color:#c9d1d9;font-family:monospace;%2$cdisplay:flex;min-height:100vh;align-items:center;justify-content:center}%2$c.box{text-align:center}.big{font-size:72px;color:#ff7b72}%2$ch1{color:#ff7b72;font-size:22px}a{color:#58a6ff;text-decoration:none}%2$c.art{color:#8b949e;font-size:16px;line-height:1.25}%2$c</style></head><body><div class=box>%2$c<div class=big>404</div><h1>lost in the void</h1>%2$c<p>no such page: )HTML%3$c;%2$c    body += esc(path);%2$c    body += R%3$cHTML(</p><div class=art><pre>%2$c    ________%2$c   |  _  _  |%2$c   | |_||_| |%2$c   |   __   |%2$c   |__|  |__|%2$c</pre></div><p><a href=/>go home</a></p>%2$c</div></body></html>)HTML%3$c;%2$c    return body;%2$c}%2$c%2$cstatic void sendAll(int fd, const std::string& resp) {%2$c    size_t off = 0;%2$c    while (off < resp.size()) {%2$c        ssize_t w = ::write(fd, resp.data() + off, resp.size() - off);%2$c        if (w <= 0) break;%2$c        off += (size_t)w;%2$c    }%2$c    gBytes += (long)resp.size();%2$c}%2$c%2$cint main(int argc, char** argv) {%2$c    int port = 8080;%2$c    if (argc > 1) {%2$c        int v = 0;%2$c        const char* q = argv[1];%2$c        bool bad = (*q == 0);%2$c        while (*q >= '0' && *q <= '9') {%2$c            v = v * 10 + (*q - '0');%2$c            ++q;%2$c        }%2$c        if (!bad && *q == 0 && v > 0) port = v;%2$c    }%2$c    gStart = (long)time(0);%2$c    int fd = ::socket(AF_INET, SOCK_STREAM, 0);%2$c    if (fd < 0) return 1;%2$c    int one = 1;%2$c    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));%2$c    sockaddr_in sa;%2$c    std::memset(&sa, 0, sizeof(sa));%2$c    sa.sin_family = AF_INET;%2$c    sa.sin_addr.s_addr = htonl(INADDR_ANY);%2$c    sa.sin_port = htons((unsigned short)port);%2$c    if (::bind(fd, (sockaddr*)&sa, sizeof(sa)) < 0) return 2;%2$c    if (::listen(fd, 32) < 0) return 3;%2$c    std::string endh;%2$c    eol(endh);%2$c    eol(endh);%2$c    for (;;) {%2$c        sockaddr_in cl;%2$c        socklen_t cll = sizeof(cl);%2$c        int c = ::accept(fd, (sockaddr*)&cl, &cll);%2$c        if (c < 0) continue;%2$c        char buf[8192];%2$c        std::string req;%2$c        size_t got = 0;%2$c        for (;;) {%2$c            ssize_t r = ::read(c, buf + got, sizeof(buf) - got);%2$c            if (r <= 0) break;%2$c            got += (size_t)r;%2$c            req.assign(buf, got);%2$c            if (req.find(endh) != std::string::npos) break;%2$c            if (got >= sizeof(buf)) break;%2$c        }%2$c        gReq += 1;%2$c        std::string path = %3$c/%3$c;%2$c        size_t p1 = req.find(' ');%2$c        if (p1 != std::string::npos) {%2$c            size_t p2 = req.find(' ', p1 + 1);%2$c            if (p2 != std::string::npos) {%2$c                path = req.substr(p1 + 1, p2 - p1 - 1);%2$c            }%2$c        }%2$c        std::string body = page404(path);%2$c        std::string ctype = %3$ctext/html; charset=utf-8%3$c;%2$c        int status = 404;%2$c        if (path == %3$c/%3$c) {%2$c            body = pageRoot();%2$c            status = 200;%2$c        } else if (path == %3$c/stats%3$c) {%2$c            body = pageStats();%2$c            ctype = %3$capplication/json; charset=utf-8%3$c;%2$c            status = 200;%2$c        }%2$c        std::string head;%2$c        head += %3$cHTTP/1.1 %3$c;%2$c        head += std::to_string(status);%2$c        head += %3$c %3$c;%2$c        if (status == 200) head += %3$cOK%3$c;%2$c        else head += %3$cNot Found%3$c;%2$c        eol(head);%2$c        head += %3$cContent-Type: %3$c;%2$c        head += ctype;%2$c        eol(head);%2$c        head += %3$cContent-Length: %3$c;%2$c        head += std::to_string((long)body.size());%2$c        eol(head);%2$c        head += %3$cConnection: close%3$c;%2$c        eol(head);%2$c        head += %3$cServer: quine%3$c;%2$c        eol(head);%2$c        eol(head);%2$c        sendAll(c, head + body);%2$c        ::close(c);%2$c    }%2$c    return 0;%2$c}";
    long need = std::snprintf(0, 0, s, s, 10, 34);
    std::string out((size_t)need, (char)0);
    std::snprintf(&out[0], (size_t)need + 1, s, s, 10, 34);
    return out;
}

static void eol(std::string& t) {
    t += (char)13;
    t += (char)10;
}

static const char* KWS[] = {
    "alignas", "alignof", "and", "asm", "auto", "bitand", "bitor", "bool",
    "break", "case", "catch", "char", "class", "compl", "concept", "const",
    "constexpr", "const_cast", "continue", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
    "extern", "false", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept", "not",
    "nullptr", "operator", "or", "private", "protected", "public", "register",
    "reinterpret_cast", "return", "short", "signed", "sizeof", "static",
    "static_cast", "struct", "switch", "template", "this", "throw", "true",
    "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "while", "xor", "not_eq", "or_eq",
    "std", "string", "vector", "size_t", "ssize_t", "main", "include", "NULL"
};
static const int kwN = (int)(sizeof(KWS) / sizeof(KWS[0]));

static bool isKw(const std::string& w) {
    for (int i = 0; i < kwN; ++i) {
        if (w == KWS[i]) return true;
    }
    return false;
}

static std::string esc(const std::string& t) {
    std::string o;
    for (size_t i = 0; i < t.size(); ++i) {
        char c = t[i];
        if (c == '&') o += "&amp;";
        else if (c == '<') o += "&lt;";
        else if (c == '>') o += "&gt;";
        else o += c;
    }
    return o;
}

static std::string hl(const std::string& line) {
    std::string out;
    size_t i = 0;
    size_t n = line.size();
    while (i < n) {
        char c = line[i];
        if (c == '/' && i + 1 < n && line[i + 1] == '/') {
            out += "<span class=c>" + esc(line.substr(i)) + "</span>";
            break;
        } else if (c == '"') {
            size_t j = i + 1;
            while (j < n && line[j] != '"') ++j;
            if (j < n) ++j;
            out += "<span class=s>" + esc(line.substr(i, j - i)) + "</span>";
            i = j;
        } else if (c == 39) {
            size_t j = i + 1;
            while (j < n && line[j] != 39) ++j;
            if (j < n) ++j;
            out += "<span class=s>" + esc(line.substr(i, j - i)) + "</span>";
            i = j;
        } else if (c == '#') {
            out += "<span class=p>" + esc(line.substr(i)) + "</span>";
            break;
        } else if (std::isalpha((unsigned char)c) || c == '_') {
            size_t j = i;
            while (j < n) {
                unsigned char d = (unsigned char)line[j];
                if (std::isalnum(d) || line[j] == '_') ++j; else break;
            }
            std::string w = line.substr(i, j - i);
            if (isKw(w)) out += "<span class=k>" + w + "</span>";
            else out += w;
            i = j;
        } else if (std::isdigit((unsigned char)c)) {
            size_t j = i;
            while (j < n) {
                unsigned char d = (unsigned char)line[j];
                if (std::isxdigit(d) || line[j] == '.' || line[j] == 'x') ++j; else break;
            }
            out += "<span class=n>" + line.substr(i, j - i) + "</span>";
            i = j;
        } else {
            if (c == '<') out += "&lt;";
            else if (c == '>') out += "&gt;";
            else if (c == '&') out += "&amp;";
            else out += c;
            ++i;
        }
    }
    return out;
}

static std::string hlsrc(const std::string& src) {
    std::string out;
    size_t start = 0;
    long num = 1;
    while (start <= src.size()) {
        size_t e = src.find((char)10, start);
        std::string line;
        if (e == std::string::npos) {
            line = src.substr(start);
            start = src.size() + 1;
        } else {
            line = src.substr(start, e - start);
            start = e + 1;
        }
        out += "<div class=L><span class=ln>";
        out += std::to_string(num);
        out += "</span>";
        out += hl(line);
        out += "</div>";
        ++num;
    }
    return out;
}

static std::string ut() {
    return std::to_string((long)time(0) - gStart);
}

static void rep(std::string& t, const std::string& a, const std::string& b) {
    size_t p = 0;
    for (;;) {
        p = t.find(a, p);
        if (p == std::string::npos) return;
        t.replace(p, a.size(), b);
        p += b.size();
    }
}

static std::string q2(const char* s) {
    std::string t;
    t += (char)34;
    t += s;
    t += (char)34;
    return t;
}

static std::string pageRoot() {
    std::string src = expandSrc();
    std::string body =
        "<!DOCTYPE html>"
        "<html><head><meta charset=utf-8><title>quine_server</title><style>"
        "body{margin:0;background:#0d1117;color:#c9d1d9;"
        "font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;}"
        ".bar{position:sticky;top:0;background:#161b22;"
        "border-bottom:1px solid #30363d;padding:14px 26px;"
        "display:flex;gap:26px;align-items:baseline;flex-wrap:wrap;}"
        ".t{color:#58a6ff;font-weight:bold;font-size:19px;}"
        ".b{color:#8b949e;font-size:13px;}"
        "main{padding:26px;margin:0 auto;max-width:1080px;}"
        ".L{white-space:pre-wrap;overflow-wrap:anywhere;line-height:1.5;}"
        ".ln{color:#4b5563;display:inline-block;width:3em;"
        "text-align:right;margin-right:1em;user-select:none;}"
        ".k{color:#ff7b72}.s{color:#a5d6ff}.c{color:#8b949e}"
        ".n{color:#79c0ff}.p{color:#d2a8ff}"
        "</style></head><body>"
        "<header class=bar>"
        "<span class=t>quine_server.cpp</span>"
        "<span class=b>this page IS its own source - served from memory</span>"
        "<span class=b>uptime UPTIME_TAG s</span>"
        "<span class=b>requests REQ_TAG</span>"
        "<span class=b>bytes BYTE_TAG</span>"
        "</header><main>HILITE_TAG</main></body></html>";
    rep(body, "UPTIME_TAG", ut());
    rep(body, "REQ_TAG", std::to_string(gReq));
    rep(body, "BYTE_TAG", std::to_string(gBytes));
    rep(body, "HILITE_TAG", hlsrc(src));
    return body;
}

static std::string pageStats() {
    std::string j;
    j += (char)123;
    j += q2("uptime");
    j += (char)58;
    j += ut();
    j += (char)44;
    j += q2("requests");
    j += (char)58;
    j += std::to_string(gReq);
    j += (char)44;
    j += q2("bytes");
    j += (char)58;
    j += std::to_string(gBytes);
    j += (char)125;
    return j;
}

static std::string page404(const std::string& path) {
    std::string body = R"HTML(
<!DOCTYPE html><html><head><meta charset=utf-8><title>404</title><style>
body{margin:0;background:#0d1117;color:#c9d1d9;font-family:monospace;
display:flex;min-height:100vh;align-items:center;justify-content:center}
.box{text-align:center}.big{font-size:72px;color:#ff7b72}
h1{color:#ff7b72;font-size:22px}a{color:#58a6ff;text-decoration:none}
.art{color:#8b949e;font-size:16px;line-height:1.25}
</style></head><body><div class=box>
<div class=big>404</div><h1>lost in the void</h1>
<p>no such page: )HTML";
    body += esc(path);
    body += R"HTML(</p><div class=art><pre>
    ________
   |  _  _  |
   | |_||_| |
   |   __   |
   |__|  |__|
</pre></div><p><a href=/>go home</a></p>
</div></body></html>)HTML";
    return body;
}

static void sendAll(int fd, const std::string& resp) {
    size_t off = 0;
    while (off < resp.size()) {
        ssize_t w = ::write(fd, resp.data() + off, resp.size() - off);
        if (w <= 0) break;
        off += (size_t)w;
    }
    gBytes += (long)resp.size();
}

int main(int argc, char** argv) {
    int port = 8080;
    if (argc > 1) {
        int v = 0;
        const char* q = argv[1];
        bool bad = (*q == 0);
        while (*q >= '0' && *q <= '9') {
            v = v * 10 + (*q - '0');
            ++q;
        }
        if (!bad && *q == 0 && v > 0) port = v;
    }
    gStart = (long)time(0);
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;
    int one = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons((unsigned short)port);
    if (::bind(fd, (sockaddr*)&sa, sizeof(sa)) < 0) return 2;
    if (::listen(fd, 32) < 0) return 3;
    std::string endh;
    eol(endh);
    eol(endh);
    for (;;) {
        sockaddr_in cl;
        socklen_t cll = sizeof(cl);
        int c = ::accept(fd, (sockaddr*)&cl, &cll);
        if (c < 0) continue;
        char buf[8192];
        std::string req;
        size_t got = 0;
        for (;;) {
            ssize_t r = ::read(c, buf + got, sizeof(buf) - got);
            if (r <= 0) break;
            got += (size_t)r;
            req.assign(buf, got);
            if (req.find(endh) != std::string::npos) break;
            if (got >= sizeof(buf)) break;
        }
        gReq += 1;
        std::string path = "/";
        size_t p1 = req.find(' ');
        if (p1 != std::string::npos) {
            size_t p2 = req.find(' ', p1 + 1);
            if (p2 != std::string::npos) {
                path = req.substr(p1 + 1, p2 - p1 - 1);
            }
        }
        std::string body = page404(path);
        std::string ctype = "text/html; charset=utf-8";
        int status = 404;
        if (path == "/") {
            body = pageRoot();
            status = 200;
        } else if (path == "/stats") {
            body = pageStats();
            ctype = "application/json; charset=utf-8";
            status = 200;
        }
        std::string head;
        head += "HTTP/1.1 ";
        head += std::to_string(status);
        head += " ";
        if (status == 200) head += "OK";
        else head += "Not Found";
        eol(head);
        head += "Content-Type: ";
        head += ctype;
        eol(head);
        head += "Content-Length: ";
        head += std::to_string((long)body.size());
        eol(head);
        head += "Connection: close";
        eol(head);
        head += "Server: quine";
        eol(head);
        eol(head);
        sendAll(c, head + body);
        ::close(c);
    }
    return 0;
}