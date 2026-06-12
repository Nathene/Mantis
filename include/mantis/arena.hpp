#pragma once

#include <cstddef>

namespace mantis {

    class Arena {
        char* data;
        size_t capacity;
        size_t current_offset;

    public:
        Arena(size_t size) : data(new char[size]), capacity(size), current_offset(0) {}

        ~Arena() {
            delete[] data;
        }

        Arena(const Arena&) = delete;
        Arena& operator=(const Arena&) = delete;

        Arena(Arena&& other) noexcept : data(other.data), capacity(other.capacity), current_offset(other.current_offset) {
            other.data = nullptr;
        }

        Arena& operator=(Arena&& other) noexcept {
            if (this != &other) {
                delete[] data;
                data = other.data;
                capacity = other.capacity;
                current_offset = other.current_offset;
                other.data = nullptr;
            }
            return *this;
        }

        char* get_write_ptr() { return data + current_offset; }
        [[nodiscard]] bool advance(size_t size) {
            if (current_offset + size > capacity) {
                return false;
            }
            current_offset += size;
            return true;
        }
        void reset() { current_offset = 0; }

    };
}
