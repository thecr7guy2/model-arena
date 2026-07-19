#include <sys/socket.h>
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
static const char* quine_fmt = "#include <sys/socket.h>%c#include <netinet/in.h>%c#include <unistd.h>%c#include <cstdlib>%c#include <cstring>%c#include <ctime>%c#include <string>%c#include <sstream>%c#include <algorithm>%c#include <chrono>%c#include <cstdio>%c%c// ---- quine infrastructure ------------------------------------------------%cstatic const char* quine_fmt = %c%s%c;%c%cstatic std::string build_self_source() {%c    char buf[400000];%c    int args[] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 92, 92, 10, 10, 10, 10, 34, 34, 10, 92, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 92, 10, 10, 10, 92, 10, 34, 34, 10, 10, 10, 10, 10, 34, 92, 10, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 10, 92, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 92, 10, 34, 34, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 92, 10, 10, 10, 34, 34, 10, 34, 34, 34, 34, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 92, 34, 92, 34, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 34, 34, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 34, 92, 92, 34, 10, 10, 34, 92, 92, 34, 10, 10, 34, 92, 92, 92, 92, 92, 92, 34, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };%c    int nargs = sizeof(args) / sizeof(args[0]);%c    // We need to interleave args and the %%s argument (quine_fmt).%c    // Build format by scanning quine_fmt and replacing %%c/%%s.%c    std::string out;%c    const char* p = quine_fmt;%c    int ai = 0;%c    while (*p) {%c        if (*p == '%%' && *(p+1) == 'c') {%c            out += (char)args[ai++];%c            p += 2;%c        } else if (*p == '%%' && *(p+1) == 's') {%c            out += quine_fmt;%c            p += 2;%c        } else {%c            out += *p;%c            p++;%c        }%c    }%c    return out;%c}%c// -------------------------------------------------------------------------%c%cstatic std::string html_escape(const std::string& s) {%c    std::string r; r.reserve(s.size() * 2);%c    for (unsigned char c : s) {%c        switch (c) {%c            case '&': r += %c&amp;%c; break;%c            case '<': r += %c&lt;%c; break;%c            case '>': r += %c&gt;%c; break;%c            case '%c': r += %c&quot;%c; break;%c            default:  r += c;%c        }%c    }%c    return r;%c}%c%cstatic bool is_keyword(const std::string& w) {%c    static const char* kw[] = {%c        %calignas%c,%calignof%c,%cand%c,%cand_eq%c,%casm%c,%cauto%c,%cbitand%c,%cbitor%c,%c        %cbool%c,%cbreak%c,%ccase%c,%ccatch%c,%cchar%c,%cclass%c,%ccompl%c,%cconst%c,%c        %cconstexpr%c,%cconst_cast%c,%ccontinue%c,%cdecltype%c,%cdefault%c,%cdelete%c,%c        %cdo%c,%cdouble%c,%cdynamic_cast%c,%celse%c,%cenum%c,%cexplicit%c,%cexport%c,%c        %cextern%c,%cfalse%c,%cfloat%c,%cfor%c,%cfriend%c,%cgoto%c,%cif%c,%cinline%c,%c        %cint%c,%clong%c,%cmutable%c,%cnamespace%c,%cnew%c,%cnoexcept%c,%cnot%c,%cnot_eq%c,%c        %cnullptr%c,%coperator%c,%cor%c,%cor_eq%c,%cprivate%c,%cprotected%c,%cpublic%c,%c        %cregister%c,%creinterpret_cast%c,%creturn%c,%cshort%c,%csigned%c,%csizeof%c,%c        %cstatic%c,%cstatic_assert%c,%cstatic_cast%c,%cstruct%c,%cswitch%c,%ctemplate%c,%c        %cthis%c,%cthread_local%c,%cthrow%c,%ctrue%c,%ctry%c,%ctypedef%c,%ctypeid%c,%c        %ctypename%c,%cunion%c,%cunsigned%c,%cusing%c,%cvirtual%c,%cvoid%c,%cvolatile%c,%c        %cwhile%c,%cxor%c,%cxor_eq%c,%cinclude%c,%cdefine%c,%cifdef%c,%cifndef%c,%cendif%c,%c        %cpragma%c,%cundef%c,%cerror%c,%cline%c,%c#include%c,%c#define%c,%c#ifdef%c,%c        %c#ifndef%c,%c#endif%c,%c#pragma%c,%c#undef%c,%c#error%c%c    };%c    for (auto* k : kw) if (w == k) return true;%c    return false;%c}%c%cstatic bool is_type(const std::string& w) {%c    static const char* tp[] = {%c        %csize_t%c,%cssize_t%c,%cint8_t%c,%cint16_t%c,%cint32_t%c,%cint64_t%c,%c        %cuint8_t%c,%cuint16_t%c,%cuint32_t%c,%cuint64_t%c,%ctime_t%c,%cclock_t%c,%c        %csocklen_t%c,%csa_family_t%c,%cin_addr_t%c,%cin_port_t%c,%cpid_t%c,%c        %cstd%c,%cstring%c,%cvector%c,%cmap%c,%cunordered_map%c,%cset%c,%cpair%c,%c        %costream%c,%cistream%c,%cstringstream%c,%costringstream%c,%cistringstream%c,%c        %cchrono%c,%cthread%c,%cmutex%c,%clock_guard%c,%cunique_ptr%c,%cshared_ptr%c,%c        %cfunction%c,%cbind%c,%cref%c,%ccref%c%c    };%c    for (auto* t : tp) if (w == t) return true;%c    return false;%c}%c%cstatic bool is_number(const std::string& w) {%c    if (w.empty()) return false;%c    for (size_t i = 0; i < w.size(); i++) {%c        char c = w[i];%c        if (!((c >= '0' && c <= '9') || c == '.' || c == 'x' || c == 'X' ||%c              c == 'u' || c == 'U' || c == 'l' || c == 'L' ||%c              c == 'f' || c == 'F' || (i == 0 && c == '-')))%c            return false;%c    }%c    return true;%c}%c%cstatic bool is_preproc(const std::string& w) {%c    return w == %c#include%c || w == %c#define%c || w == %c#ifdef%c ||%c           w == %c#ifndef%c || w == %c#endif%c || w == %c#pragma%c ||%c           w == %c#undef%c || w == %c#error%c || w == %c#if%c || w == %c#else%c;%c}%c%cstatic std::string highlight(const std::string& src) {%c    std::string r;%c    std::string w;%c    bool in_string = false;%c    bool in_char   = false;%c    bool in_preproc = false;%c    char str_delim = 0;%c    for (size_t i = 0; i < src.size(); i++) {%c        unsigned char c = src[i];%c        if (in_string || in_char) {%c            r += c;%c            if (c == '%c%c' && i + 1 < src.size()) {%c                r += src[++i];%c            } else if (in_string && c == str_delim) {%c                in_string = false;%c                r += %c</span>%c;%c            } else if (in_char && c == '%c'') {%c                in_char = false;%c                r += %c</span>%c;%c            }%c            continue;%c        }%c        // preprocessor line%c        if (in_preproc) {%c            r += c;%c            if (c == '%cn') in_preproc = false;%c            continue;%c        }%c        if (c == '#' && (i == 0 || src[i-1] == '%cn')) {%c            r += %c<span class=pp>%c;%c            in_preproc = true;%c            r += c;%c            continue;%c        }%c        if (c == '%c' || c == '%c'') {%c            if (!w.empty()) {%c                if (is_keyword(w)) {%c                    r += %c<span class=kw>%c + w + %c</span>%c;%c                } else if (is_type(w)) {%c                    r += %c<span class=ty>%c + w + %c</span>%c;%c                } else if (is_number(w)) {%c                    r += %c<span class=nm>%c + w + %c</span>%c;%c                } else {%c                    r += w;%c                }%c                w.clear();%c            }%c            r += %c<span class=st>%c + std::string(1, c);%c            in_string = (c == '%c');%c            in_char   = (c == '%c'');%c            str_delim = c;%c            continue;%c        }%c        if (c == '/' && i + 1 < src.size() && src[i+1] == '/') {%c            if (!w.empty()) {%c                if (is_keyword(w)) r += %c<span class=kw>%c + w + %c</span>%c;%c                else if (is_type(w)) r += %c<span class=ty>%c + w + %c</span>%c;%c                else r += w;%c                w.clear();%c            }%c            r += %c<span class=cm>//%c;%c            i++;%c            while (i + 1 < src.size() && src[i+1] != '%cn') r += src[++i];%c            r += %c</span>%c;%c            continue;%c        }%c        if (c == '/' && i + 1 < src.size() && src[i+1] == '*') {%c            if (!w.empty()) {%c                if (is_keyword(w)) r += %c<span class=kw>%c + w + %c</span>%c;%c                else if (is_type(w)) r += %c<span class=ty>%c + w + %c</span>%c;%c                else r += w;%c                w.clear();%c            }%c            r += %c<span class=cm>/*%c;%c            i += 2;%c            while (i < src.size()) {%c                r += src[i];%c                if (src[i] == '*' && i + 1 < src.size() && src[i+1] == '/') {%c                    r += src[++i]; break;%c                }%c                i++;%c            }%c            r += %c</span>%c;%c            continue;%c        }%c        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||%c            c == '_' || c == '#' || (c >= '0' && c <= '9' && !w.empty()) ||%c            (c >= '0' && c <= '9' && w.empty())) {%c            if (c == '#' && !w.empty()) { r += w; w.clear(); w += c; }%c            else { w += c; }%c            continue;%c        }%c        if (!w.empty()) {%c            if (is_keyword(w)) {%c                if (w == %c#include%c || w == %c#define%c || w == %c#ifdef%c ||%c                    w == %c#ifndef%c || w == %c#endif%c || w == %c#pragma%c) {%c                    r += %c<span class=pp>%c + w + %c</span>%c;%c                } else {%c                    r += %c<span class=kw>%c + w + %c</span>%c;%c                }%c            } else if (is_type(w)) {%c                r += %c<span class=ty>%c + w + %c</span>%c;%c            } else if (is_number(w)) {%c                r += %c<span class=nm>%c + w + %c</span>%c;%c            } else {%c                r += w;%c            }%c            w.clear();%c        }%c        r += c;%c    }%c    if (!w.empty()) {%c        if (is_keyword(w)) r += %c<span class=kw>%c + w + %c</span>%c;%c        else if (is_type(w)) r += %c<span class=ty>%c + w + %c</span>%c;%c        else r += w;%c    }%c    return r;%c}%c%cstatic std::string page_header(const std::string& self_src,%c                               long uptime, long requests) {%c    std::string h;%c    h += %c<!DOCTYPE html><html lang=en><head><meta charset=UTF-8>%c;%c    h += %c<title>quine server</title>%c;%c    h += %c<style>%c;%c    h += %c*{margin:0;padding:0;box-sizing:border-box}%c;%c    h += %cbody{background:#1e1e1e;color:#d4d4d4;font:14px/1.5 'Cascadia Code','Fira Code','JetBrains Mono','DejaVu Sans Mono',monospace}%c;%c    h += %c#bar{position:sticky;top:0;z-index:10;background:#2d2d2d;border-bottom:2px solid #569cd6;padding:8px 20px;display:flex;gap:24px;flex-wrap:wrap;align-items:center;box-shadow:0 2px 8px rgba(0,0,0,.5)}%c;%c    h += %c#bar .title{color:#569cd6;font-weight:700;font-size:16px}%c;%c    h += %c#bar .stat{color:#9cdcfe}%c;%c    h += %c#bar .stat span{color:#ce9178}%c;%c    h += %c#code{overflow-x:auto;padding:16px 20px 40px}%c;%c    h += %c#code pre{margin:0}%c;%c    h += %c.kw{color:#569cd6}.ty{color:#4ec9b0}.st{color:#ce9178}%c;%c    h += %c.cm{color:#6a9955}.nm{color:#b5cea8}.pp{color:#c586c0}%c;%c    h += %c.ln{color:#858585;user-select:none;text-align:right;padding-right:16px;display:inline-block;width:48px}%c;%c    h += %ca{color:#569cd6;text-decoration:none;margin-left:auto}%c;%c    h += %ca:hover{text-decoration:underline}%c;%c    h += %c</style></head><body>%c;%c    h += %c<div id=bar>%c;%c    h += %c<span class=title>quine server</span>%c;%c    h += %c<span class=stat>uptime <span>%c + std::to_string(uptime) + %cs</span></span>%c;%c    h += %c<span class=stat>requests <span>%c + std::to_string(requests) + %c</span></span>%c;%c    h += %c<a href=/stats>stats</a>%c;%c    h += %c</div>%c;%c    h += %c<div id=code><pre>%c;%c    return h;%c}%c%cstatic std::string page_footer() {%c    return %c</pre></div></body></html>%c;%c}%c%cstatic std::string render_source(const std::string& self_src,%c                                  long uptime, long requests) {%c    std::string h = page_header(self_src, uptime, requests);%c    // Add line numbers and highlighted source%c    std::string line;%c    size_t lnum = 0;%c    for (size_t i = 0; i <= self_src.size(); i++) {%c        if (i == self_src.size() || self_src[i] == '%cn') {%c            lnum++;%c            std::string ln = std::to_string(lnum);%c            while (ln.size() < 3) ln = %c %c + ln;%c            h += %c<span class=ln>%c + ln + %c</span>%c;%c            h += highlight(line) + %c%cn%c;%c            line.clear();%c        } else {%c            line += self_src[i];%c        }%c    }%c    h += page_footer();%c    return h;%c}%c%cstatic std::string render_stats(long uptime, long requests, long bytes) {%c    std::string j;%c    j += %c{%c%cuptime_seconds%c%c:%c;%c    j += std::to_string(uptime);%c    j += %c,%c%ctotal_requests%c%c:%c;%c    j += std::to_string(requests);%c    j += %c,%c%ctotal_bytes_served%c%c:%c;%c    j += std::to_string(bytes);%c    j += %c}%cn%c;%c    return j;%c}%c%cstatic std::string render_404() {%c    std::string h;%c    h += %c<!DOCTYPE html><html lang=en><head><meta charset=UTF-8>%c;%c    h += %c<title>404 - not found</title>%c;%c    h += %c<style>body{background:#1e1e1e;color:#d4d4d4;font-family:monospace;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;text-align:center}%c;%c    h += %ch1{color:#f44747;font-size:72px;margin:0}%c;%c    h += %cp{color:#9cdcfe;font-size:18px}%c;%c    h += %cpre{color:#6a9955;font-size:14px;margin-top:20px}%c;%c    h += %ca{color:#569cd6;text-decoration:none}%c;%c    h += %ca:hover{text-decoration:underline}%c;%c    h += %c</style></head><body>%c;%c    h += %c<h1>404</h1>%c;%c    h += %c<p>nothing to see here</p>%c;%c    h += %c<pre>%c;%c    h += %c    ,-%c%c%c%c%c%c-.%cn%c;%c    h += %c   /        %c%c%cn%c;%c    h += %c  /_        _%c%c%cn%c;%c    h += %c  //%c%c      /%c%c%c%c%cn%c;%c    h += %c  |%c%c %c%c    / / |%cn%c;%c    h += %c  | %c%c %c%c  / /  |%cn%c;%c    h += %c  |  %c%c %c%c/ /   |%cn%c;%c    h += %c  |   %c%c  /    |%cn%c;%c    h += %c  |    %c%c/     |%cn%c;%c    h += %c  |          |%cn%c;%c    h += %c  |          |%cn%c;%c    h += %c  |          |%cn%c;%c    h += %c  |          |%cn%c;%c    h += %c  |    __    |%cn%c;%c    h += %c  |   /  %c%c   |%cn%c;%c    h += %c  |  /    %c%c  |%cn%c;%c    h += %c  | /      %c%c |%cn%c;%c    h += %c  |/        %c%c|%cn%c;%c    h += %c  '--....--'%cn%c;%c    h += %c</pre>%c;%c    h += %c<p><a href=/>go home</a></p>%c;%c    h += %c</body></html>%c;%c    return h;%c}%c%cstatic void send_response(int fd, int status, const std::string& ct,%c                           const std::string& body) {%c    std::string r;%c    r += %cHTTP/1.1 %c;%c    r += std::to_string(status);%c    r += %c %c;%c    switch (status) {%c        case 200: r += %cOK%c; break;%c        case 404: r += %cNot Found%c; break;%c        default:  r += %cUnknown%c;%c    }%c    r += %c%cr%cnContent-Type: %c;%c    r += ct;%c    r += %c%cr%cnContent-Length: %c;%c    r += std::to_string(body.size());%c    r += %c%cr%cnConnection: close%cr%cn%cr%cn%c;%c    r += body;%c    ::write(fd, r.data(), r.size());%c}%c%cint main(int argc, char** argv) {%c    if (argc < 2) {%c        ::write(2, %cUsage: quine_server <port>%cn%c, 27);%c        return 1;%c    }%c    int port = std::atoi(argv[1]);%c    if (port <= 0 || port > 65535) {%c        ::write(2, %cinvalid port%cn%c, 13);%c        return 1;%c    }%c%c    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);%c    if (server_fd < 0) { ::write(2, %csocket failed%cn%c, 14); return 1; }%c    int opt = 1;%c    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));%c    struct sockaddr_in addr;%c    std::memset(&addr, 0, sizeof(addr));%c    addr.sin_family = AF_INET;%c    addr.sin_addr.s_addr = INADDR_ANY;%c    addr.sin_port = htons(port);%c    if (::bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {%c        ::write(2, %cbind failed%cn%c, 12); return 1;%c    }%c    if (::listen(server_fd, 128) < 0) {%c        ::write(2, %clisten failed%cn%c, 14); return 1;%c    }%c%c    // Build the self-source once at startup%c    std::string self_source = build_self_source();%c%c    auto start_time = std::chrono::steady_clock::now();%c    long total_requests = 0;%c    long total_bytes = 0;%c%c    char buf[16384];%c%c    while (true) {%c        struct sockaddr_in cli;%c        socklen_t cli_len = sizeof(cli);%c        int client_fd = ::accept(server_fd, (struct sockaddr*)&cli, &cli_len);%c        if (client_fd < 0) continue;%c%c        total_requests++;%c        ssize_t n = ::read(client_fd, buf, sizeof(buf) - 1);%c        if (n <= 0) { ::close(client_fd); continue; }%c        buf[n] = 0;%c%c        // Parse method and path from the first line%c        std::string req(buf);%c        std::string method, path;%c        size_t sp1 = req.find(' ');%c        if (sp1 != std::string::npos) {%c            method = req.substr(0, sp1);%c            size_t sp2 = req.find(' ', sp1 + 1);%c            if (sp2 != std::string::npos) {%c                path = req.substr(sp1 + 1, sp2 - sp1 - 1);%c            }%c        }%c%c        auto now = std::chrono::steady_clock::now();%c        long uptime = std::chrono::duration_cast<std::chrono::seconds>(%c                          now - start_time).count();%c%c        std::string response_body;%c        std::string content_type;%c        int status = 200;%c%c        if (method != %cGET%c) {%c            status = 404;%c            response_body = render_404();%c            content_type = %ctext/html%c;%c        } else if (path == %c/%c || path.empty()) {%c            response_body = render_source(self_source, uptime, total_requests);%c            content_type = %ctext/html; charset=utf-8%c;%c        } else if (path == %c/stats%c) {%c            response_body = render_stats(uptime, total_requests, total_bytes);%c            content_type = %capplication/json%c;%c        } else {%c            status = 404;%c            response_body = render_404();%c            content_type = %ctext/html%c;%c        }%c%c        send_response(client_fd, status, content_type, response_body);%c        total_bytes += response_body.size();%c        ::close(client_fd);%c    }%c    ::close(server_fd);%c    return 0;%c}%c";

static std::string build_self_source() {
    char buf[400000];
    int args[] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 92, 92, 10, 10, 10, 10, 34, 34, 10, 92, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 92, 10, 10, 10, 92, 10, 34, 34, 10, 10, 10, 10, 10, 34, 92, 10, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 10, 92, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 92, 10, 34, 34, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 34, 34, 34, 34, 10, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 34, 10, 34, 34, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 92, 10, 10, 10, 34, 34, 10, 34, 34, 34, 34, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 92, 34, 34, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 34, 92, 34, 92, 34, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 92, 92, 34, 10, 34, 92, 34, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 34, 34, 10, 10, 34, 34, 10, 34, 34, 10, 34, 34, 10, 10, 34, 92, 92, 34, 10, 10, 34, 92, 92, 34, 10, 10, 34, 92, 92, 92, 92, 92, 92, 34, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 92, 34, 10, 10, 10, 34, 92, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 34, 34, 10, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 10, 34, 34, 10, 10, 34, 34, 10, 10, 10, 10, 34, 34, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
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
