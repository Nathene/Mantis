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
#include <span>
#include <array>

// MacOS kqueue headers
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

namespace mantis {
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
        EV_SET(&change_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
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
            std::span<char> write_span = arena.get_write_span();

            ssize_t bytes_read = read(client_fd, write_span.data(), write_span.size());
            if (bytes_read > 0) {
                if (!arena.advance(static_cast<size_t>(bytes_read))) {
                    std::cerr << "[Mantis] Arena overflow: " << bytes_read << " bytes\n";
                    break;
                }

                auto message = write_span.subspan(0, static_cast<size_t>(bytes_read));
                size_t consumed = Protocol::parse_buffer(message);
                if (consumed > 0) {
                    auto order = Protocol::parse_order_message(message);
                    std::cout << "Buying " << order.quantity << " shares..." << "\n";
                    arena.compact(consumed);
                }
            }
            else if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                std::cerr << "[Mantis] Failed to read from client: " << errno << '\n';
                break;
            }
            else if (bytes_read == 0) {
                std::cout << "[Mantis] Graceful EOF from client FD " << client_fd << '\n';
                close(client_fd);
                break;
            }
        }
    }

    void Server::start() {
        setup_socket();
        setup_kqueue();

        const int MAX_EVENTS = 64;

        std::array<struct kevent, MAX_EVENTS> events{};

        std::cout << "[Mantis] Engine online. Server started on port " << port << "...\n";

        while (true) {
            int num_events = kevent(kq_fd, nullptr, 0, events.data(), MAX_EVENTS, nullptr);
            if (num_events == 0) {
                continue;
            }

            std::span<struct kevent> active_events(events.data(), static_cast<size_t>(num_events));

            for (const auto& event : active_events) {
                if (event.ident == static_cast<uintptr_t>(server_fd)) {
                    accept_connections();
                } else {
                    drain_client(static_cast<int>(event.ident));
                }
            }
        }
    }
}
