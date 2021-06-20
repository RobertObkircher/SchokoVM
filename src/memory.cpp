#include "memory.hpp"

#include <locale>
#include <codecvt>
#include "classfile.hpp"
#include "classloading.hpp"

Heap Heap::the_heap;

Reference Heap::new_instance(ClassFile *clazz) {
    return allocate_array<Value>(clazz, (s4) clazz->total_instance_field_count);
}

Reference Heap::make_string(std::u16string_view const &string_utf16) {
    size_t string_utf16_length = string_utf16.size() * sizeof(char16_t);

    auto charArray = new_array<u1>(BootstrapClassLoader::primitive(Primitive::Byte).array,
                                   static_cast<s4>(string_utf16_length));
    std::memcpy(charArray.data<u1>(), string_utf16.data(), string_utf16_length);

    auto *string_clazz = BootstrapClassLoader::constants().java_lang_String;
    [[maybe_unused]] auto const &value_field = string_clazz->fields[0];
    assert(value_field.name_index->value == "value" && value_field.descriptor_index->value == "[B");
    [[maybe_unused]] auto const &coder_field = string_clazz->fields[1];
    assert(coder_field.name_index->value == "coder" && coder_field.descriptor_index->value == "B");

    auto reference = new_instance(string_clazz);
    reference.data<Value>()[0] = Value{charArray};
    // as declared in String.java: LATIN1 = 0, UTF16 = 1
    reference.data<Value>()[1] = Value{(s1) 1};

    return reference;
}

Reference Heap::make_string(std::string const &modified_utf8) {
    // java uses modified utf8, but the conversion below doesn't like that
    std::string string_utf8;
    {
        string_utf8.reserve(modified_utf8.length());
        for (size_t i = 0; i < modified_utf8.length(); ++i) {
            u1 c = static_cast<u1>(modified_utf8[i]);

            if (c == 0b11101101) {
                u1 v = static_cast<u1>(modified_utf8[i + 1]);
                u1 w = static_cast<u1>(modified_utf8[i + 2]);
                u1 x = static_cast<u1>(modified_utf8[i + 3]);
                u1 y = static_cast<u1>(modified_utf8[i + 4]);
                u1 z = static_cast<u1>(modified_utf8[i + 5]);

                if (((v & 0xf0) == 0b10100000) && ((w & 0b11000000) == 0b10000000)
                    && (x == 0b11101101) && ((y & 0xf0) == 0b10110000) &&
                    ((z & 11000000) == 0b10000000)) {
                    int codepoint =
                            0x10000 + ((v & 0x0f) << 16) + ((w & 0x3f) << 10) + ((y & 0x0f) << 6) +
                            (z & 0x3f);
                    // convert into the 4-byte utf8 variant
                    string_utf8.push_back(static_cast<char>(0b11110000 | ((codepoint >> 18) & 0b1111)));
                    string_utf8.push_back(
                            static_cast<char>(0b10000000 | ((codepoint >> 12) & 0b111111)));
                    string_utf8.push_back(
                            static_cast<char>(0b10000000 | ((codepoint >> 6) & 0b111111)));
                    string_utf8.push_back(static_cast<char>(0b10000000 | ((codepoint) & 0b111111)));
                    i += 5;
                    continue;
                }
            }

            if ((c & 0b10000000) == 0) { // copy 1 byte over
                string_utf8.push_back(static_cast<char>(c));
            } else if ((c & 0b11100000) == 0b11000000) { // copy 2 byte over
                string_utf8.push_back(static_cast<char>(c));
                string_utf8.push_back(modified_utf8[++i]);
            } else if ((c & 0b11110000) == 0b11100000) { // copy 3 byte over
                string_utf8.push_back(static_cast<char>(c));
                string_utf8.push_back(modified_utf8[++i]);
                string_utf8.push_back(modified_utf8[++i]);
            } else {
                throw std::runtime_error(
                        "Invalid byte in modified utf8 string: " + std::to_string((int) c));
            }
        }
    }

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string string_utf16 = converter.from_bytes(string_utf8.data());

    auto reference = make_string(string_utf16);
    interned_strings[modified_utf8] = reference;

    return reference;
}

Reference Heap::load_string(CONSTANT_Utf8_info *data) {
    const std::string &modified_utf8 = data->value;
    if (interned_strings.contains(modified_utf8)) {
        return interned_strings.at(modified_utf8);
    }

    auto reference = make_string(modified_utf8);
    interned_strings[modified_utf8] = reference;

    return reference;
}

ClassFile *Heap::allocate_class() {
    classes.push_back(std::make_unique<ClassFile>());
    auto *result = classes[classes.size() - 1].get();
    // NOTE: Classes that are loaded before the constant is initalized need to be patched later
    result->header.clazz = BootstrapClassLoader::constants().java_lang_Class;
    return result;
}
