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

            header.message_type = static_cast<uint8_t>(Protocol::MessageType::OrderPlacement);
            header.message_length = sizeof(Protocol::WireOrderPlacement);

            Protocol::WireOrderPlacement order{};
            order.account_id = 1;
            order.price_ticks = static_cast<std::int64_t>(123.45 * Protocol::PRICE_SCALE);
            order.quantity = 1;
            std::memcpy(order.symbol.data(), "BTC", 3);

            // Calculate and attach checksum
            std::span<const char> payload_bytes(reinterpret_cast<const char*>(&order), sizeof(order));
            header.checksum = Protocol::calculate_checksum(payload_bytes);

            std::vector<char> buffer(sizeof(Protocol::WireHeader) + sizeof(Protocol::WireOrderPlacement));
            std::memcpy(buffer.data(), &header, sizeof(Protocol::WireHeader));
            std::memcpy(buffer.data() + sizeof(Protocol::WireHeader), &order, sizeof(Protocol::WireOrderPlacement));

            REQUIRE(Protocol::parse_buffer(buffer) == buffer.size());
        }
}
