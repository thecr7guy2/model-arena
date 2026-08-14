// chat.cpp — single-threaded, edge-triggered epoll TCP chat server (C++17, Linux)
// Build: g++ -std=c++17 -O2 chat.cpp -o chat
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <utility>

namespace {

constexpr int kMaxLine   = 8192;    // per-client input buffering limit
constexpr int kMaxOut    = 65536;   // per-client output buffering limit
constexpr int kMaxEvents = 64;

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

std::string trim(const std::string &s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

struct Client {
    int      fd       = -1;
    std::string peer;                     // "ip:port" for logging
    std::string inbuf;
    std::string outbuf;
    std::string nick;
    bool     has_nick = false;
    bool     writable = false;            // EPOLLOUT currently enabled?
    bool     dead     = false;            // scheduled for removal
};

class Server {
public:
    int run(int port) {
        listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ < 0) { perror("socket"); return 1; }
        int one = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        set_nonblocking(listen_fd_);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
            perror("bind"); return 1;
        }
        if (listen(listen_fd_, 128) < 0) { perror("listen"); return 1; }

        epfd_ = epoll_create1(0);
        if (epfd_ < 0) { perror("epoll_create1"); return 1; }

        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listen_fd_;
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) { perror("epoll_ctl"); return 1; }

        logf("chat server listening on port %d (pid %d)", port, getpid());
        loop();
        return 0;
    }

private:
    int  epfd_      = -1;
    int  listen_fd_ = -1;
    bool running_   = true;
    std::map<int, Client> clients_;
    std::vector<int> to_close_;

    void logf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);
        std::fputc('\n', stdout);
        std::fflush(stdout);
    }

    // --- epoll / I/O -------------------------------------------------------

    void accept_clients() {
        for (;;) {
            sockaddr_in cli;
            socklen_t len = sizeof(cli);
            int fd = accept4(listen_fd_, reinterpret_cast<sockaddr *>(&cli), &len,
                             SOCK_NONBLOCK);
            if (fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) return;  // drained
                if (errno == EINTR) continue;
                perror("accept");
                return;
            }
            Client c;
            c.fd = fd;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
            c.peer = std::string(ip) + ":" + std::to_string(ntohs(cli.sin_port));
            epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = fd;
            epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
            clients_.emplace(fd, std::move(c));
        }
    }

    void handle_client(int fd, uint32_t events) {
        auto it = clients_.find(fd);
        if (it == clients_.end() || it->second.dead) return;
        Client &c = it->second;

        if (events & (EPOLLIN | EPOLLHUP)) {
            char buf[4096];
            for (;;) {
                ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
                if (n > 0) {
                    c.inbuf.append(buf, static_cast<size_t>(n));
                    if (c.inbuf.size() > kMaxLine) { schedule_drop(fd); return; }
                    process_input(c);
                    if (c.dead) return;
                } else if (n == 0) {
                    schedule_drop(fd);   // orderly close (EOF)
                    return;
                } else if (errno == EINTR) {
                    continue;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;               // drained; ET requires draining until EAGAIN
                } else {
                    schedule_drop(fd);   // real error
                    return;
                }
            }
            if (events & EPOLLHUP) { schedule_drop(fd); return; }
        }

        if ((events & EPOLLOUT) && !c.dead) {
            bool closed = false;
            flush(c, closed);
            if (closed) schedule_drop(fd);
        }
        if (events & EPOLLERR) schedule_drop(fd);
    }

    // --- message handling ---------------------------------------------------

    void process_input(Client &c) {
        while (!c.dead) {
            size_t nl = c.inbuf.find('\n');
            if (nl == std::string::npos) {
                if (c.inbuf.size() > kMaxLine) schedule_drop(c.fd);
                return;
            }
            std::string raw = c.inbuf.substr(0, nl);
            c.inbuf.erase(0, nl + 1);
            process_line(c, raw);
        }
    }

    void process_line(Client &c, const std::string &raw) {
        std::string line = trim(raw);

        if (!c.has_nick) {
            if (line.empty()) {
                enqueue(c, "empty nickname not allowed; try again\n");
                return;
            }
            c.nick = line;
            c.has_nick = true;
            logf("+ %s connected from %s", c.nick.c_str(), c.peer.c_str());
            enqueue(c, "welcome, " + c.nick +
                       ". type /who for users, /quit to leave\n");
            return;
        }

        if (line == "/who") {
            std::string reply = "connected users: ";
            for (auto &kv : clients_) {
                if (kv.second.has_nick && !kv.second.dead)
                    reply += kv.second.nick + " ";
            }
            enqueue(c, reply + "\n");
            return;
        }
        if (line == "/quit") {
            schedule_drop(c.fd);
            return;
        }
        if (!line.empty()) {
            broadcast("[" + c.nick + "] " + line + "\n", c.fd);
        }
    }

    void broadcast(const std::string &msg, int except_fd) {
        std::vector<int> targets;
        for (auto &kv : clients_) {
            if (kv.first != except_fd && !kv.second.dead)
                targets.push_back(kv.first);
        }
        for (int fd : targets) {
            auto it = clients_.find(fd);
            if (it != clients_.end() && !it->second.dead)
                enqueue(it->second, msg);
        }
    }

    // --- output (partial-write safe) ----------------------------------------

    void enqueue(Client &c, const std::string &msg) {
        if (c.dead) return;
        if (c.outbuf.size() + msg.size() > kMaxOut) { schedule_drop(c.fd); return; }
        c.outbuf += msg;
        bool closed = false;
        flush(c, closed);
        if (closed) schedule_drop(c.fd);
    }

    void flush(Client &c, bool &closed) {
        closed = false;
        while (!c.outbuf.empty()) {
            ssize_t n = ::send(c.fd, c.outbuf.data(), c.outbuf.size(), MSG_NOSIGNAL);
            if (n > 0) {
                c.outbuf.erase(0, static_cast<size_t>(n));
                continue;
            }
            if (n < 0 && errno == EINTR) continue;
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                ensure_out(c.fd);   // re-arm EPOLLOUT if not yet armed
                return;
            }
            closed = true;          // peer got reset / removed
            return;
        }
        if (c.writable) {           // fully drained -> disable EPOLLOUT
            epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = c.fd;
            epoll_ctl(epfd_, EPOLL_CTL_MOD, c.fd, &ev);
            c.writable = false;
        }
    }

    bool ensure_out(int fd) {
        auto it = clients_.find(fd);
        if (it == clients_.end() || it->second.writable) return true;
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
        ev.data.fd = fd;
        if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == 0) {
            it->second.writable = true;
            return true;
        }
        return false;
    }

    // --- lifecycle ------------------------------------------------------------

    bool schedule_drop(int fd) {
        auto it = clients_.find(fd);
        if (it == clients_.end() || it->second.dead) return false;
        it->second.dead = true;
        to_close_.push_back(fd);
        return true;
    }

    void apply_drops() {
        for (int fd : to_close_) {
            auto it = clients_.find(fd);
            if (it == clients_.end()) continue;
            Client &c = it->second;
            if (c.has_nick)
                logf("- %s disconnected (%s)", c.nick.c_str(), c.peer.c_str());
            else
                logf("- connection from %s closed before choosing a nickname",
                     c.peer.c_str());
            ::close(fd);
            clients_.erase(it);
        }
        to_close_.clear();
    }

    void loop() {
        epoll_event events[kMaxEvents];
        while (running_) {
            int n = epoll_wait(epfd_, events, kMaxEvents, -1);
            if (n < 0) {
                if (errno == EINTR) continue;
                perror("epoll_wait");
                break;
            }
            for (int i = 0; i < n; ++i) {
                int fd = events[i].data.fd;
                uint32_t ev = events[i].events;
                if (fd == listen_fd_) {
                    if (ev & EPOLLIN) accept_clients();
                } else {
                    handle_client(fd, ev);
                }
            }
            apply_drops();
        }
    }
};

}  // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }
    std::signal(SIGPIPE, SIG_IGN);   // handle peer resets ourselves
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::fprintf(stderr, "invalid port: %s\n", argv[1]);
        return 1;
    }
    Server server;
    return server.run(port);
}
