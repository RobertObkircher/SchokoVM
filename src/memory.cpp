#include "memory.hpp"

#include "classfile.hpp"

Reference Heap::new_instance(ClassFile *clazz) {
    return allocate_array<Value>(clazz, (s4) clazz->total_instance_field_count);
}
