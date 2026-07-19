#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <csignal>

static constexpr size_t MAX_EVENTS = 64;
static constexpr size_t READ_BUF_SIZE = 4096;

struct Client {
    std::string nick;
    bool has_nick = false;
    std::string read_buf;
    std::deque<std::string> send_queue;
};

class ChatServer {
public:
    explicit ChatServer(int port) : port_(port) {}

    int setup() {
        signal(SIGPIPE, SIG_IGN);

        listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (listen_fd_ < 0) {
            std::perror("socket");
            return -1;
        }

        int opt = 1;
        if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::perror("setsockopt");
            return -1;
        }

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::perror("bind");
            return -1;
        }

        if (listen(listen_fd_, SOMAXCONN) < 0) {
            std::perror("listen");
            return -1;
        }

        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ < 0) {
            std::perror("epoll_create1");
            return -1;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listen_fd_;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
            std::perror("epoll_ctl listen");
            return -1;
        }

        return 0;
    }

    void run() {
        struct epoll_event events[MAX_EVENTS];

        while (true) {
            int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                std::perror("epoll_wait");
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                uint32_t ev = events[i].events;

                if (fd == listen_fd_) {
                    if (ev & EPOLLIN) {
                        handle_accept();
                    }
                } else {
                    if (ev & EPOLLIN) {
                        handle_read(fd);
                    }
                    if (ev & EPOLLOUT) {
                        if (clients_.count(fd)) {
                            handle_write(fd);
                        }
                    }
                    if (ev & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                        if (clients_.count(fd)) {
                            disconnect(fd);
                        }
                    }
                }
            }
        }
    }

private:
    int port_;
    int listen_fd_ = -1;
    int epoll_fd_ = -1;
    std::unordered_map<int, std::unique_ptr<Client>> clients_;

    void handle_accept() {
        while (true) {
            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);
            int fd = accept4(listen_fd_, (struct sockaddr*)&addr, &addrlen, SOCK_NONBLOCK);
            if (fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                if (errno == EINTR) continue;
                std::perror("accept");
                break;
            }

            auto client = std::make_unique<Client>();

            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
            ev.data.fd = fd;
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
                std::perror("epoll_ctl add client");
                ::close(fd);
                continue;
            }

            clients_[fd] = std::move(client);
            std::cout << "<-- " << fd << " connected" << std::endl;
        }
    }

    void handle_read(int fd) {
        char buf[READ_BUF_SIZE];
        bool closed = false;

        while (true) {
            ssize_t n = ::read(fd, buf, sizeof(buf));
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                closed = true;
                break;
            }
            if (n == 0) {
                closed = true;
                break;
            }
            auto it = clients_.find(fd);
            if (it == clients_.end()) { closed = true; break; }
            it->second->read_buf.append(buf, n);
        }

        if (closed) {
            disconnect(fd);
            return;
        }

        process_lines(fd);
    }

    void process_lines(int fd) {
        while (true) {
            auto it = clients_.find(fd);
            if (it == clients_.end()) return;
            auto& client = it->second;

            size_t pos = client->read_buf.find('\n');
            if (pos == std::string::npos) return;

            std::string line = client->read_buf.substr(0, pos);
            client->read_buf.erase(0, pos + 1);

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) continue;

            if (!client->has_nick) {
                client->nick = std::move(line);
                client->has_nick = true;
                std::cout << "--> " << client->nick << " (" << fd << ") joined" << std::endl;
                continue;
            }

            if (line == "/quit") {
                disconnect(fd);
                return;
            }

            if (line == "/who") {
                std::string who;
                for (const auto& pair : clients_) {
                    if (pair.second->has_nick) {
                        who += pair.second->nick + "\n";
                    }
                }
                if (!who.empty()) who.pop_back();
                send_data(fd, std::move(who));
                continue;
            }

            std::string msg = "[" + client->nick + "] " + line + "\n";
            std::vector<int> recipients;
            recipients.reserve(clients_.size());
            for (const auto& pair : clients_) {
                if (pair.first != fd && pair.second->has_nick) {
                    recipients.push_back(pair.first);
                }
            }
            for (int f : recipients) {
                send_data(f, msg);
            }
        }
    }

    void handle_write(int fd) {
        auto it = clients_.find(fd);
        if (it == clients_.end()) return;
        auto& client = it->second;

        while (!client->send_queue.empty()) {
            auto& msg = client->send_queue.front();
            ssize_t n = ::write(fd, msg.data(), msg.size());
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) return;
                disconnect(fd);
                return;
            }
            if (static_cast<size_t>(n) < msg.size()) {
                msg.erase(0, n);
                return;
            }
            client->send_queue.pop_front();
        }

        mod_epoll(fd, EPOLLIN | EPOLLRDHUP | EPOLLET);
    }

    void send_data(int fd, std::string data) {
        auto it = clients_.find(fd);
        if (it == clients_.end()) return;
        auto& client = it->second;

        if (client->send_queue.empty()) {
            ssize_t n = ::write(fd, data.data(), data.size());
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    client->send_queue.push_back(std::move(data));
                    mod_epoll(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
                } else {
                    disconnect(fd);
                }
                return;
            }
            if (static_cast<size_t>(n) < data.size()) {
                data.erase(0, n);
                client->send_queue.push_back(std::move(data));
                mod_epoll(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
            }
        } else {
            client->send_queue.push_back(std::move(data));
        }
    }

    void mod_epoll(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }

    void disconnect(int fd) {
        auto it = clients_.find(fd);
        if (it == clients_.end()) return;

        if (it->second->has_nick) {
            std::cout << "<-- " << it->second->nick << " (" << fd << ") left" << std::endl;
        } else {
            std::cout << "<-- " << fd << " disconnected" << std::endl;
        }

        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
        ::close(fd);
        clients_.erase(fd);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port" << std::endl;
        return 1;
    }

    ChatServer server(port);
    if (server.setup() < 0) {
        return 1;
    }

    server.run();
    return 0;
}
