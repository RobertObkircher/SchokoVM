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
    u4 flags;
    s4 length;

    enum Flags : u4 {
        GC_BIT = 1,
    };

    [[nodiscard]] bool gc_bit() const {
        return (flags & GC_BIT) != 0;
    }

    void gc_bit(bool value) {
        if (value) {
            flags |= GC_BIT;
        } else {
            flags &= ~GC_BIT;
        }
    }
};

struct CONSTANT_Utf8_info;

struct Heap {
    static inline Heap &get() { return the_heap; }

    struct OperatorDeleter {
        void operator()(void *pointer) {
            operator delete(pointer);
        }
    };

    std::vector<std::unique_ptr<void, OperatorDeleter>> allocations;
    std::vector<std::unique_ptr<ClassFile>> classes;
    std::unordered_map<std::string, Reference> interned_strings;

    // Returns an object of the same class and structure (length), but doens't copy any data
    Reference clone(Reference const &object);

    Reference new_instance(ClassFile *clazz);

    template<typename Element>
    Reference new_array(ClassFile *clazz, s4 length) {
        return allocate_array<Element>(clazz, length);
    }

    Reference allocate_array(ClassFile *clazz, size_t total_size, s4 length);

    template<class Element>
    Reference allocate_array(ClassFile *clazz, s4 length) {
        assert(length >= 0);
        size_t size = offset_of_array_after_header<Object, Element>() + static_cast<size_t>(length) * sizeof(Element);
        return allocate_array(clazz, size, length);
    }

    Reference make_string(std::string const &modified_utf8);

    Reference make_string(std::u16string_view const &data);

    Reference load_string(CONSTANT_Utf8_info *data);

    ClassFile *allocate_class();

    size_t garbage_collection(std::vector<struct Thread *> &threads);

private:
    static Heap the_heap;

    bool gc_bit_unmarked = false;

    bool all_objects_are_unmarked();

    void mark(std::vector<struct Thread *> &threads, bool gc_bit_marked);

    size_t sweep(bool unmarked);
};

#endif //SCHOKOVM_MEMORY_HPP
