#include "classloading.hpp"

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

        bool pushed_initializers = false;
        size_t parent_instance_field_count = 0;
        // TODO use stdlib
        if (clazz->super_class->name->value != "java/lang/Object" &&
            clazz->super_class->name->value != "java/lang/Exception") {
            switch (resolve_class(class_files, clazz->super_class, thread, frame)) {
                case ClassResolution::OK:
                    break;
                case ClassResolution::PUSHED_INITIALIZERS:
                    pushed_initializers = true;
                    break;
                case ClassResolution::NOT_FOUND:
                    return ClassResolution::NOT_FOUND;
            }
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
            switch (resolve_class(class_files, interface, thread, frame)) {
                case ClassResolution::OK:
                    break;
                case ClassResolution::PUSHED_INITIALIZERS:
                    pushed_initializers = true;
                    break;
                case ClassResolution::NOT_FOUND:
                    return ClassResolution::NOT_FOUND;
            }
        }

        clazz->resolved = true;

        if (false /* clazz.has_initializer */) {
            pushed_initializers = true;

            // TODO create a stackframe with the initializer but do not advance the pc of the current frame
            (void) thread;
            (void) frame;
        }

        // make it available even if we haven't executed the initializers yet
        // TODO is this a bad idea?
        // TODO maybe a better solution would be to run the initializers in a completely separate interpreter loop.
        // In that case we wouldn't have to restart/reexecute the caller instruction afterwards
        class_info->clazz = clazz;

        if (pushed_initializers)
            return ClassResolution::PUSHED_INITIALIZERS;
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
