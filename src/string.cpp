#include "string.hpp"

ModifiedUtf8::ModifiedUtf8(char16_t code_point) : chars() {
    u2 c = code_point;

    auto get_byte = [c](u1 prefix, int high, int low) {
        u2 result = c >> low;
        int bit_count = high - low + 1;
        u2 mask = 0xFF >> (8 - bit_count);
        result &= mask;
        result |= prefix;
        return static_cast<u1>(result);
    };

    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-4.html#jvms-4.4.7
    if (0x0001 <= c && c <= 0x007F) {
        chars[0] = static_cast<u1>(c);
        count = 1;
    } else if (c == 0 || (0x0080 <= c && c <= 0x07FF)) {
        chars[0] = get_byte(0b11000000, 10, 6);
        chars[1] = get_byte(0b10000000, 5, 0);
        count = 2;
    } else if (0x0800 <= c && c <= 0xFFFF) {
        chars[0] = get_byte(0b11100000, 15, 12);
        chars[1] = get_byte(0b10000000, 11, 6);
        chars[2] = get_byte(0b10000000, 5, 0);
        count = 3;
    } else {
        assert(false); // unreachable
    }
}

size_t JavaString::copy_to_modified_utf8_buffer(s4 start_16, s4 length_16, u1 *optional_buffer) {
    assert(start_16 >= 0);
    assert(length_16 >= 0);

    if (coder() == Latin) {
        // we know that each latin char is one code point
        auto view = view8().substr(static_cast<size_t>(start_16), static_cast<size_t>(length_16));
        if (optional_buffer) {
            std::copy(view.begin(), view.end(), optional_buffer);
            optional_buffer[view.length()] = 0;
        }
        return view8().length() + 1;
    } else {
        size_t length = 0;
        auto view = view16().substr(static_cast<size_t>(start_16), static_cast<size_t>(length_16));
        for (const auto &c : view) {
            ModifiedUtf8 m{c};
            if (optional_buffer != nullptr) {
                for (size_t i = 0; i < m.count; ++i) {
                    *optional_buffer = m.chars[i];
                    ++optional_buffer;
                }
            }
            length += m.count;
        }
        if (optional_buffer != nullptr) {
            *optional_buffer = 0;
        }
        return length + 1;
    }
}

size_t JavaString::copy_to_buffer(s4 start_16, s4 length_16, u2 *optional_buffer) {
    assert(start_16 >= 0);
    assert(length_16 >= 0);

    if (coder() == Latin) {
        // we know that each latin char is one code point
        auto view = view8().substr(static_cast<size_t>(start_16), static_cast<size_t>(length_16));
        if (optional_buffer) {
            std::copy(view.begin(), view.end(), optional_buffer);
        }
        return view8().length();
    } else {
        auto view = view16().substr(static_cast<size_t>(start_16), static_cast<size_t>(length_16));
        if (optional_buffer != nullptr) {
            std::copy(view.begin(), view.end(), optional_buffer);
        }
        return view.length();
    }

}
