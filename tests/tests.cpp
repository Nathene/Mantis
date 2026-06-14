#include <libkern/OSByteOrder.h>
#include <catch2/catch_test_macros.hpp>
#include "mantis/arena.hpp"
#include "mantis/protocol.hpp"
#include <array>
#include <vector>
#include <cstring>

// Arena Tests
TEST_CASE("Arena basic operations", "[arena]") {
    mantis::Arena arena(1024);
    // Use the span, not the raw pointer
    REQUIRE(arena.get_write_span().data() != nullptr);
    REQUIRE(arena.advance(10) == true);
}

TEST_CASE("Arena compaction", "[arena]") {
    mantis::Arena arena(100);

    // 1. Fill arena
    auto span1 = arena.get_write_span();
    std::memcpy(span1.data(), "AAAAABBBBB", 10);
    REQUIRE(arena.advance(10));

    // 2. Consume "AAAAA"
    arena.compact(5);

    // 3. Verify remaining data is "BBBBB"
    auto filled = arena.get_filled_span();
    REQUIRE(std::string_view(filled.data(), filled.size()) == "BBBBB");
}

TEST_CASE("Arena overflow protection", "[arena]") {
    mantis::Arena arena(10);
    REQUIRE(arena.advance(10) == true);
    REQUIRE(arena.advance(1) == false);
}

// Protocol Tests
TEST_CASE("Protocol parsing", "[protocol]") {

    SECTION("Reject incomplete header") {
        std::array<char, 9> raw = {'t', 'o', 'o', '_', 's', 'h', 'o', 'r', 't'};
        REQUIRE(Protocol::parse_buffer(raw) == 0);
    }

    SECTION("Successful full message parsing") {
        Protocol::WireHeader header{};
        header.magic_byte = Protocol::MAGIC_BYTE;
        header.version = 1;

        // EXPLICITLY use Big-Endian for the wire
        header.message_type = OSSwapHostToBigInt16(static_cast<uint16_t>(Protocol::MessageType::OrderPlacement));
        header.message_length = OSSwapHostToBigInt16(sizeof(Protocol::WireOrderPlacement));

        Protocol::WireOrderPlacement order{};
        order.account_id = OSSwapBigToHostConstInt64(98765);
        order.price_ticks = OSSwapBigToHostConstInt64(110000.32 * Protocol::PRICE_SCALE);
        order.quantity = OSSwapBigToHostConstInt32(10);
        std::memcpy(order.symbol.data(), "BTC", 3);

        std::vector<char> buffer(sizeof(Protocol::WireHeader) + sizeof(Protocol::WireOrderPlacement));
        std::memcpy(buffer.data(), &header, sizeof(Protocol::WireHeader));
        std::memcpy(buffer.data() + sizeof(Protocol::WireHeader), &order, sizeof(Protocol::WireOrderPlacement));

        REQUIRE(Protocol::parse_buffer(buffer) == buffer.size());
    }
}
