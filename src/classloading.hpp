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

struct Constants {
    ClassFile *java_io_Serializable{};

    ClassFile *java_lang_Class{};
    ClassFile *java_lang_Cloneable{};
    ClassFile *java_lang_Object{};
    ClassFile *java_lang_String{};

    Primitive primitives[Primitive::TYPE_COUNT] = {
            {Primitive::Byte,    "byte",    nullptr, "java/lang/Byte",      nullptr, 'B', "[B", nullptr},
            {Primitive::Char,    "char",    nullptr, "java/lang/Character", nullptr, 'C', "[C", nullptr},
            {Primitive::Double,  "double",  nullptr, "java/lang/Double",    nullptr, 'D', "[D", nullptr},
            {Primitive::Float,   "float",   nullptr, "java/lang/Float",     nullptr, 'F', "[F", nullptr},
            {Primitive::Int,     "int",     nullptr, "java/lang/Integer",   nullptr, 'I', "[I", nullptr},
            {Primitive::Long,    "long",    nullptr, "java/lang/Long",      nullptr, 'J', "[J", nullptr},
            {Primitive::Short,   "short",   nullptr, "java/lang/Short",     nullptr, 'S', "[S", nullptr},
            {Primitive::Boolean, "boolean", nullptr, "java/lang/Boolean",   nullptr, 'Z', "[Z", nullptr},
            {Primitive::Void,    "void",    nullptr, "java/lang/Void",      nullptr, 'V', "[V", nullptr},
    };
};

struct BootstrapClassLoader {
    static BootstrapClassLoader &get() { return the_bootstrap_class_loader; }

    static Constants const &constants() { return the_bootstrap_class_loader.m_constants; }

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

/**
 * Throws if the class was not found
 * @return whether a stack frame for an initializer was pushed
 */
bool resolve_class(CONSTANT_Class_info *class_info, Thread &thread, Frame &frame);

field_info *find_field(ClassFile *clazz, std::string_view name, std::string_view descriptor, Reference &exception);

void resolve_field(ClassFile *clazz, CONSTANT_Fieldref_info *fieldref_info, Reference &exception);


#endif //SCHOKOVM_CLASSLOADING_HPP
