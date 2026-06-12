#pragma once

#include <cstdint>
#include "mantis/arena.hpp"

namespace mantis {

    class Server {
        int server_fd{-1};
        int kq_fd{-1};

        uint16_t port;
        Arena arena;

        // internal helpers
        void make_non_blocking(int fd);
        void setup_socket();
        void setup_kqueue();

        // hot path event handlers
        void accept_connections();
        void drain_client(int client_fd);

    public:
        explicit Server(uint16_t port);

        ~Server();
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server& operator=(Server&&) = delete;

        void start();
    };
}
