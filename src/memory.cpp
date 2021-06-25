#include "memory.hpp"

#include <locale>
#include <codecvt>
#include "classfile.hpp"
#include "classloading.hpp"
#include "string.hpp"

Heap Heap::the_heap;

Reference Heap::clone(Reference const &original) {
    auto clazz = original.object()->clazz;
    auto copy = allocate_array(original.object()->clazz,
                               clazz->offset_of_array_after_header +
                               clazz->element_size * static_cast<size_t>(original.object()->length),
                               original.object()->length);

    memcpy(reinterpret_cast<char *>(copy.memory) + clazz->offset_of_array_after_header,
           reinterpret_cast<char *>(original.memory) + clazz->offset_of_array_after_header,
           clazz->element_size * static_cast<size_t>(original.object()->length));
    return copy;
}

Reference Heap::new_instance(ClassFile *clazz) {
    return allocate_array<Value>(clazz, (s4) clazz->total_instance_field_count);
}

Reference Heap::allocate_array(ClassFile *clazz, size_t total_size, s4 length) {
    assert(length >= 0);
    std::unique_ptr<void, OperatorDeleter> pointer(operator new(total_size));

    // TODO is this good enough to initialize all primitive java fields?
    // from cppreference calloc: Initialization to all bits zero does not guarantee that a floating-point or a pointer would be initialized to 0.0 and the null pointer value, respectively (although that is true on all common platforms)
    memset(pointer.get(), 0, total_size);

    Reference reference{pointer.get()};
    auto *object = reference.object();
    object->clazz = clazz;
    object->length = length;

    allocations.push_back(std::move(pointer));

    return reference;
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
    JavaString{reference}.coder() = JavaString::Utf16;

    return reference;
}

Reference Heap::make_string(std::string const &modified_utf8) {
    std::u16string string_utf16;
    string_utf16.reserve(modified_utf8.size());
    {
        for (size_t i = 0; i < modified_utf8.length(); ++i) {
            u1 x = static_cast<u1>(modified_utf8[i]);

            if ((x & 0b10000000) == 0) { // copy 1 byte over
                assert(x != 0);
                string_utf16.push_back(x);
            } else if ((x & 0b11100000) == 0b11000000) { // copy 2 byte over
                u1 y = static_cast<u1>(modified_utf8[++i]);
                string_utf16.push_back(static_cast<u2>(((x & 0x1f) << 6) + (y & 0x3f)));
            } else if ((x & 0b11110000) == 0b11100000) { // copy 3 byte over
                u1 y = static_cast<u1>(modified_utf8[++i]);
                u1 z = static_cast<u1>(modified_utf8[++i]);
                string_utf16.push_back(static_cast<u2>(((x & 0xf) << 12) + ((y & 0x3f) << 6) + (z & 0x3f)));
            } else {
                throw std::runtime_error("Invalid byte in modified utf8 string: " + std::to_string((int) x));
            }
        }
    }

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
