#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr int kMaxEvents = 64;
constexpr std::size_t kMaxLine = 64 * 1024;
constexpr std::size_t kMaxOut = 4 * 1024 * 1024;
constexpr std::size_t kMaxNick = 64;

struct Client {
    int fd = -1;
    std::string in;
    std::string out;
    std::string nick;
    std::uint32_t events = 0;
    bool registered = false;
    bool dead = false;
};

std::string trim(std::string_view text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return std::string(text.substr(begin, end - begin));
}

class ChatServer {
public:
    explicit ChatServer(std::uint16_t port) : port_(port) {}

    ~ChatServer() {
        for (const auto& entry : clients_) ::close(entry.first);
        if (listen_fd_ >= 0) ::close(listen_fd_);
        if (epfd_ >= 0) ::close(epfd_);
    }

    bool init() {
        if (!create_listener()) return false;
        epfd_ = ::epoll_create1(EPOLL_CLOEXEC);
        if (epfd_ < 0) {
            std::perror("epoll_create1");
            return false;
        }
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listen_fd_;
        if (::epoll_ctl(epfd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
            std::perror("epoll_ctl");
            return false;
        }
        log("listening on port " + std::to_string(port_));
        return true;
    }

    void run() {
        std::vector<epoll_event> events(kMaxEvents);
        for (;;) {
            int ready = ::epoll_wait(epfd_, events.data(), kMaxEvents, -1);
            if (ready < 0) {
                if (errno == EINTR) continue;
                std::perror("epoll_wait");
                break;
            }
            for (int i = 0; i < ready; ++i) {
                int fd = events[i].data.fd;
                std::uint32_t mask = events[i].events;
                if (fd == listen_fd_) {
                    accept_clients();
                    continue;
                }
                auto it = clients_.find(fd);
                if (it == clients_.end() || it->second.dead) continue;
                Client& client = it->second;
                if (mask & (EPOLLERR | EPOLLHUP)) {
                    mark_dead(client);
                    continue;
                }
                if (mask & (EPOLLIN | EPOLLRDHUP)) read_client(client);
                if (!client.dead && (mask & EPOLLOUT)) flush(client);
            }
            reap();
        }
    }

private:
    std::uint16_t port_;
    int listen_fd_ = -1;
    int epfd_ = -1;
    std::unordered_map<int, Client> clients_;
    std::vector<int> dead_fds_;

    static void log(const std::string& text) {
        std::printf("[server] %s\n", text.c_str());
        std::fflush(stdout);
    }

    bool create_listener() {
        listen_fd_ = ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (listen_fd_ >= 0) {
            int one = 1;
            int zero = 0;
            ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
            ::setsockopt(listen_fd_, IPPROTO_IPV6, IPV6_V6ONLY, &zero, sizeof(zero));
            sockaddr_in6 addr{};
            addr.sin6_family = AF_INET6;
            addr.sin6_addr = in6addr_any;
            addr.sin6_port = htons(port_);
            if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0 &&
                ::listen(listen_fd_, SOMAXCONN) == 0) {
                return true;
            }
            ::close(listen_fd_);
            listen_fd_ = -1;
        }

        listen_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (listen_fd_ < 0) {
            std::perror("socket");
            return false;
        }
        int one = 1;
        ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);
        if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            std::perror("bind");
            return false;
        }
        if (::listen(listen_fd_, SOMAXCONN) < 0) {
            std::perror("listen");
            return false;
        }
        return true;
    }

    bool add_to_epoll(int fd, std::uint32_t events) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        return ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == 0;
    }

    static std::string format_address(const sockaddr_storage& addr) {
        char host[INET6_ADDRSTRLEN] = {0};
        std::uint16_t port = 0;
        if (addr.ss_family == AF_INET) {
            const auto* a = reinterpret_cast<const sockaddr_in*>(&addr);
            ::inet_ntop(AF_INET, &a->sin_addr, host, sizeof(host));
            port = ntohs(a->sin_port);
        } else if (addr.ss_family == AF_INET6) {
            const auto* a = reinterpret_cast<const sockaddr_in6*>(&addr);
            ::inet_ntop(AF_INET6, &a->sin6_addr, host, sizeof(host));
            port = ntohs(a->sin6_port);
        } else {
            std::strcpy(host, "unknown");
        }
        return std::string(host) + ":" + std::to_string(port);
    }

    void accept_clients() {
        for (;;) {
            sockaddr_storage addr{};
            socklen_t addr_len = sizeof(addr);
            int fd = ::accept4(listen_fd_, reinterpret_cast<sockaddr*>(&addr), &addr_len,
                               SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (fd < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                std::perror("accept4");
                break;
            }
            int one = 1;
            ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

            Client client;
            client.fd = fd;
            client.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
            auto inserted = clients_.emplace(fd, std::move(client));
            if (!add_to_epoll(fd, inserted.first->second.events)) {
                clients_.erase(inserted.first);
                ::close(fd);
                continue;
            }
            log("connection from " + format_address(addr));
        }
    }

    void read_client(Client& client) {
        char buffer[65536];
        for (;;) {
            ssize_t n = ::recv(client.fd, buffer, sizeof(buffer), 0);
            if (n > 0) {
                client.in.append(buffer, static_cast<std::size_t>(n));
                process_buffer(client);
                if (client.dead) return;
            } else if (n == 0) {
                mark_dead(client);
                return;
            } else {
                if (errno == EINTR) continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) return;
                mark_dead(client);
                return;
            }
        }
    }

    void process_buffer(Client& client) {
        std::size_t begin = 0;
        std::size_t pos = 0;
        while (!client.dead && (pos = client.in.find('\n', begin)) != std::string::npos) {
            std::string_view line(client.in.data() + begin, pos - begin);
            begin = pos + 1;
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
            handle_line(client, line);
        }
        if (begin > 0) client.in.erase(0, begin);
        if (!client.dead && client.in.size() > kMaxLine) {
            log("client fd=" + std::to_string(client.fd) +
                " exceeded maximum line length, disconnecting");
            mark_dead(client);
        }
    }

    void handle_line(Client& client, std::string_view line) {
        if (!client.registered) {
            std::string nick = trim(line);
            if (nick.empty()) nick = "guest";
            if (nick.size() > kMaxNick) nick.resize(kMaxNick);
            client.nick = std::move(nick);
            client.registered = true;
            log(client.nick + " joined");
            return;
        }

        std::string_view command = line;
        while (!command.empty() && std::isspace(static_cast<unsigned char>(command.front()))) {
            command.remove_prefix(1);
        }
        while (!command.empty() && std::isspace(static_cast<unsigned char>(command.back()))) {
            command.remove_suffix(1);
        }

        if (command == "/quit") {
            mark_dead(client);
            return;
        }
        if (command == "/who") {
            send_who(client);
            return;
        }
        broadcast(client, line);
    }

    void send_who(Client& client) {
        std::string list;
        std::size_t count = 0;
        for (const auto& entry : clients_) {
            const Client& other = entry.second;
            if (!other.registered || other.dead) continue;
            if (count > 0) list += ", ";
            list += other.nick;
            ++count;
        }
        enqueue(client, "Connected users (" + std::to_string(count) + "): " + list + "\n");
    }

    void broadcast(const Client& sender, std::string_view line) {
        std::string message;
        message.reserve(sender.nick.size() + line.size() + 4);
        message += '[';
        message += sender.nick;
        message += "] ";
        message.append(line.data(), line.size());
        message += '\n';
        for (auto& entry : clients_) {
            Client& other = entry.second;
            if (other.fd == sender.fd || other.dead) continue;
            enqueue(other, message);
        }
    }

    void enqueue(Client& client, const std::string& data) {
        if (client.dead) return;
        if (client.out.size() + data.size() > kMaxOut) {
            log("client fd=" + std::to_string(client.fd) +
                " output buffer overflow, disconnecting");
            mark_dead(client);
            return;
        }
        client.out += data;
        flush(client);
    }

    void flush(Client& client) {
        while (!client.out.empty()) {
            ssize_t n = ::send(client.fd, client.out.data(), client.out.size(), MSG_NOSIGNAL);
            if (n > 0) {
                client.out.erase(0, static_cast<std::size_t>(n));
            } else if (n < 0 && errno == EINTR) {
                continue;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break;
            } else {
                mark_dead(client);
                return;
            }
        }
        update_events(client);
    }

    void update_events(Client& client) {
        if (client.dead) return;
        std::uint32_t want = EPOLLIN | EPOLLET | EPOLLRDHUP;
        if (!client.out.empty()) want |= EPOLLOUT;
        if (want == client.events) return;
        epoll_event ev{};
        ev.events = want;
        ev.data.fd = client.fd;
        if (::epoll_ctl(epfd_, EPOLL_CTL_MOD, client.fd, &ev) < 0 && errno != EINTR) {
            mark_dead(client);
            return;
        }
        client.events = want;
    }

    void mark_dead(Client& client) {
        if (client.dead) return;
        client.dead = true;
        dead_fds_.push_back(client.fd);
    }

    void reap() {
        for (int fd : dead_fds_) {
            auto it = clients_.find(fd);
            if (it == clients_.end()) continue;
            Client& client = it->second;
            if (client.registered) log(client.nick + " left");
            ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
            ::close(fd);
            clients_.erase(it);
        }
        dead_fds_.clear();
    }
};

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    char* end = nullptr;
    long port = std::strtol(argv[1], &end, 10);
    if (*argv[1] == '\0' || end == nullptr || *end != '\0' || port < 1 || port > 65535) {
        std::fprintf(stderr, "invalid port: %s\n", argv[1]);
        return 1;
    }

    std::signal(SIGPIPE, SIG_IGN);

    ChatServer server(static_cast<std::uint16_t>(port));
    if (!server.init()) return 1;
    server.run();
    return 0;
}