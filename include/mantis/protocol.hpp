#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <span>
#include <libkern/OSByteOrder.h>

namespace Protocol {

template <typename T>
requires std::integral<T>
struct BigEndian {
private:
    T value;

    static constexpr T swap_if_needed(T v) {
        if constexpr (sizeof(T) == sizeof(std::uint64_t)) return OSSwapBigToHostConstInt64(v);
        if constexpr (sizeof(T) == sizeof(std::uint32_t)) return OSSwapBigToHostConstInt32(v);
        if constexpr (sizeof(T) == sizeof(std::uint16_t)) return OSSwapBigToHostConstInt16(v);
        return v;
    }
public:

    BigEndian() = default;

    // Host -> Big Endian
    BigEndian(T v) : value(swap_if_needed(v)) {}

    // Host -> Big Endian
    BigEndian& operator=(T v) {
        value = swap_if_needed(v);
        return *this;
    }

    // Big Endian -> Host
    operator T() const {
        return swap_if_needed(value);
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
    BigEndian<std::uint16_t> message_type{}; // 2
    BigEndian<std::uint16_t> message_length{}; // 2
    BigEndian<std::uint16_t> checksum{}; // 2
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

static constexpr int TICK_PRECISION = 8;

// Will use the heap, however this is just for debugging / visualization.
std::string format_price(int64_t ticks);

size_t parse_buffer(std::span<const char> buffer);
Order parse_order_message(std::span<const char> buffer);
std::uint16_t calculate_checksum(std::span<const char> payload);

}
