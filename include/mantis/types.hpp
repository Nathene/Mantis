#pragma once

#include <cstdint>

namespace mantis {

struct FileDescriptor {
    int fd;
    constexpr explicit FileDescriptor(int v) : fd(v) {}
    constexpr operator int() const { return fd; }
};

struct Port {
    std::uint16_t port;
    constexpr explicit Port(std::uint16_t v) : port(v) {}
    constexpr operator std::uint16_t() const { return port; }
};

using Byte = char;

inline constexpr FileDescriptor INVALID_FD{-1};

}
