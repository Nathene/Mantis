#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <span>
#include <libkern/OSByteOrder.h>
#include <sys/types.h>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <string>

namespace Protocol {

    template <typename T>
    struct BigEndian {
        T value;
        operator T() const {
            if constexpr (sizeof(T) == 8) return OSSwapBigToHostConstInt64(value);
            if constexpr (sizeof(T) == 4) return OSSwapBigToHostConstInt32(value);
            if constexpr (sizeof(T) == 2) return OSSwapBigToHostConstInt16(value);
            return value;
        }
    };

    static constexpr std::int64_t PRICE_SCALE = 100000000;
    static constexpr std::uint16_t MAX_MESSAGE_SIZE = 1000;
    static constexpr std::uint8_t MAGIC_BYTE = 0xCC;
    static constexpr size_t SYMBOL_SIZE = 8;

    #pragma pack(push, 1)

    // Use BigEndian to conform with the wire protocol
    struct WireHeader {
        std::uint8_t magic_byte{}; // 1
        std::uint8_t version{}; // 1
        std::uint16_t message_type{}; // 2
        std::uint16_t message_length{}; // 2
        std::uint16_t checksum{}; // 2
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // Use BigEndian to conform with the wire protocol
    struct WireOrderPlacement {
        BigEndian<std::uint64_t> account_id{}; // 8
        BigEndian<std::int64_t> price_ticks{}; // 8
        BigEndian<std::int32_t> quantity{}; // 4
        BigEndian<std::uint8_t> side{}; // 1
        BigEndian<std::uint8_t> order_type{}; // 1
        std::array<char, SYMBOL_SIZE> symbol{}; // 16
        BigEndian<std::uint16_t> reserved{}; // 2
    };
    #pragma pack(pop)

    enum class MessageType : std::uint8_t { Unknown = 0, OrderPlacement = 1 };
    static inline MessageType to_message_type(std::uint8_t type) {
        return static_cast<MessageType>(type);
    }

    struct Order {
        std::uint64_t account_id{};
        std::int64_t price{};
        std::uint32_t quantity{};
        std::string symbol{};
    };

    inline Order map_to_logic(const WireOrderPlacement& wire) {
        return Order{
            .account_id = wire.account_id,
            .price = wire.price_ticks,
            .quantity = static_cast<std::uint32_t>(wire.quantity),
            .symbol = std::string(wire.symbol.data(), SYMBOL_SIZE)
        };
    }

    inline std::string format_price(int64_t ticks) {
        // handle negative ticks
        bool negative = ticks < 0;
        std::int64_t abs_ticks = std::abs(ticks);

        std::int64_t whole = abs_ticks / PRICE_SCALE;
        std::int64_t fractional = abs_ticks % PRICE_SCALE;

        std::ostringstream oss;
        if (negative) oss << "-";

        // 8 digit precision
        oss << whole << "." << std::setfill('0') << std::setw(8) << fractional;

        std::string s = oss.str();
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s += '0';

        return s;
    }

    size_t parse_buffer(std::span<const char> buffer);
}
