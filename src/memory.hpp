#ifndef SCHOKOVM_MEMORY_HPP
#define SCHOKOVM_MEMORY_HPP

#include <memory>

#include "classfile.hpp"
#include "future.hpp"
#include "types.hpp"

struct Object;

union Value {
    // for dummy elements
    Value() : s8(0) {}

    explicit Value(::u4 u4) : s4(future::bit_cast<::s4>(u4)) {}

    explicit Value(::s4 s4) : s4(s4) {}

    explicit Value(::u8 u8) : s8(future::bit_cast<::s8>(u8)) {}

    explicit Value(::s8 s8) : s8(s8) {}

    explicit Value(float float_) : float_(float_) {}

    explicit Value(double double_) : double_(double_) {}

    explicit Value(Object *reference) : reference(reference) {}

    ::s4 s4;
    ::s8 s8;
    float float_;
    double double_;
    Object *reference;
};

template<class Header, class Element>
consteval size_t offset_of_array_after_header() {
    // see also std::align
    size_t result = (sizeof(Header) - 1u + alignof(Element)) & -alignof(Element);
    assert(result >= sizeof(Header));
    return result;
}

struct Object {
    // instead of a virtual function pointer table we just store the class
    ClassFile *clazz;
    s4 flags;
    s4 size; // for arrays

    // For instances Element=Value
    // For arrays Element=s1,s2,s4,s8,Object*
    template<typename Element>
    inline Element *data() {
        return reinterpret_cast<Element *>(reinterpret_cast<char *>(this) +
                                           offset_of_array_after_header<Object, Element>());
    }
};

static_assert(std::is_standard_layout<Object>() && std::is_trivial<Object>(), "must be pod");
static_assert(std::is_standard_layout<Value>()); // not sure if it is a problem that it isn't trivial

// for now we will individually allocate objects via malloc
struct Heap {
    struct OperatorDeleter {
        void operator()(void *pointer) {
            operator delete(pointer);
        }
    };

    // for now we make one allocation per object and keep it in a list
    std::vector<std::unique_ptr<void, OperatorDeleter>> instances;

    Object *allocate_instance(ClassFile *clazz) {
        return allocate_array<Value>(clazz, (s4) clazz->fields.size());
    }

    template<class Element>
    Object *allocate_array(ClassFile *clazz, s4 size) {
        assert(size >= 0);

        const size_t offset = offset_of_array_after_header<Object, Element>();;
        std::unique_ptr<void, OperatorDeleter> pointer(
                operator new(offset + static_cast<size_t>(size) * sizeof(Element)));

        auto *object = reinterpret_cast<Object *>(pointer.get());
        object->clazz = clazz;
        object->size = size;

        instances.push_back(std::move(pointer));

        return object;
    }

};

#endif //SCHOKOVM_MEMORY_HPP
