#pragma once

#include <cstdint>
#include "mantis/arena.hpp"
#include "mantis/types.hpp"
#include "mantis/net.hpp"


namespace mantis {

// Use ULL suffixes so the entire multiplication happens in 64-bit space
inline constexpr uint64_t ARENA_SIZE = 10ULL * 1024ULL * 1024ULL;

class Server {
    FileDescriptor server_fd{INVALID_FD};
    int kq_fd{-1};

    Port port;
    Arena arena;

    // internal helpers
    inline Result make_socket_non_blocking(FileDescriptor fd);
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
