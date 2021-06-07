#ifndef SCHOKOVM_MEMORY_HPP
#define SCHOKOVM_MEMORY_HPP

#include <cstddef>
#include <memory>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <string>

#include "future.hpp"
#include "types.hpp"

struct Object;
struct ClassFile;

template<class Header, class Element>
consteval size_t offset_of_array_after_header() {
    // see also std::align
    size_t result = (sizeof(Header) - 1u + alignof(Element)) & -alignof(Element);
    assert(result >= sizeof(Header));
    assert(result < sizeof(Header) + alignof(Element));
    assert(result % alignof(Element) == 0);
    return result;
}

struct Reference {
    void *memory;

    bool operator==(const Reference &rhs) const { return memory == rhs.memory; };

    [[nodiscard]] inline Object *object() const {
        return static_cast<Object *>(memory);
    }

    template<typename Element>
    [[nodiscard]] inline Element *element_at_offset(size_t offset) const {
        return static_cast<Element *>(static_cast<void *>((static_cast<char *>(memory) + offset)));
    }

    // For instances Element=Value
    // For arrays Element=s1,s2,s4,s8,Object*
    template<typename Element>
    [[nodiscard]] inline Element *data() const {
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
    s4 length;
};

struct Heap {
    static inline Heap &get() { return the_heap; }

    struct OperatorDeleter {
        void operator()(void *pointer) {
            operator delete(pointer);
        }
    };

    std::vector<std::unique_ptr<void, OperatorDeleter>> allocations;
    std::unordered_map<std::string, Reference> interned_strings;

    Reference new_instance(ClassFile *clazz);

    template<typename Element>
    Reference new_array(ClassFile *clazz, s4 length) {
        return allocate_array<Element>(clazz, length);
    }

    template<class Element>
    Reference allocate_array(ClassFile *clazz, s4 length) {
        assert(length >= 0);

        size_t size = offset_of_array_after_header<Object, Element>() + static_cast<size_t>(length) * sizeof(Element);
        std::unique_ptr<void, OperatorDeleter> pointer(operator new(size));

        // TODO is this good enough to initialize all primitive java fields?
        // from cppreference calloc: Initialization to all bits zero does not guarantee that a floating-point or a pointer would be initialized to 0.0 and the null pointer value, respectively (although that is true on all common platforms)
        memset(pointer.get(), 0, size);

        Reference reference{pointer.get()};
        auto *object = reference.object();
        object->clazz = clazz;
        object->length = length;

        allocations.push_back(std::move(pointer));

        return reference;
    }

    Reference make_string(ClassFile *string_clazz, ClassFile *byte_array_clazz, const std::string &value_utf8);

private:
    static Heap the_heap;
};

#endif //SCHOKOVM_MEMORY_HPP
