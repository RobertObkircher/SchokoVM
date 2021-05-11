#include "classloading.hpp"

static bool initialize_class(ClassFile *clazz, Thread &thread, Frame &frame) {
    for (const auto &field : clazz->fields) {
        if (field.is_static()) {
            bool fail = false;
            for (const auto &attribute : field.attributes) {
                auto *constant_value_attribute = std::get_if<ConstantValue_attribute>(&attribute.variant);
                if (constant_value_attribute != nullptr) {
                    if (fail) {
                        assert(false);
                    }
                    fail = true;
                    auto const &value = clazz->constant_pool.table[constant_value_attribute->constantvalue_index].variant;

                    auto &descriptor = field.descriptor_index->value;
                    if (descriptor == "I" || descriptor == "S" || descriptor == "C" || descriptor == "B" ||
                        descriptor == "Z") {
                        clazz->static_field_values[field.index] = Value(std::get<CONSTANT_Integer_info>(value).value);
                    } else if (descriptor == "F") {
                        clazz->static_field_values[field.index] = Value(std::get<CONSTANT_Float_info>(value).value);
                    } else if (descriptor == "J") {
                        clazz->static_field_values[field.index] = Value(std::get<CONSTANT_Long_info>(value).value);
                    } else if (descriptor == "D") {
                        clazz->static_field_values[field.index] = Value(std::get<CONSTANT_Double_info>(value).value);
                    } else if (descriptor == "Ljava/lang/String;") {
                        // TODO
                        assert(false);
                    } else {
                        assert(false);
                    }
                }
            }
        }
    }

    if (clazz->clinit_index >= 0) {
        method_info *method = &clazz->methods[static_cast<unsigned long>(clazz->clinit_index)];

        size_t operand_stack_top = frame.first_operand_index + frame.operands_top;
        frame.operands_top += -method->stack_slots_for_parameters + method->return_size;

        frame.invoke_length = 0;
        thread.stack.parent_frames.push_back(frame);

        frame = {thread.stack, clazz, method, operand_stack_top};
        if (thread.stack.memory_used > thread.stack.memory.size())
            throw std::runtime_error("stack overflow");
        return true;
    }
    return false;
}


ClassResolution
resolve_class(std::unordered_map<std::string_view, ClassFile *> &class_files, CONSTANT_Class_info *class_info,
              Thread &thread, Frame &frame) {
    if (class_info->clazz == nullptr) {
        auto &name = class_info->name->value;

        // see https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.3.5
        // and https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.1

        // TODO we also need to deal with array classes here. They also load the class for the elements.

        auto result = class_files.find(name);
        if (result == class_files.end())
            return ClassResolution::NOT_FOUND;
        ClassFile *clazz = result->second;

        if (clazz->resolved) {
            class_info->clazz = clazz;
            return ClassResolution::OK;
        }

        for (const auto &field : clazz->fields) {
            if (!field.is_static())
                ++clazz->declared_instance_field_count;
        }

        size_t parent_instance_field_count = 0;
        // TODO use stdlib
        if (clazz->super_class != nullptr && clazz->super_class->name->value != "java/lang/Object" &&
            clazz->super_class->name->value != "java/lang/Exception") {
            auto resolution = resolve_class(class_files, clazz->super_class, thread, frame);
            if (resolution != ClassResolution::OK)
                return resolution;
            parent_instance_field_count = clazz->super_class->clazz->total_instance_field_count;
        }

        // instance and static fields
        clazz->total_instance_field_count = clazz->declared_instance_field_count + parent_instance_field_count;
        for (size_t static_index = 0, instance_index = parent_instance_field_count; auto &field : clazz->fields) {
            if (field.is_static()) {
                // used to index into clazz->static_field_values
                field.index = static_index++;
            } else {
                // used to index into instance fields
                field.index = instance_index++;
            }
            field.category = (field.descriptor_index->value == "D" || field.descriptor_index->value == "J")
                             ? ValueCategory::C2 : ValueCategory::C1;
        }
        clazz->static_field_values.resize(clazz->fields.size() - clazz->declared_instance_field_count);

        for (auto &interface : clazz->interfaces) {
            auto resolution = resolve_class(class_files, interface, thread, frame);
            if (resolution != ClassResolution::OK)
                return resolution;
        }

        clazz->resolved = true;
        class_info->clazz = clazz;

        if (initialize_class(clazz, thread, frame))
            return ClassResolution::PUSHED_INITIALIZER;
    }
    return ClassResolution::OK;
}

bool resolve_field_recursive(ClassFile *clazz, CONSTANT_Fieldref_info *field_info) {
    // Steps: https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.2
    for (;;) {
        // 1.
        assert(!field_info->resolved);
        for (auto const &f : clazz->fields) {
            if (f.name_index->value == field_info->name_and_type->name->value &&
                f.descriptor_index->value == field_info->name_and_type->descriptor->value) {

                field_info->resolved = true;
                field_info->is_boolean = f.descriptor_index->value == "Z";
                field_info->is_static = f.is_static();
                field_info->index = f.index;
                field_info->category = f.category;

                // TODO check access_flags
                return true;
            }
        }

        // 2.
        assert(!field_info->resolved);
        for (auto const &interface : field_info->class_->clazz->interfaces) {
            if (resolve_field_recursive(interface->clazz, field_info))
                return true;
        }

        assert(!field_info->resolved);

        if (clazz->super_class != nullptr) {
            // 3.
            clazz = clazz->super_class->clazz;
        } else {
            break;
        }
    }
    return false;
}
