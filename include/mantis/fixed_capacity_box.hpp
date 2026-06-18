#include <utility>

namespace mantis {

    template <typename T, std::size_t Size, std::size_t Align>
    class FixedCapacityBox {
        public:
            FixedCapacityBox() {
                static_assert(sizeof(T) <= Size, "T must be smaller than or equal to Size");
                static_assert(alignof(T) == Align, "Align must be a multiple of sizeof(T)");
                new (data()) T();
            }
            ~FixedCapacityBox() {
                get().~T();
            }

            FixedCapacityBox(const FixedCapacityBox& other) {
                new (data()) T(other.get());
            }

            FixedCapacityBox(FixedCapacityBox&& other) noexcept {
                new (data()) T(std::move(other.get()));
            }

            FixedCapacityBox& operator=(const FixedCapacityBox& other) {
                if (this != &other) {
                    // safely construct a temporary T from the other's value
                    alignas(Align) std::byte temp_storage_[Size];
                    new (temp_storage_) T(other.get());

                    // destroy our old object
                    get().~T();

                    // move the temporary T into our main storage slot
                    new (data()) T(std::move(*reinterpret_cast<T*>(temp_storage_)));
                    reinterpret_cast<T*>(temp_storage_)->~T();
                }
                return *this;
            }

            FixedCapacityBox& operator=(FixedCapacityBox&& other) noexcept {
                if (this != &other) {
                    // destroy current managed object
                    get().~T();

                    // move-construct the new object from other
                    new (data()) T(std::move(other.get()));
                }
                return *this;
            }


            T& get() { return *reinterpret_cast<T*>(data()); }
            const T& get() const { return *reinterpret_cast<const T*>(data()); }



        private:
            std::byte* data() { return storage_; }
            const std::byte* data() const { return storage_; }
            alignas(Align) std::byte storage_[Size];
    };
}
