#include "mantis/server.hpp"
#include "mantis/arena.hpp"
#include "mantis/protocol.hpp"

#include <cstdint>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

// MacOS kqueue headers
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

namespace mantis {
    constexpr uint16_t BUF_SIZE = 4096;

    // Use ULL suffixes so the entire multiplication happens in 64-bit space
    constexpr uint64_t ARENA_SIZE = 10ULL * 1024ULL * 1024ULL;

    Server::Server(uint16_t port_num) : port(port_num), arena(ARENA_SIZE) {}

    Server::~Server() {
        if (server_fd != -1) { close(server_fd); }
        if (kq_fd != -1) { close(kq_fd); }
    }

    // TODO: Handle these ugly POSIX apis. Have them in a seperate module, where each arugment is clearly explained.
    // Calling them in this should be obvious what they do, and hide the ugly implementation elsewhere.

    void Server::make_non_blocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void Server::setup_socket() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, 0);
        make_non_blocking(server_fd);

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        int bind_result = bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
        if (bind_result == -1) {
            std::cerr << "Failed to bind socket: " << errno << '\n';
            exit(EXIT_FAILURE);
        }

        listen(server_fd, SOMAXCONN);
    }

    // TODO: Support epoll as well.

    void Server::setup_kqueue() {
        kq_fd = kqueue();

        struct kevent change_event{};
        EV_SET(&change_event, server_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
        kevent(kq_fd, &change_event, 1, nullptr, 0, nullptr);
    }

    void Server::accept_connections() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
            if (client_fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                std::cerr << "Failed to accept connection: " << errno << '\n';
                break;
            }
            make_non_blocking(client_fd);

            // register new client with kqueue
            struct kevent change_event{};
            EV_SET(&change_event, client_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
            kevent(kq_fd, &change_event, 1, nullptr, 0, nullptr);

            std::cout << "[Mantis] Accepted connection from FD " << client_fd << " : " << ntohs(client_addr.sin_port) << "\n";
        }
    }

    void Server::drain_client(int client_fd) {
        while (true) {
            char* write_ptr = arena.get_write_ptr();
            ssize_t bytes_read = read(client_fd, write_ptr, BUF_SIZE);
            if (bytes_read > 0) {
                Protocol::parse_buffer(write_ptr);
                arena.reset();
            }
            else if (bytes_read == -1) {
                std::cerr << "[Mantis] Failed to read from client: " << errno << '\n';
                break;
            }
            else if (bytes_read == 0) {
                close(client_fd);
                std::cout << "[Mantis] Client FD " << client_fd << " disconnected.\n";
                break;
            }
        }
    }

    void Server::start() {
        setup_socket();
        setup_kqueue();

        const int MAX_EVENTS = 64;
        struct kevent events[MAX_EVENTS];

        std::cout << "[Mantis] Engine online. Server started on port " << port << "...\n";

        while (true) {
            int num_events = kevent(kq_fd, nullptr, 0, events, MAX_EVENTS, nullptr);
            for (int i{}; i < num_events; ++i) {
                if (events[i].ident == server_fd) {
                    accept_connections();
                } else {
                    drain_client(events[i].ident);
                }
            }
        }
    }
}
