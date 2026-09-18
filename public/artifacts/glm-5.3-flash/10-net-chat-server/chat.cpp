#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static const size_t kMaxBacklog = 4 * 1024 * 1024;

static int epfd = -1;

struct Client {
    int fd = -1;
    std::string peer;
    std::string nick;
    std::string inbuf;
    std::string outbuf;
    size_t outoff = 0;
    bool skip_line = false;
    bool want_close = false;
    const char* how = "closed";
};

static std::unordered_map<int, Client> clients;

static void log_line(const std::string& s) {
    std::fputs((s + "\n").c_str(), stdout);
    std::fflush(stdout);
}

static void set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl >= 0) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
}

static void mod_mask(const Client& c, bool want_out) {
    epoll_event ev{};
    uint32_t mask = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (want_out) mask |= EPOLLOUT;
    ev.events = mask;
    ev.data.fd = c.fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, c.fd, &ev);
}

static bool flush_client(Client& c) {
    while (c.outoff < c.outbuf.size()) {
        ssize_t n = ::write(c.fd, c.outbuf.data() + c.outoff, c.outbuf.size() - c.outoff);
        if (n > 0) {
            c.outoff += static_cast<size_t>(n);
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            mod_mask(c, true);
            return true;
        } else {
            c.want_close = true;
            c.how = "error";
            return false;
        }
    }
    if (c.outoff >= c.outbuf.size()) {
        c.outbuf.clear();
        c.outoff = 0;
        mod_mask(c, false);
    }
    return true;
}

static void queue_out(Client& c, const std::string& s) {
    if (c.want_close) return;
    if (c.outbuf.size() - c.outoff + s.size() > kMaxBacklog) {
        c.want_close = true;
        c.how = "slow";
        return;
    }
    c.outbuf.append(s);
    flush_client(c);
}

static void broadcast(const std::string& msg, int except_fd) {
    for (auto& kv : clients) {
        if (kv.first == except_fd) continue;
        queue_out(kv.second, msg);
    }
}

static void handle_line(Client& c, const std::string& raw) {
    std::string line = raw;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (c.nick.empty()) {
        size_t b = line.find_first_not_of(" \t");
        if (b == std::string::npos) {
            queue_out(c, "send a nickname first\n");
            return;
        }
        size_t e = line.find_last_not_of(" \t");
        std::string nick = line.substr(b, e - b + 1);
        if (nick.size() > 32) nick.resize(32);
        c.nick = nick;
        log_line("* " + nick + " joined (" + c.peer + ")");
        queue_out(c, "welcome " + nick + ", commands: /who /quit\n");
        return;
    }
    if (line == "/quit") {
        c.how = "quit";
        queue_out(c, "bye\n");
        c.want_close = true;
        return;
    }
    if (line == "/who") {
        std::string list;
        size_t count = 0;
        for (auto& kv : clients) {
            if (kv.second.nick.empty()) continue;
            if (count) list += ", ";
            list += kv.second.nick;
            ++count;
        }
        queue_out(c, "users online (" + std::to_string(count) + "): " + list + "\n");
        return;
    }
    broadcast("[" + c.nick + "] " + line + "\n", c.fd);
}

static void feed(Client& c, const char* data, size_t len) {
    c.inbuf.append(data, len);
    size_t start = 0;
    bool advanced = false;
    while (true) {
        size_t nl = c.inbuf.find('\n', start);
        if (nl == std::string::npos) break;
        std::string line = c.inbuf.substr(start, nl - start);
        start = nl + 1;
        advanced = true;
        if (c.skip_line) {
            c.skip_line = false;
            continue;
        }
        handle_line(c, line);
        if (c.want_close) break;
    }
    if (start) c.inbuf.erase(0, start);
    if (!advanced && c.inbuf.size() > kMaxBacklog) {
        c.skip_line = true;
        c.inbuf.clear();
    }
}

static void on_readable(Client& c) {
    char buf[4096];
    bool eof = false;
    while (!c.want_close) {
        ssize_t n = ::read(c.fd, buf, sizeof buf);
        if (n > 0) {
            feed(c, buf, static_cast<size_t>(n));
            continue;
        }
        if (n == 0) {
            eof = true;
            break;
        }
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        c.want_close = true;
        c.how = "error";
        return;
    }
    if (eof) {
        flush_client(c);
        c.want_close = true;
        c.how = "eof";
    }
}

static void accept_all(int lfd) {
    while (true) {
        sockaddr_in sa;
        socklen_t sl = sizeof sa;
        int cfd = accept(lfd, reinterpret_cast<sockaddr*>(&sa), &sl);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            return;
        }
        set_nonblock(cfd);
        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &sa.sin_addr, ip, sizeof ip);
        std::string peer = std::string(ip) + ":" + std::to_string(ntohs(sa.sin_port));
        log_line("* connection from " + peer);
        Client c;
        c.fd = cfd;
        c.peer = peer;
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = cfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
        clients.emplace(cfd, std::move(c));
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }
    char* end = nullptr;
    long port = std::strtol(argv[1], &end, 10);
    if (!argv[1][0] || (end && *end) || port <= 0 || port > 65535) {
        std::fprintf(stderr, "invalid port: %s\n", argv[1]);
        return 1;
    }
    std::signal(SIGPIPE, SIG_IGN);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return 1;
    }
    set_nonblock(lfd);
    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(lfd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(lfd, 128) < 0) {
        perror("listen");
        return 1;
    }

    epfd = epoll_create(256);
    if (epfd < 0) {
        perror("epoll_create");
        return 1;
    }
    set_nonblock(epfd);
    epoll_event lev{};
    lev.events = EPOLLIN | EPOLLET;
    lev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &lev);

    log_line(std::string("* listening on port ") + argv[1]);

    std::vector<epoll_event> evs(256);
    while (true) {
        int n = epoll_wait(epfd, evs.data(), static_cast<int>(evs.size()), -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = evs[i].data.fd;
            uint32_t e = evs[i].events;
            if (fd == lfd) {
                accept_all(lfd);
                continue;
            }
            auto it = clients.find(fd);
            if (it == clients.end() || it->second.want_close) continue;
            Client& c = it->second;
            if (e & EPOLLOUT) {
                if (!flush_client(c)) continue;
            }
            if (e & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                on_readable(c);
            }
        }
        for (auto it = clients.begin(); it != clients.end();) {
            Client& c = it->second;
            if (c.want_close) {
                if (c.nick.empty())
                    log_line("* " + c.peer + " left (" + c.how + ")");
                else
                    log_line("* " + c.nick + " left (" + c.how + ")");
                ::close(c.fd);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    return 0;
}
