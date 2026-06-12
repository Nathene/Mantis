#pragma once

#include <cstdint>
#include <sys/types.h>

namespace Protocol {
    constexpr uint16_t MAX_MESSAGE_SIZE = 1000;
    constexpr uint8_t MAGIC_BYTE = 0xCC;

    enum class MessageType : std::uint8_t {
        Unknown = 0,
        OrderPlacement = 1,
    };

    #pragma pack(push, 1)
    struct MessageHeader {
        uint8_t magic_byte;
        uint8_t version;
        uint16_t message_type;
        uint16_t message_length;
        uint16_t checksum;
    };

    struct OrderPlacement {
        char symbol[8];
        std::uint64_t account_id;
        double price;
        std::uint32_t quantity;
        uint8_t side;
        uint8_t order_type;

        uint16_t _padding;
    };

    #pragma pack(pop)

    void parse_buffer(const char* buffer);
}
