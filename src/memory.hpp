#ifndef SCHOKOVM_MEMORY_HPP
#define SCHOKOVM_MEMORY_HPP

#include <cstddef>
#include <memory>
#include <cassert>
#include <vector>

#include "future.hpp"
#include "types.hpp"

struct Object;
struct ClassFile;

template<class Header, class Element>
consteval size_t offset_of_array_after_header() {
    // see also std::align
    size_t result = (sizeof(Header) - 1u + alignof(Element)) & -alignof(Element);
    assert(result >= sizeof(Header));
    assert(result <= sizeof(Header) + alignof(std::max_align_t));
    return result;
}

struct Reference {
    void *memory;

    bool operator==(const Reference &rhs) const { return memory == rhs.memory; };

    inline Object *object() {
        return static_cast<Object *>(memory);
    }

    template<typename Element>
    inline Element *element_at_offset(size_t offset) {
        return static_cast<Element *>(static_cast<void *>((static_cast<char *>(memory) + offset)));
    }

    // For instances Element=Value
    // For arrays Element=s1,s2,s4,s8,Object*
    template<typename Element>
    inline Element *data() {
        return element_at_offset<Element>(offset_of_array_after_header<Object, Element>());
    }
};

Reference const JAVA_NULL = Reference{nullptr};

enum class ValueCategory {
    C1, C2
};

// see interpreter Stack for documentation
union Value {
    // for dummy elements
    Value() : s8(0) {}

    explicit Value(::u4 u4) : s4(future::bit_cast<::s4>(u4)) {}

    explicit Value(::s4 s4) : s4(s4) {}

    explicit Value(::u8 u8) : s8(future::bit_cast<::s8>(u8)) {}

    explicit Value(::s8 s8) : s8(s8) {}

    explicit Value(float float_) : float_(float_) {}

    explicit Value(double double_) : double_(double_) {}

    explicit Value(Reference reference) : reference(reference) {}

    ::s4 s4;
    ::s8 s8;
    float float_;
    double double_;
    Reference reference;
};

// NOTE: If this struct contains padding at the end we will *not* use it for fields/elemetns.
struct Object {
    ClassFile *clazz;
    s4 flags;
    s4 size;
};

static_assert(std::is_standard_layout<Object>() && std::is_trivial<Object>(), "must be pod");
static_assert(std::is_standard_layout<Value>()); // not sure if it is a problem that it isn't trivial

struct Heap {
    struct OperatorDeleter {
        void operator()(void *pointer) {
            operator delete(pointer);
        }
    };

    std::vector<std::unique_ptr<void, OperatorDeleter>> allocations;

    Reference new_instance(ClassFile *clazz);

    template<class Element>
    Reference allocate_array(ClassFile *clazz, s4 size) {
        assert(size >= 0);

        size_t const offset = offset_of_array_after_header<Object, Element>();;
        size_t memory_size = offset + static_cast<size_t>(size) * sizeof(Element);
        std::unique_ptr<void, OperatorDeleter> pointer(operator new(memory_size));

        // TODO is this good enough to initialize all primitive java fields?
        memset(pointer.get(), 0, memory_size);

        Reference reference{pointer.get()};
        auto *object = reference.object();
        object->clazz = clazz;
        object->size = size;

        allocations.push_back(std::move(pointer));

        return reference;
    }

};

#endif //SCHOKOVM_MEMORY_HPP
