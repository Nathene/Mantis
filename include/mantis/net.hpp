#pragma once

#include "mantis/types.hpp"
#include "mantis/fixed_capacity_box.hpp"

#include <cerrno>
#include <cstdlib>
#include <string_view>


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

struct InsideSockAddr;

struct Result {
    StatusCode code;
    [[nodiscard]] bool is_ok() const { return code == StatusCode::Success; }
    explicit operator bool() const { return is_ok(); }
    void expect(std::string_view msg) const;
};

namespace net {
    Result bind_socket(FileDescriptor fd, Port port);
    Result set_reuse_addr(FileDescriptor fd);
    Result make_socket_non_blocking(FileDescriptor fd);
    Result listen_socket(FileDescriptor fd);
};

}
