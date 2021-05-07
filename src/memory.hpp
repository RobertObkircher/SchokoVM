#ifndef SCHOKOVM_MEMORY_HPP
#define SCHOKOVM_MEMORY_HPP

#include "classfile.hpp"
#include "future.hpp"
#include "types.hpp"

struct Reference {
    void *memory;

    bool operator==(const Reference &rhs) const { return memory == rhs.memory; };
};

Reference const JAVA_NULL = Reference{nullptr};

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

struct Object {
    ClassFile *clazz;
};

#endif //SCHOKOVM_MEMORY_HPP
