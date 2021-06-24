#ifndef SCHOKOVM_STRING_HPP
#define SCHOKOVM_STRING_HPP

#include <locale>
#include <codecvt>
#include <bitset>

#include "memory.hpp"

struct ModifiedUtf8 {
    u1 chars[4]; // zero terminated
    size_t count; // 1,2,3

    explicit ModifiedUtf8(char16_t code_point);
};

struct JavaString {
    explicit JavaString(const Reference &instance) : instance(instance) {
        assert(instance != JAVA_NULL);
        assert(value() != JAVA_NULL);
    }

    [[nodiscard]] Reference reference() const { return instance; }

    // java fields:

    Reference &value() { return instance.data<Value>()[0].reference; }

    enum Kind : s1 {
        Latin = 0,
        Utf16 = 1,
    };

    Kind &coder() { return reinterpret_cast<Kind &>(instance.data<Value>()[1].s4); }

    // utiltiy functions:

    s4 array_length() {
        if (coder() == Utf16) {
            assert(value().object()->length % 2 == 0);
        }
        return value().object()->length;
    }

    size_t count_utf8() {
        if (coder() == Latin) {
            return static_cast<size_t>(array_length());
        } else {
            size_t length = 0;
            for (const auto &c : view16()) {
                length += ModifiedUtf8{c}.count;
            }
            return length;
        }
    }

    s4 length() {
        return array_length() >> coder();
    }

    // returns number of bytes written (including zero at the end)
    size_t copy_to_modified_utf8_buffer(s4 start_16, s4 length_16, u1 *optional_buffer = nullptr);

    // returns number of chars written, not zero terminated
    size_t copy_to_buffer(s4 start_16, s4 length_16, u2 *optional_buffer = nullptr);

private:
    Reference const instance;

    std::basic_string_view<u1> view8() {
        return {value().data<u1>(), value().data<u1>() + array_length()};
    };

    std::basic_string_view<u2> view16() {
        return {reinterpret_cast<u2 *>(value().data<u1>()),
                reinterpret_cast<u2 *>(value().data<u1>() + array_length())};
    }
};

#endif //SCHOKOVM_STRING_HPP
