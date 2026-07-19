#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int MAX_EVENTS = 64;
constexpr size_t BUF_SIZE = 4096;

struct Client {
    std::string nick;
    std::string read_buf;
    std::string write_buf;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }

    int orig_flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, orig_flags | O_NONBLOCK);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        perror("epoll_ctl add listen");
        return 1;
    }

    std::map<int, Client> clients;

    auto broadcast = [&](int sender_fd, const std::string& msg) {
        for (auto& [fd, client] : clients) {
            if (fd == sender_fd) continue;
            client.write_buf += msg;
            if (client.write_buf.size() > 1024 * 1024) {
                client.write_buf.clear();
            }
        }
    };

    auto send_to = [&](int target_fd, const std::string& msg) {
        auto it = clients.find(target_fd);
        if (it != clients.end()) {
            it->second.write_buf += msg;
        }
    };

    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("epoll_ctl add client");
                    close(client_fd);
                    continue;
                }

                clients[client_fd] = Client();
                std::cout << "[*] Client connected from " 
                          << inet_ntoa(client_addr.sin_addr) << ":" 
                          << ntohs(client_addr.sin_port) << "\n";
            } else {
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    auto it = clients.find(fd);
                    if (it != clients.end()) {
                        std::cout << "[*] " << it->second.nick 
                                  << " disconnected (fd: " << fd << ")\n";
                        clients.erase(it);
                    }
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    continue;
                }

                if (events[i].events & EPOLLIN) {
                    char buf[BUF_SIZE];
                    ssize_t bytes_read;
                    
                    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
                        auto& client = clients[fd];
                        client.read_buf.append(buf, bytes_read);

                        size_t pos;
                        while ((pos = client.read_buf.find('\n')) != std::string::npos) {
                            std::string line = client.read_buf.substr(0, pos);
                            client.read_buf.erase(0, pos + 1);

                            if (!line.empty() && line.back() == '\r') {
                                line.pop_back();
                            }

                            if (client.nick.empty()) {
                                client.nick = line;
                                std::cout << "[*] " << client.nick 
                                          << " joined (fd: " << fd << ")\n";
                                client.write_buf += "Welcome, " + client.nick + "!\n";
                            } else if (line == "/who") {
                                std::string resp = "Users: ";
                                for (auto& [f, c] : clients) {
                                    resp += c.nick + " ";
                                }
                                resp += "\n";
                                send_to(fd, resp);
                            } else if (line == "/quit") {
                                std::cout << "[*] " << client.nick 
                                          << " quit (fd: " << fd << ")\n";
                                clients.erase(fd);
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                                close(fd);
                                break;
                            } else {
                                std::string msg = "[" + client.nick + "] " + line + "\n";
                                broadcast(fd, msg);
                            }
                        }
                    }

                    if (bytes_read == 0) {
                        auto it = clients.find(fd);
                        if (it != clients.end()) {
                            std::cout << "[*] " << it->second.nick 
                                      << " disconnected (fd: " << fd << ")\n";
                            clients.erase(it);
                        }
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                    }
                }

                if (events[i].events & EPOLLOUT) {
                    auto it = clients.find(fd);
                    if (it != clients.end() && !it->second.write_buf.empty()) {
                        ssize_t bytes_written = write(fd, 
                            it->second.write_buf.data(), 
                            it->second.write_buf.size());
                        if (bytes_written > 0) {
                            it->second.write_buf.erase(0, bytes_written);
                        }
                    }
                }
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}