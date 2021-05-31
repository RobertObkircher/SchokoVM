#ifndef SCHOKOVM_CLASSLOADING_HPP
#define SCHOKOVM_CLASSLOADING_HPP

#include <optional>
#include <unordered_map>

#include "classfile.hpp"
#include "interpreter.hpp"
#include "zip.hpp"

struct ClassPathEntry {
    std::string module_name;
    std::unordered_map<std::string, std::unique_ptr<ClassFile>> class_files;

    // TODO use subtyping instead?
    // TODO should module entries refer to other entries?
    // one of those will be empty
    std::string directory;
    ZipArchive zip;
};

struct BootstrapClassLoader {
    std::vector<ClassPathEntry> class_path_entries;
    std::vector<char> buffer;

    explicit BootstrapClassLoader(const std::string &bootclasspath);

    ClassFile *load(std::string const &name);
};

/**
 * Throws if the class was not found
 * @return whether a stack frame for an initializer was pushed
 */
bool resolve_class(BootstrapClassLoader &bootstrap_class_loader, CONSTANT_Class_info *class_info,
                   Thread &thread, Frame &frame);

bool resolve_field_recursive(ClassFile *clazz, CONSTANT_Fieldref_info *field_info);


#endif //SCHOKOVM_CLASSLOADING_HPP
