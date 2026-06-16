#pragma once
#include "mantis/types.hpp"

#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

namespace mantis {

enum class StatusCode : std::uint8_t {
    Success,
    SocketBindFailed,
    SocketListenFailed,
    SocketReuseAddrFailed,
    FcntlGetFailed,
    FcntlSetFailed,
    ArenaOverflow,
};

struct Result {
    StatusCode code;
    [[nodiscard]] bool is_ok() const { return code == StatusCode::Success; }
    explicit operator bool() const { return is_ok(); }
};

namespace net {

    inline Result bind_socket(FileDescriptor fd, Port port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET; // IPv4
        addr.sin_addr.s_addr = INADDR_ANY; // Bind to any interface
        addr.sin_port = htons(port); // Convert port to network byte order

        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            return {StatusCode::SocketBindFailed};
        }
        return {StatusCode::Success};
    }

    inline Result set_reuse_addr(FileDescriptor fd) {
        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            return {StatusCode::SocketReuseAddrFailed};
        }
        return {StatusCode::Success};
    }

    inline Result listen_socket(FileDescriptor fd) {
            // SOMAXCONN is a system constant for the maximum queue length
            if (listen(fd, SOMAXCONN) < 0) {
                return {StatusCode::SocketListenFailed};
            }
            return {StatusCode::Success};
        }

    inline Result make_socket_non_blocking(FileDescriptor fd) {
        // We isolate the variadic fcntl call here, and nowhere else.
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return {StatusCode::FcntlGetFailed};

        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            return {StatusCode::FcntlSetFailed};
        }
        return {StatusCode::Success};
    }
    } // namespace net
}
