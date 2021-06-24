#ifndef SCHOKOVM_NATIVE_HPP
#define SCHOKOVM_NATIVE_HPP

#ifdef __APPLE__
#  include <ffi/ffi.h>
#else
#  include <ffi.h>
#endif
#include <optional>
#include <span>
#include <vector>

#include "types.hpp"

union Value;
struct ClassFile;
struct method_info;

void *get_native_function_pointer(method_info *method);

struct NativeFunction {
    NativeFunction(method_info *method, void *function_pointer);

    void prepare_argument_pointers(void **arguments, void **jni_env_argument, ClassFile **class_argument,
                                   bool use_class_argument, std::span<Value> &locals);

    Value call(void **arguments);

    size_t argument_count() { return m_argument_types.size(); }

private:
    void *m_function_pointer;

    ffi_cif m_cif;
    std::vector<ffi_type *> m_argument_types;
    std::vector<u2> m_argument_offsets;
};

#endif //SCHOKOVM_NATIVE_HPP
