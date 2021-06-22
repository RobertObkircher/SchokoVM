#include "memory.hpp"

#include <codecvt>
#include <iostream>
#include <locale>
#include <queue>
#include <unordered_set>

#include "classfile.hpp"
#include "classloading.hpp"
#include "string.hpp"
#include "interpreter.hpp"

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

bool Heap::all_objects_are_unmarked() {
    for (const auto &item : allocations) {
        Reference reference{item.get()};
        if (reference.object()->gc_bit() != gc_bit_unmarked) {
            return false;
        }
    }
    for (const auto &item : classes) {
        if (item->header.gc_bit() != gc_bit_unmarked) {
            return false;
        }
    }
    return true;
}

namespace {
void mark_recursively(std::queue<Object *> &queue, bool gc_bit_marked, Reference to_mark,
                      std::unordered_set<Object *> const &all_object_pointers) {
    auto mark = [&queue, gc_bit_marked, &all_object_pointers](Reference reference) {
        if (reference != JAVA_NULL) {
            Object *object = reference.object();
            assert(all_object_pointers.contains(object));
            // ensure that every object is added at most once
            if (object->gc_bit() != gc_bit_marked) {
                object->gc_bit(gc_bit_marked);
                queue.push(object);
            }
        }
    };

    mark(to_mark);

    while (!queue.empty()) {
        Object *object = queue.front();
        queue.pop();

        ClassFile *clazz = object->clazz;
        mark(Reference{clazz});

        if (!clazz->is_array()) {
            // TODO we might have to initializes more classes
            assert(clazz->resolved);

            for (const auto &field : clazz->fields) {
                auto const &descriptor = field.descriptor_index->value;
                if (descriptor.starts_with("L") || descriptor.starts_with("[")) {
                    if (field.is_static()) {
                        mark(clazz->static_field_values[field.index].reference);
                    } else {
                        Reference reference{object};
                        mark(reference.data<Value>()[field.index].reference);
                    }
                }
            }
        } else {
            auto const &element = clazz->array_element_type->name();
            if (element.starts_with("L") || element.starts_with("[")) {
                Reference reference{object};
                for (s4 i = 0; i < object->length; ++i) {
                    mark(reference.data<Reference>()[i]);
                }
            }
        }

        if (clazz->name() == Names::java_lang_Class) {
            [[maybe_unused]] auto *class_instance = reinterpret_cast<ClassFile *> (object);
            // TODO Mark all referenced objects/classes (e.g. super class, interfaces, constant pool entries, ect.)
        }
    }
};
}

void Heap::mark(std::vector<Thread *> &threads, bool gc_bit_marked) {
    auto is_potential_pointer = [](void *pointer) {
        auto value = reinterpret_cast<std::uintptr_t>(pointer);
        return value != 0 && (value % alignof(std::max_align_t)) == 0;
    };

    std::unordered_set<Object *> all_object_pointers;
    for (const auto &clazz : classes) {
        auto *object = reinterpret_cast<Object *>(clazz.get());
        assert(is_potential_pointer(object));
        assert(object->clazz == BootstrapClassLoader::constants().java_lang_Class);
        all_object_pointers.insert(object);
    }
    for (const auto &allocation : allocations) {
        auto *object = reinterpret_cast<Object *>(allocation.get());
        assert(is_potential_pointer(object));
        assert(object->clazz != BootstrapClassLoader::constants().java_lang_Class);
        all_object_pointers.insert(object);
    }

    std::queue<Object *> queue;

    // we don't free classes for now so they are always in the root set
    for (const auto &clazz : classes) {
        mark_recursively(queue, gc_bit_marked, Reference{clazz.get()}, all_object_pointers);
    }

    for (const auto &thread : threads) {
        for (const auto &frame : thread->stack.frames) {
            for (const auto &value : frame.locals) {
                if (is_potential_pointer(value.reference.memory) &&
                    all_object_pointers.contains(value.reference.object())) {
                    mark_recursively(queue, gc_bit_marked, value.reference, all_object_pointers);
                }
            }
            for (const auto &value : frame.operands.subspan(0, frame.operands_top)) {
                if (is_potential_pointer(value.reference.memory) &&
                    all_object_pointers.contains(value.reference.object())) {
                    mark_recursively(queue, gc_bit_marked, value.reference, all_object_pointers);
                }
            }
        }
    }
}

size_t Heap::sweep(bool unmarked) {
    std::erase_if(interned_strings, [unmarked](auto const &a) {
        return a.second.object()->gc_bit() == unmarked;
    });

    size_t erased = std::erase_if(allocations, [unmarked](auto const &a) {
        Reference reference{a.get()};
        return reference.object()->gc_bit() == unmarked;
    });

    erased += std::erase_if(classes, [unmarked](auto const &a) {
        // TODO we don't free classes for now
        assert(a->header.gc_bit() != unmarked);
        return a->header.gc_bit() == unmarked;
    });

    return erased;
}

size_t Heap::garbage_collection(std::vector<Thread *> &threads) {
    assert(all_objects_are_unmarked());

    mark(threads, !gc_bit_unmarked);

    size_t deleted = sweep(gc_bit_unmarked);

    gc_bit_unmarked = !gc_bit_unmarked;

    assert(all_objects_are_unmarked());

    return deleted;
}
