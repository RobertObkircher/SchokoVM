#include "memory.hpp"

#include <locale>
#include <codecvt>
#include "classfile.hpp"

Reference Heap::new_instance(ClassFile *clazz) {
    return allocate_array<Value>(clazz, (s4) clazz->total_instance_field_count);
}

Reference Heap::make_string(ClassFile *string_clazz, ClassFile *byte_array_clazz, const std::string &value_utf8) {
    if (interned_strings.contains(value_utf8)) {
        return interned_strings.at(value_utf8);
    }

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string string_utf16 = converter.from_bytes(value_utf8.data());
    size_t string_utf16_length = string_utf16.size() * sizeof(char16_t);

    auto charArray = new_array<u1>(byte_array_clazz, static_cast<s4>(string_utf16_length));
    std::memcpy(charArray.data<u1>(), string_utf16.data(), string_utf16_length);

    auto const &value_field = string_clazz->fields[0];
    assert(value_field.name_index->value == "value" && value_field.descriptor_index->value == "[B");
    auto const &coder_field = string_clazz->fields[1];
    assert(coder_field.name_index->value == "coder" && coder_field.descriptor_index->value == "B");

    auto reference = new_instance(string_clazz);
    reference.data<Value>()[0] = Value{charArray};
    // as declared in String.java: LATIN1 = 0, UTF16 = 1
    reference.data<Value>()[1] = Value{(s1) 1};

    interned_strings[value_utf8] = reference;

    return reference;
}
