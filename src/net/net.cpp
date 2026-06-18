#include "mantis/net.hpp"

#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string_view>
#include "mantis/types.hpp"
#include <iostream>

namespace mantis {

    struct InsideSockAddr {
        sockaddr_in addr;
    };

    void Result::expect(std::string_view msg) const {
        if (!is_ok()) {
            std::cerr << msg << " (Status Code: " << static_cast<int>(code)
                      << ", errno: " << errno << " - " << std::strerror(errno) << ")\n";
            std::exit(EXIT_FAILURE);
        }
    }

    Result net::bind_socket(FileDescriptor fd, Port port) {
        sockaddr_in addr{};

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = port;

        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            return Result{StatusCode::SocketBindFailed};
        }

        return {StatusCode::Success};

    }

    Result net::set_reuse_addr(FileDescriptor fd) {
        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            return Result{StatusCode::SocketReuseAddrFailed};
        }

        return {StatusCode::Success};
    }

    Result net::listen_socket(FileDescriptor fd) {
        if (listen(fd, SOMAXCONN) < 0) {
            return Result{StatusCode::SocketListenFailed};
        }

        return {StatusCode::Success};
    }

    Result net::make_socket_non_blocking(FileDescriptor fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return {StatusCode::FcntlGetFailed};

        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            return {StatusCode::FcntlSetFailed};
        }

        return {StatusCode::Success};
    }
}
