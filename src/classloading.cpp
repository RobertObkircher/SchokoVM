#include <filesystem>
#include <utility>

#include "classloading.hpp"
#include "parser.hpp"
#include "util.hpp"
#include "zip.hpp"

BootstrapClassLoader::BootstrapClassLoader(const std::string &bootclasspath)
        : class_path_entries(), buffer(), array_classes() {
    // TODO fix hardcoded path
    class_path_entries.push_back({"java.base", {}, "../jdk/exploded-modules/java.base", {}});

    for (auto &path : split(bootclasspath, ':')) {
        if (path.ends_with(".jar") || path.ends_with(".zip")) {
            class_path_entries.push_back({"", {}, "", ZipArchive(path)});
        } else {
            class_path_entries.push_back({path, {}, path, {}});
        }
    }

    make_array_class("[B");
    make_array_class("[C");
    make_array_class("[D");
    make_array_class("[F");
    make_array_class("[I");
    make_array_class("[J");
    make_array_class("[S");
    make_array_class("[Z");
}

// https://stackoverflow.com/a/7782037
struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

ClassFile *BootstrapClassLoader::load(std::string const &name) {
    if (name.size() >= 2 && name[0] == '[') {
        if (auto found = array_classes.find(name); found != array_classes.end()) {
            return found->second.get();
        }

        std::string element_name;
        if (name[1] == 'L') {
            // turn [Ljava.lang.Boolean; into java/lang/Boolean
            element_name = name.substr(2, name.size() - 3);
            for (auto &item : element_name) {
                if (item == '.') {
                    item = '/';
                }
            }
        } else if (name[1] == '[') {
            element_name = name.substr(1);
        } else {
            abort(); // primitive arrays are in the hashmap
        }

        ClassFile *element_type = load(element_name);
        if (element_type == nullptr) {
            return nullptr;
        }
        ClassFile *array_class = make_array_class(name);
        array_class->array_element_type = element_type;
        return array_class;
    }

    for (auto &cp_entry : class_path_entries) {
        if (auto loaded = cp_entry.class_files.find(name); loaded != cp_entry.class_files.end()) {
            if (loaded->second == nullptr) {
                continue;
            }
            return loaded->second.get();
        }

        std::unique_ptr<ClassFile> parsed{};

        if (!cp_entry.directory.empty()) {
            auto path = cp_entry.directory + "/" + name + ".class";
            std::ifstream in{path, std::ios::in | std::ios::binary};

            if (in) {
                Parser parser{in};
                parsed = std::make_unique<ClassFile>(parser.parse());
            }
        } else if (!cp_entry.zip.path.empty()) {
            auto path = name + ".class";

            if (ZipEntry const *zip_entry = cp_entry.zip.entry_for_path(path); zip_entry != nullptr) {
                cp_entry.zip.read(*zip_entry, buffer);

                membuf buf{buffer.data(), buffer.data() + buffer.size()};
                std::istream in{&buf};

                Parser parser{in};
                parsed = std::make_unique<ClassFile>(parser.parse());
            }
        }

        ClassFile *result = parsed.get();
        cp_entry.class_files.insert({name, std::move(parsed)});

        if (result != nullptr) {
            if (name != result->name()) {
                throw ParseError("unexpected name");
            }
            return result;
        }
    }
    return nullptr;
}

ClassFile *BootstrapClassLoader::make_array_class(std::string name) {
    auto clazz = std::make_unique<ClassFile>();

    auto add_name_and_class = [&clazz, &name](u2 index, ClassFile *c) -> CONSTANT_Class_info * {
        assert(c);
        clazz->constant_pool.table[index].variant = CONSTANT_Utf8_info{
                c == clazz.get() ? name : c->name()
        };
        clazz->constant_pool.table[index + 1].variant = CONSTANT_Class_info{
                index,
                &clazz->constant_pool.get<CONSTANT_Utf8_info>(index),
                c,
        };
        return &clazz->constant_pool.get<CONSTANT_Class_info>(index + 1);
    };

    clazz->constant_pool.table.resize(4 * 2);
    clazz->this_class = add_name_and_class(0, clazz.get());
    clazz->super_class_ref = add_name_and_class(2, clazz->super_class = load("java/lang/Object"));
    clazz->interfaces.push_back(add_name_and_class(4, load("java/lang/Cloneable")));
    clazz->interfaces.push_back(add_name_and_class(6, load("java/io/Serializable")));

    // TODO add array clone method here?

    ClassFile *result = clazz.get();
    array_classes.insert({std::move(name), std::move(clazz)});
    return result;
}


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


bool resolve_class(BootstrapClassLoader &bootstrap_class_loader, CONSTANT_Class_info *class_info,
                   Thread &thread, Frame &frame) {
    if (class_info->clazz == nullptr) {
        auto &name = class_info->name->value;

        // see https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.3.5
        // and https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.1

        // TODO we also need to deal with array classes here. They also load the class for the elements.

        ClassFile *clazz = bootstrap_class_loader.load(name);
        if (clazz == nullptr) {
            // TODO this prints "A not found" if A was found but a superclass/interface wasn't
            throw std::runtime_error("class not found: '" + name + "'");
        }

        if (clazz->resolved) {
            class_info->clazz = clazz;
            return false;
        }

        for (const auto &field : clazz->fields) {
            if (!field.is_static())
                ++clazz->declared_instance_field_count;
        }

        size_t parent_instance_field_count = 0;
        if (clazz->super_class == nullptr && clazz->super_class_ref != nullptr) {
            if (resolve_class(bootstrap_class_loader, clazz->super_class_ref, thread, frame))
                return true;

            clazz->super_class = clazz->super_class_ref->clazz;
            parent_instance_field_count = clazz->super_class->total_instance_field_count;
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
            auto runInitializer = resolve_class(bootstrap_class_loader, interface, thread, frame);
            if (runInitializer)
                return true;
        }

        clazz->resolved = true;
        class_info->clazz = clazz;

        if (initialize_class(clazz, thread, frame))
            return true;
    }
    return false;
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
        for (auto const &interface : clazz->interfaces) {
            if (resolve_field_recursive(interface->clazz, field_info))
                return true;
        }

        assert(!field_info->resolved);

        if (clazz->super_class != nullptr) {
            // 3.
            clazz = clazz->super_class;
        } else {
            break;
        }
    }
    return false;
}
