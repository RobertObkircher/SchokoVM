#ifndef SCHOKOVM_MATH_HPP
#define SCHOKOVM_MATH_HPP


template<typename T>
static inline T add_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    return future::bit_cast<T>(
            future::bit_cast<std::make_unsigned_t<T>>(a) + future::bit_cast<std::make_unsigned_t<T>>(b)
    );
}

template<typename T>
static inline T sub_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    return future::bit_cast<T>(
            future::bit_cast<std::make_unsigned_t<T>>(a) - future::bit_cast<std::make_unsigned_t<T>>(b)
    );
}

template<typename T>
static inline T mul_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    auto a_u = static_cast<std::make_unsigned_t<T>>(a);
    auto b_u = static_cast<std::make_unsigned_t<T>>(b);
    auto result = static_cast<T>(a_u * b_u);
    return result;
}

template<typename T>
static inline T div_overflow(T dividend, T divisor) {
    if (dividend == std::numeric_limits<T>::min() && divisor == -1) {
        return dividend;
    }
    // TODO test rounding
    return dividend / divisor;
}

template<typename F, typename I>
static inline I floating_to_integer(F f) {
    if (std::isnan(f)) {
        return 0;
    } else if (f > static_cast<F>(std::numeric_limits<I>::max())) {
        return std::numeric_limits<I>::max();
    } else if (f < static_cast<F>(std::numeric_limits<I>::min())) {
        return std::numeric_limits<I>::min();
    } else {
        return static_cast<I>(f);
    }
}


#endif //SCHOKOVM_MATH_HPP
