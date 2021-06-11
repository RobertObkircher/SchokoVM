#include <filesystem>
#include <utility>

#include "classloading.hpp"
#include "memory.hpp"
#include "parser.hpp"
#include "util.hpp"
#include "zip.hpp"

BootstrapClassLoader BootstrapClassLoader::the_bootstrap_class_loader;

void BootstrapClassLoader::initialize_with_boot_classpath(std::string const &bootclasspath) {
    m_class_path_entries = std::vector<ClassPathEntry>();
    m_classes = std::unordered_map<std::string, ClassFile *>();

    for (auto &path : split(bootclasspath, ':')) {
        if (path.ends_with(".jar") || path.ends_with(".zip")) {
            m_class_path_entries.push_back({"", ZipArchive(path)});
        } else {
            m_class_path_entries.push_back({path, {}});
        }
    }

    m_constants.java_lang_Class = load_or_throw("java/lang/Class");
    m_constants.java_lang_Class->header.clazz = m_constants.java_lang_Class;

    m_constants.java_io_Serializable = load_or_throw("java/io/Serializable");
    m_constants.java_lang_Cloneable = load_or_throw("java/lang/Cloneable");
    m_constants.java_lang_NullPointerException = load_or_throw("java/lang/NullPointerException");
    m_constants.java_lang_Object = load_or_throw("java/lang/Object");
    m_constants.java_lang_String = load_or_throw("java/lang/String");

    for (auto &primitive : m_constants.primitives) {
        primitive.primitive = make_builtin_class(primitive.primitive_name, nullptr);
        if (primitive.id != ClassConstants::Primitive::Void) {
            primitive.array = make_builtin_class(primitive.array_name, primitive.primitive);
        } else {
            primitive.array = nullptr;
        }
        primitive.boxed = load_or_throw(primitive.boxed_name);
    }

}

// https://stackoverflow.com/a/7782037
struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

ClassFile *BootstrapClassLoader::load_or_throw(std::string const &name) {
    ClassFile *clazz = load(name);
    if (clazz == nullptr) {
        throw std::runtime_error("Failed to load " + name);
    }
    return clazz;
}

ClassFile *BootstrapClassLoader::load(std::string const &name) {
    if (auto found = m_classes.find(name); found != m_classes.end()) {
        return found->second;
    }

    if (name.size() >= 2 && name[0] == '[') {
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
        ClassFile *array_class = make_builtin_class(name, element_type);
        return array_class;
    }

    ClassFile *result = nullptr;
    for (auto &cp_entry : m_class_path_entries) {
        if (!cp_entry.directory.empty()) {
            auto path = cp_entry.directory + "/" + name + ".class";
            std::ifstream in{path, std::ios::in | std::ios::binary};

            if (in) {
                Parser parser{in};
                result = Heap::get().allocate_class();
                *result = parser.parse();
                result->header.clazz = constants().java_lang_Class;
                break;
            }
        } else if (!cp_entry.zip.path.empty()) {
            auto path = name + ".class";

            if (ZipEntry const *zip_entry = cp_entry.zip.entry_for_path(path); zip_entry != nullptr) {
                cp_entry.zip.read(*zip_entry, m_buffer);

                membuf buf{m_buffer.data(), m_buffer.data() + m_buffer.size()};
                std::istream in{&buf};

                Parser parser{in};
                result = Heap::get().allocate_class();
                *result = parser.parse();
                result->header.clazz = constants().java_lang_Class;
                break;
            }
        }
    }

    if (result != nullptr) {
        if (name != result->name()) {
            throw ParseError("unexpected name");
        }
    }

    m_classes.insert({name, result});
    return result;
}

ClassFile *BootstrapClassLoader::make_builtin_class(std::string name, ClassFile *array_element_type) {
    auto clazz = Heap::get().allocate_class();

    u2 index = 0;
    auto add_name_and_class = [&clazz, &name, &index](ClassFile *c) -> CONSTANT_Class_info * {
        assert(c);
        clazz->constant_pool.table[index].variant = CONSTANT_Utf8_info{
                c == clazz ? name : c->name()
        };
        clazz->constant_pool.table[index + 1].variant = CONSTANT_Class_info{
                index,
                &clazz->constant_pool.get<CONSTANT_Utf8_info>(index),
                c,
        };
        auto result = &clazz->constant_pool.get<CONSTANT_Class_info>(index + 1);
        index += 2;
        return result;
    };

    if (array_element_type != nullptr) {
        clazz->constant_pool.table.resize(4 * 2);
        clazz->super_class_ref = add_name_and_class(clazz->super_class = constants().java_lang_Object);
        clazz->interfaces.push_back(add_name_and_class(constants().java_lang_Cloneable));
        clazz->interfaces.push_back(add_name_and_class(constants().java_io_Serializable));
    } else {
        clazz->constant_pool.table.resize(1 * 2);
    }
    clazz->this_class = add_name_and_class(clazz);
    clazz->array_element_type = array_element_type;

    // TODO add array clone method here?

    m_classes.insert({std::move(name), clazz});
    return clazz;
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
                        // TODO deduplicate this + i think the
                        auto string_clazz = BootstrapClassLoader::constants().java_lang_String;
                        if (resolve_class(string_clazz->this_class, thread, frame)) {
                            return true;
                        }
                        frame.pc++;

                        std::string &modified_utf8 = std::get<CONSTANT_String_info>(value).string->value;
                        std::string string_utf8;
                        string_utf8.reserve(modified_utf8.length());
                        for (size_t i = 0; i < modified_utf8.length(); ++i) {
                            u1 c = static_cast<u1>(modified_utf8[i]);

                            if (c == 0b11101101) {
                                u1 v = static_cast<u1>(modified_utf8[i + 1]);
                                u1 w = static_cast<u1>(modified_utf8[i + 2]);
                                u1 x = static_cast<u1>(modified_utf8[i + 3]);
                                u1 y = static_cast<u1>(modified_utf8[i + 4]);
                                u1 z = static_cast<u1>(modified_utf8[i + 5]);

                                if (((v & 0xf0) == 0b10100000) && ((w & 0b11000000) == 0b10000000)
                                    && (x == 0b11101101) && ((y & 0xf0) == 0b10110000) &&
                                    ((z & 11000000) == 0b10000000)) {
                                    int codepoint =
                                            0x10000 + ((v & 0x0f) << 16) + ((w & 0x3f) << 10) + ((y & 0x0f) << 6) +
                                            (z & 0x3f);
                                    // convert into the 4-byte utf8 variant
                                    string_utf8.push_back(static_cast<char>(0b11110000 | ((codepoint >> 18) & 0b1111)));
                                    string_utf8.push_back(
                                            static_cast<char>(0b10000000 | ((codepoint >> 12) & 0b111111)));
                                    string_utf8.push_back(
                                            static_cast<char>(0b10000000 | ((codepoint >> 6) & 0b111111)));
                                    string_utf8.push_back(static_cast<char>(0b10000000 | ((codepoint) & 0b111111)));
                                    i += 5;
                                    continue;
                                }
                            }

                            if ((c & 0b10000000) == 0) { // copy 1 byte over
                                string_utf8.push_back(static_cast<char>(c));
                            } else if ((c & 0b11100000) == 0b11000000) { // copy 2 byte over
                                string_utf8.push_back(static_cast<char>(c));
                                string_utf8.push_back(modified_utf8[++i]);
                            } else if ((c & 0b11110000) == 0b11100000) { // copy 3 byte over
                                string_utf8.push_back(static_cast<char>(c));
                                string_utf8.push_back(modified_utf8[++i]);
                                string_utf8.push_back(modified_utf8[++i]);
                            } else {
                                throw std::runtime_error(
                                        "Invalid byte in modified utf8 string: " + std::to_string((int) c));
                            }
                        }

                        auto java_string = Heap::get().make_string(string_utf8);
                        clazz->static_field_values[field.index] = Value(java_string);

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
        frame.operands_top += -method->stack_slots_for_parameters + method->return_category;

        frame.invoke_length = 0;
        thread.stack.parent_frames.push_back(frame);

        frame = {thread.stack, clazz, method, operand_stack_top};
        if (thread.stack.memory_used > thread.stack.memory.size())
            throw std::runtime_error("stack overflow");
        return true;
    }
    return false;
}


bool resolve_class(CONSTANT_Class_info *class_info, Thread &thread, Frame &frame) {
    if (class_info->clazz == nullptr) {
        auto &name = class_info->name->value;

        // see https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.3.5
        // and https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.1

        // TODO we also need to deal with array classes here. They also load the class for the elements.

        // TODO check other classloaders first
        ClassFile *clazz = BootstrapClassLoader::get().load(name);
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
            if (resolve_class(clazz->super_class_ref, thread, frame))
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
            auto runInitializer = resolve_class(interface, thread, frame);
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

void resolve_field(ClassFile *clazz, CONSTANT_Fieldref_info *fieldref_info, Reference &exception) {
    assert(!fieldref_info->resolved);

    field_info *info = find_field(clazz, fieldref_info->name_and_type->name->value,
                                  fieldref_info->name_and_type->descriptor->value, exception);
    if (exception != JAVA_NULL) {
        return;
    }

    fieldref_info->resolved = true;
    fieldref_info->is_boolean = info->descriptor_index->value == "Z";
    fieldref_info->is_static = info->is_static();
    fieldref_info->index = info->index;
    fieldref_info->category = info->category;
}

field_info *find_field_recursive(ClassFile *clazz, std::string_view name, std::string_view descriptor) {
    // Steps: https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.2
    for (;;) {
        // 1.
        for (auto &f : clazz->fields) {
            if (f.name_index->value == name && f.descriptor_index->value == descriptor) {
                return &f;
            }
        }

        // 2.
        for (auto const &interface : clazz->interfaces) {
            field_info *info = find_field_recursive(interface->clazz, name, descriptor);
            if (info != nullptr) {
                return info;
            }
        }

        if (clazz->super_class != nullptr) {
            // 3.
            clazz = clazz->super_class;
        } else {
            break;
        }
    }
    return nullptr;
}

field_info *find_field(ClassFile *clazz, std::string_view name, std::string_view descriptor, Reference &exception) {
    auto *result = find_field_recursive(clazz, name, descriptor);
    if (result == nullptr) {
        // TODO new NoSuchFieldError
        exception.memory = (void *) 123;
    }
    // TODO access control
    return result;
}
