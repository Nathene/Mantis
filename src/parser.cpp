#include "mantis/protocol.hpp"
#include <iostream>
#include <string_view>

namespace Protocol {
    void parse_buffer(const char *buffer) {
        auto *header = reinterpret_cast<const MessageHeader*>(buffer);

        if (header->magic_byte != MAGIC_BYTE) { return; }
        if (header->message_length > MAX_MESSAGE_SIZE) { return; }

        const char* payload_start = buffer + sizeof(MessageHeader);

        if (static_cast<MessageType>(header->message_type) == MessageType::OrderPlacement) {
            auto *order = reinterpret_cast<const OrderPlacement*>(payload_start);

            std::string_view ticker(order->symbol, 8);
            std::cout << "Buying " << order->quantity << " shares of " << ticker << " at $" << order->price << "\n";
        }


    }
} // namespace Protocol
