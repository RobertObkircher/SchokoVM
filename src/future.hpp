#ifndef SCHOKOVM_FUTURE_HPP
#define SCHOKOVM_FUTURE_HPP

#include <cstring>
#include <type_traits>

// C++20's std::bit_cast
namespace future {
    template<class To, class From>
    typename std::enable_if_t<
            sizeof(To) == sizeof(From) &&
            std::is_trivially_copyable_v<From> &&
            std::is_trivially_copyable_v<To>,
            To>
    bit_cast(const From &src) noexcept {
        static_assert(std::is_trivially_constructible_v<To>,
                      "This implementation additionally requires destination type to be trivially constructible");

        To dst;
        std::memcpy(&dst, &src, sizeof(To));
        return dst;
    }
}


#endif //SCHOKOVM_FUTURE_HPP
