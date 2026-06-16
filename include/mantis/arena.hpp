#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <memory>
#include <span>

namespace mantis {

    class Arena {
        std::unique_ptr<char[]> data;
        size_t capacity{};
        size_t current_offset{};

    public:
        Arena(size_t size) : data(std::make_unique_for_overwrite<char[]>(size)), capacity(size) {}

        [[nodiscard]] std::span<char> get_write_span() noexcept {
            return {data.get() + current_offset, capacity - current_offset};
        }
        [[nodiscard]] bool advance(size_t size) noexcept {
            if (current_offset + size > capacity) {
                return false;
            }
            current_offset += size;
            return true;
        }
        [[nodiscard]] std::span<char> get_filled_span() const noexcept {
            return {data.get(), current_offset};
        }

        void reset() { current_offset = 0; }


        void compact(size_t consumed_bytes) {
            if (consumed_bytes == 0) return;

            assert(consumed_bytes <= current_offset && "consumed_bytes must be less than or equal to current_offset");

            size_t remaining_bytes = current_offset - consumed_bytes;

            std::memmove(data.get(), data.get() + consumed_bytes, remaining_bytes);
            current_offset = remaining_bytes;
        }
    };
}
