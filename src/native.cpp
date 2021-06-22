#include <dlfcn.h>
#include <iostream>

#include "native.hpp"
#include "parser.hpp"

static ffi_type *ffi_type_from_char(char c) {
    switch (c) {
        case 'B':
            return &ffi_type_sint8;
        case 'C':
            return &ffi_type_uint16;
        case 'D':
            return &ffi_type_double;
        case 'F':
            return &ffi_type_float;
        case 'I':
            return &ffi_type_sint32;
        case 'J':
            return &ffi_type_sint64;
        case 'L':
            return &ffi_type_pointer;
        case 'S':
            return &ffi_type_sint16;
        case 'V':
            return &ffi_type_void;
        case 'Z':
            return &ffi_type_uint8;
        case '[':
            return &ffi_type_pointer;
        default:
            return nullptr;
    }
}

std::string get_jni_method_name(ClassFile *clazz, method_info *method, bool signature) {
    std::string result{"Java_"};
    result.reserve(5 + clazz->name().length() + 1 + method->name_index->value.length());
    for (char const c: clazz->name()) {
        if (c == '/') result += '_';
        else if (c == '_') result += "_1";
        else result += c;
    }
    result += "_";
    for (char const c: method->name_index->value) {
        if (c == '_') result += "_1";
        else result += c;
    }
    if (signature) {
        result += "__";
        MethodDescriptorParts parts{method->descriptor_index->value.c_str()};                                              \
        for (; !parts->is_return; ++parts) {
            for (char const c: parts->type_name) {
                if (c == '_') result += "_1";
                else if (c == ';') result += "_2";
                else if (c == '[') result += "_3";
                else if (c == '/') result += "_";
                else result += c;
            }
        }
    }
    return result;
}


void *get_native_function_pointer(ClassFile *clazz, method_info *method) {
    // TODO load this via loadLibrary
    static void *dlsym_handle = nullptr;

    if (dlsym_handle == nullptr) {
        // TODO we might need different flags
#ifdef __APPLE__
        dlsym_handle = dlopen("./libNativeLib.dylib", RTLD_LAZY | RTLD_GLOBAL);
#else
        dlsym_handle = dlopen("./libNativeLib.so", RTLD_LAZY | RTLD_GLOBAL);
#endif
        dlsym_handle = dlopen(nullptr, RTLD_LAZY);
        if (auto message = dlerror(); message != nullptr) {
            std::cerr << "dlopen failed: " << message << "\n";
            return nullptr;
        }
    }

    // TODO arbitrary unicode unicode characters aren't escaped
    //  https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/design.html
    auto name_short = get_jni_method_name(clazz, method, false);
    auto result_short = dlsym(dlsym_handle, name_short.c_str());
    if (result_short != nullptr) {
        return result_short;
    }

    auto name_full = get_jni_method_name(clazz, method, true);
    auto result_full = dlsym(dlsym_handle, name_full.c_str());
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "Could not find native function " << name_full << ": " << message << "\n";
        return nullptr;
    }
    return result_full;
}

NativeFunction::NativeFunction(method_info *method, void *function_pointer) : m_function_pointer(function_pointer),
                                                                              m_cif(), m_argument_types(),
                                                                              m_argument_offsets() {
    assert(method->is_native());

    m_argument_types.push_back(&ffi_type_pointer); // JNIEnv *env
    m_argument_types.push_back(&ffi_type_pointer); // jclass or jobject

    ffi_type *return_type;
    try {
        u2 offset = 0;
        if (!method->is_static()) {
            m_argument_offsets.push_back(offset); // "this" is not included in the descriptor
            ++offset;
        }

        MethodDescriptorParts parts{method->descriptor_index->value.c_str()};
        for (; !parts->is_return; ++parts) {
            ffi_type *t = ffi_type_from_char(parts->type_name[0]);
            assert(t);
            m_argument_types.push_back(t);
            m_argument_offsets.push_back(offset);
            offset += parts->category;
        }

        return_type = ffi_type_from_char(parts->type_name[0]);
        assert(return_type);
    } catch (ParseError &e) {
        abort();
    }

    ffi_status status = ffi_prep_cif(
            &m_cif,
            FFI_DEFAULT_ABI,
            static_cast<unsigned int>(m_argument_types.size()),
            return_type,
            m_argument_types.data()
    );

    if (status != FFI_OK) {
        std::cerr << "Failed to prepare ffi call\n";
        abort();
    }
}

void NativeFunction::prepare_argument_pointers(void **arguments, void **jni_env_argument, ClassFile **class_argument,
                                               bool use_class_argument, std::span<Value> &locals) {
    arguments[0] = jni_env_argument;
    ++arguments;

    if (use_class_argument) {
        arguments[0] = class_argument;
        ++arguments;
    }

    for (u2 offset : m_argument_offsets) {
        arguments[0] = &locals[offset];
        ++arguments;
    }
}

Value NativeFunction::call(void **arguments) {
    Value result;

    static_assert(sizeof(Value) <= sizeof(ffi_arg));
    ffi_call(&m_cif, FFI_FN(m_function_pointer), &result, arguments);

    return result;
}

