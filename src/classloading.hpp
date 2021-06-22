#ifndef SCHOKOVM_CLASSLOADING_HPP
#define SCHOKOVM_CLASSLOADING_HPP

#include <optional>
#include <unordered_map>

#include "classfile.hpp"
#include "interpreter.hpp"
#include "zip.hpp"

struct ClassPathEntry {
    // one of these will be empty
    std::string directory;
    ZipArchive zip;
};

struct Primitive {
    enum Type {
        Byte,
        Char,
        Double,
        Float,
        Int,
        Long,
        Short,
        Boolean,
        Void,
        TYPE_COUNT,
    };

    Type const id{};

    char const *const primitive_name{};
    ClassFile *primitive{};

    char const *const boxed_name{};
    ClassFile *boxed{};

    char const descriptor_char{};
    char const *const array_name{};
    ClassFile *array{};
};

namespace Names {
    using ccc = const char *const;
    ccc java_io_Serializable = "java/io/Serializable";
    ccc java_lang_ArithmeticException = "java/lang/ArithmeticException";
    ccc java_lang_Boolean = "java/lang/Boolean";
    ccc java_lang_Byte = "java/lang/Byte";
    ccc java_lang_Character = "java/lang/Character";
    ccc java_lang_Class = "java/lang/Class";
    ccc java_lang_ClassCastException = "java/lang/ClassCastException";
    ccc java_lang_Cloneable = "java/lang/Cloneable";
    ccc java_lang_Double = "java/lang/Double";
    ccc java_lang_Float = "java/lang/Float";
    ccc java_lang_Integer = "java/lang/Integer";
    ccc java_lang_Long = "java/lang/Long";
    ccc java_lang_NullPointerException = "java/lang/NullPointerException";
    ccc java_lang_Object = "java/lang/Object";
    ccc java_lang_Short = "java/lang/Short";
    ccc java_lang_String = "java/lang/String";
    ccc java_lang_Thread = "java/lang/Thread";
    ccc java_lang_ThreadGroup = "java/lang/ThreadGroup";
    ccc java_lang_Throwable = "java/lang/Throwable";
    ccc java_lang_Void = "java/lang/Void";
};

struct Constants {
    ClassFile *java_io_Serializable{};

    ClassFile *java_lang_Class{};
    ClassFile *java_lang_Cloneable{};
    ClassFile *java_lang_Object{};
    ClassFile *java_lang_String{};
    ClassFile *java_lang_ThreadGroup{};
    ClassFile *java_lang_Thread{};
    ClassFile *java_lang_Throwable{};

    Primitive primitives[Primitive::TYPE_COUNT] = {
            {Primitive::Byte,    "byte",    nullptr, Names::java_lang_Byte,      nullptr, 'B', "[B", nullptr},
            {Primitive::Char,    "char",    nullptr, Names::java_lang_Character, nullptr, 'C', "[C", nullptr},
            {Primitive::Double,  "double",  nullptr, Names::java_lang_Double,    nullptr, 'D', "[D", nullptr},
            {Primitive::Float,   "float",   nullptr, Names::java_lang_Float,     nullptr, 'F', "[F", nullptr},
            {Primitive::Int,     "int",     nullptr, Names::java_lang_Integer,   nullptr, 'I', "[I", nullptr},
            {Primitive::Long,    "long",    nullptr, Names::java_lang_Long,      nullptr, 'J', "[J", nullptr},
            {Primitive::Short,   "short",   nullptr, Names::java_lang_Short,     nullptr, 'S', "[S", nullptr},
            {Primitive::Boolean, "boolean", nullptr, Names::java_lang_Boolean,   nullptr, 'Z', "[Z", nullptr},
            {Primitive::Void,    "void",    nullptr, Names::java_lang_Void,      nullptr, 'V', "[V", nullptr},
    };

    void resolve_and_initialize(Thread &thread);
};

struct BootstrapClassLoader {
    static BootstrapClassLoader &get() { return the_bootstrap_class_loader; }

    static Constants const &constants() { return the_bootstrap_class_loader.m_constants; }

    void resolve_and_initialize_constants(Thread &thread) {
        the_bootstrap_class_loader.m_constants.resolve_and_initialize(thread);
    }

    static Primitive const &
    primitive(Primitive::Type id) { return the_bootstrap_class_loader.m_constants.primitives[id]; }

    void initialize_with_boot_classpath(std::string const &bootclasspath);

    ClassFile *load(std::string const &name);

    ClassFile *load_or_throw(std::string const &name);

private:
    std::vector<ClassPathEntry> m_class_path_entries;
    std::vector<char> m_buffer;
    std::unordered_map<std::string, ClassFile *> m_classes;
    Constants m_constants;

    static BootstrapClassLoader the_bootstrap_class_loader;

    ClassFile *make_builtin_class(std::string name, ClassFile *element_type);
};

Result initialize_class(ClassFile *C, Thread &thread);

inline Result initialize_class(ClassFile *C, Thread &thread, Frame &frame) {
    // quick check without lock
    if (C->is_initialized) {
        return ResultOk;
    }
    thread.stack.push_frame(frame);

    auto result = initialize_class(C, thread);

    frame = thread.stack.pop_frame();

    return result;
}


Result resolve_class(ClassFile *clazz);

/**
 * Throws if the class was not found
 */
Result resolve_class(CONSTANT_Class_info *class_info);

field_info *find_field(ClassFile *clazz, std::string_view name, std::string_view descriptor, Reference &exception);

Result resolve_field(ClassFile *clazz, CONSTANT_Fieldref_info *fieldref_info, Reference &exception);


#endif //SCHOKOVM_CLASSLOADING_HPP
