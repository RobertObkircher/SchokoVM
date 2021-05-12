#ifndef SCHOKOVM_CLASSLOADING_HPP
#define SCHOKOVM_CLASSLOADING_HPP

#include <unordered_map>
#include "classfile.hpp"
#include "interpreter.hpp"

enum class ClassResolution {
    OK,
    PUSHED_INITIALIZER,
    NOT_FOUND,
};

ClassResolution
resolve_class(std::unordered_map<std::string_view, ClassFile *> &class_files, CONSTANT_Class_info *class_info,
              Thread &thread, Frame &frame);

bool resolve_field_recursive(ClassFile *clazz, CONSTANT_Fieldref_info *field_info);


#endif //SCHOKOVM_CLASSLOADING_HPP
