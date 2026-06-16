#include "mantis/protocol.hpp"
#include <arpa/inet.h> // For ntohs
#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

#include <libkern/OSByteOrder.h>

namespace Protocol {
    size_t parse_buffer(std::span<const char> buffer) {
        if (buffer.size() < sizeof(WireHeader)) return 0;

        WireHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(WireHeader));

        std::uint8_t msg_type = header.message_type;
        std::uint16_t msg_len = header.message_length;

        if (header.magic_byte != MAGIC_BYTE || msg_len > MAX_MESSAGE_SIZE) return 0;

        size_t total_size = sizeof(WireHeader) + msg_len;
        if (buffer.size() < total_size) return 0;

        std::span<const char> payload(buffer.data() + sizeof(WireHeader), msg_len);
        std::uint16_t expected_checksum = calculate_checksum(payload);

        std::uint16_t received_checksum = header.checksum;

        if (expected_checksum != received_checksum) {
            // If they sent Little Endian, the checksums won't match.
            // Reject the message immediately.
            return 0;
        }

        if (Protocol::to_message_type(msg_type) == MessageType::OrderPlacement) {
            WireOrderPlacement wire_payload;
            std::memcpy(&wire_payload, buffer.data() + sizeof(WireHeader), sizeof(WireOrderPlacement));

            Order order = map_to_logic(wire_payload);

            std::cout << "Buying " << order.quantity
                        << " shares of " << order.symbol
                        << " at $" << format_price(order.price)
                        << "\n";
        }
        return total_size;
    }

    Order parse_order_message(std::span<const char> buffer) {
        WireOrderPlacement wire_payload;
        std::memcpy(&wire_payload, buffer.data() + sizeof(WireHeader), sizeof(WireOrderPlacement));
        return map_to_logic(wire_payload);
    }
} // namespace Protocol
