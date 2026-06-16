#include "mantis/protocol.hpp"

namespace Protocol {

std::uint16_t calculate_checksum(std::span<const char> payload) {
    std::uint16_t checksum = 0;
    for (char c : payload) {
        checksum += static_cast<std::uint8_t>(c);
    }
    return checksum;
}

}
