#include "mantis/arena.hpp"
#include "mantis/protocol.hpp"
#include <array>
#include <iostream>
#include <cassert>
#include <cstddef>

// Simple test runner
#define TEST(name) void name()

TEST(test_arena_basic) {
    mantis::Arena arena(1024);
    assert(arena.get_write_ptr() != nullptr);

    assert(arena.advance(10));

    std::cout << "[PASS] test_arena_basic\n";
}

TEST(test_arena_compaction) {
    mantis::Arena arena(100);

    // 1. Fill arena with "AAAAABBBBB"
    auto span1 = arena.get_write_span();
    std::memcpy(span1.data(), "AAAAABBBBB", 10);
    assert(arena.advance(10));

    // 2. Consume "AAAAA" (first 5 bytes)
    std::string test_string{"AAAAA"};
    arena.compact(test_string.size());

    // 3. Verify remaining data is "BBBBB" at the start
    assert(std::string_view(arena.get_filled_span().data(), arena.get_filled_span().size()) == "BBBBB");

    std::cout << "[PASS] test_arena_compaction\n";
}

TEST(test_arena_overflow) {
    mantis::Arena arena(10);
    assert(arena.advance(10)); // Should work
    assert(!arena.advance(1)); // Should fail (overflow)
    std::cout << "[PASS] test_arena_overflow\n";
}

TEST(test_protocol_incomplete) {
    // Test that we return 0 when buffer is too small for header
    std::array<char, 9> raw = {'t', 'o', 'o', '_', 's', 'h', 'o', 'r', 't'};
    std::span<char> span{raw};

    assert(Protocol::parse_buffer(span) == 0);

    std::cout << "[PASS] test_protocol_incomplete\n";
}

// TODO Migrate to catch2

int main() {
    std::cout << "Running Mantis Test Suite...\n";
    // Arena
    test_arena_basic();
    test_arena_compaction();
    test_arena_overflow();

    // Protocol
    test_protocol_incomplete();
    std::cout << "All tests passed!\n";
    return 0;
}
