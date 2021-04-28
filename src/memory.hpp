#ifndef SCHOKOVM_MEMORY_HPP
#define SCHOKOVM_MEMORY_HPP

#include <memory>
#include <utility>

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
    size_t flags;

    explicit Object(ClassFile *clazz) : clazz(clazz), flags(0) {}
};

struct Instance : Object {
    std::vector<Value> fields;

    explicit Instance(ClassFile *clazz) : Object(clazz), fields({}) {
        fields.resize(clazz->fields.size());
    }
};

static_assert(sizeof (Instance) == 40);

template<class T>
struct Array : Object {
    std::vector<T> elements;

    explicit Array(ClassFile *clazz, s4 size) : Object(clazz), elements({}) {
        elements.resize(size);
    }
};

// for now we will individually allocate objects via malloc
struct Heap {
    // for now we make one allocation per object and keep it in a list
    // separate arrays, so we do not need a virtual destructor
    std::vector<std::unique_ptr<Instance>> instances;
    std::vector<std::unique_ptr<Array<s1>>> byte_arrays;
    std::vector<std::unique_ptr<Array<s2>>> short_arrays;
    std::vector<std::unique_ptr<Array<s4>>> int_arrays;
    std::vector<std::unique_ptr<Array<s8>>> long_arrays;
    std::vector<std::unique_ptr<Array<Object *>>> object_arrays;

    Instance *allocate_instance(ClassFile *clazz) {
        instances.push_back(std::make_unique<Instance>(clazz));
        return instances[instances.size() - 1].get();
    }

    Array<s4> *allocate_int_array(s4 size) {
        ClassFile *int_array = nullptr; // should be known here
        int_arrays.push_back(std::make_unique<Array<s4>>(int_array, size));
        return int_arrays[int_arrays.size() - 1].get();
    }

};

#endif //SCHOKOVM_MEMORY_HPP
