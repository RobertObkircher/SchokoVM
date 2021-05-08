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

// TODO where to put this?
static void *dlsym_handle = nullptr;

static void *get_native_pointer(ClassFile *clazz, method_info *method) {
    if (dlsym_handle == nullptr) {
        // TODO we might need different flags
        dlsym_handle = dlopen("./libNativeLib.so", RTLD_LAZY);
        if (auto message = dlerror(); message != nullptr) {
            std::cerr << "dlopen failed: " << message << "\n";
            return nullptr;
        }
    }

    // TODO there are actually multiple names that we should try and some characters need to be escaped
    auto name = std::string("Java_");
    for (char const c : clazz->this_class->name->value) {
        name += c == '/' ? '_' : c;
    }
    name += '_';
    name += method->name_index->value;

    auto result = dlsym(dlsym_handle, name.c_str());
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "Could not find native function " << name << ": " << message << "\n";
        return nullptr;
    }
    return result;
}

std::optional<NativeFunction> NativeFunction::create(ClassFile *clazz, method_info *method) {
    assert(method->is_native());

    NativeFunction result{};
    result.m_function_pointer = get_native_pointer(clazz, method);
    if (result.m_function_pointer == nullptr) {
        // TODO set exception and return none
        abort();
    }

    result.m_argument_types.push_back(&ffi_type_pointer); // JNIEnv *env
    result.m_argument_types.push_back(&ffi_type_pointer); // jclass or jobject

    ffi_type *return_type;
    try {
        u2 offset = 0;
        if (!method->is_static()) {
            result.m_argument_offsets.push_back(offset); // "this" is not included in the descriptor
            ++offset;
        }

        MethodDescriptorParts parts{method->descriptor_index->value.c_str()};
        for (; !parts->is_return; ++parts) {
            ffi_type *t = ffi_type_from_char(parts->type_name[0]);
            assert(t);
            result.m_argument_types.push_back(t);
            result.m_argument_offsets.push_back(offset);
            offset += parts->category;
        }

        return_type = ffi_type_from_char(parts->type_name[0]);
        assert(return_type);
    } catch (ParseError &e) {
        abort();
    }

    ffi_status status = ffi_prep_cif(
            &result.m_cif,
            FFI_DEFAULT_ABI,
            static_cast<unsigned int>(result.m_argument_types.size()),
            return_type,
            result.m_argument_types.data()
    );

    if (status != FFI_OK) {
        std::cerr << "Failed to prepare ffi call\n";
        abort();
    }

    return {std::move(result)};
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

