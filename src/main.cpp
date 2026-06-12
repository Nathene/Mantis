#include <cstdint>
#include <mantis/server.hpp>

constexpr uint16_t port = 8080;

int main() {
    mantis::Server server(port);
    server.start();
    return 0;
}
