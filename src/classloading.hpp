#ifndef SCHOKOVM_CLASSLOADING_HPP
#define SCHOKOVM_CLASSLOADING_HPP

#include <unordered_map>
#include "classfile.hpp"
#include "interpreter.hpp"

/**
 * Throws if the class was not found
 * @return whether a stack frame for an initializer was pushed
 */
bool resolve_class(std::unordered_map<std::string_view, ClassFile *> &class_files, CONSTANT_Class_info *class_info,
                   Thread &thread, Frame &frame);

bool resolve_field_recursive(ClassFile *clazz, CONSTANT_Fieldref_info *field_info);


#endif //SCHOKOVM_CLASSLOADING_HPP
