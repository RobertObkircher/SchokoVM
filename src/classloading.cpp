#include <filesystem>
#include <utility>
#include <mutex>

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
    m_constants.java_lang_Object = load_or_throw("java/lang/Object");
    m_constants.java_lang_String = load_or_throw("java/lang/String");

    for (auto &primitive : m_constants.primitives) {
        primitive.primitive = make_builtin_class(primitive.primitive_name, nullptr);
        if (primitive.id != Primitive::Void) {
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
            // turn [Ljava.lang.Boolean; into java.lang.Boolean
            element_name = name.substr(2, name.size() - 3);
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
                parser.parse(*result);
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
                parser.parse(*result);
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
        clazz->field_component_type = Value{Reference{array_element_type}};
    } else {
        clazz->constant_pool.table.resize(1 * 2);
    }
    clazz->this_class = add_name_and_class(clazz);
    clazz->array_element_type = array_element_type;

    // TODO add array clone method here?

    m_classes.insert({std::move(name), clazz});
    return clazz;
}

// Note: This is not "preparation"
static void initialize_static_fields(ClassFile *clazz) {
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
                        auto java_string = Heap::get().make_string(std::get<CONSTANT_String_info>(value).string->value);
                        clazz->static_field_values[field.index] = Value(java_string);
                    } else {
                        assert(false);
                    }
                }
            }
        }
    }
}

static Result recursively_initialize_interfaces(ClassFile *clazz, Thread &thread) {
    assert(clazz);
    for (auto &class_info : clazz->interfaces) {
        auto *interface = class_info->clazz;
        assert(interface->is_interface());

        if (recursively_initialize_interfaces(interface, thread)) {
            return Exception;
        }

        for (method_info const &method : interface->methods) {
            if (!method.is_abstract() && !method.is_static()) {
                if (initialize_class(class_info->clazz, thread)) {
                    return Exception;
                }
                break;
            }
        }
    }
    return ResultOk;
};

// https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.5
Result initialize_class(ClassFile *C, Thread &thread) {
    // quick check without lock
    if (C->is_initialized) {
        return ResultOk;
    }

    // 1. Synchronize on the initialization lock, LC, for C.
    //    This involves waiting until the current thread can acquire LC.
    std::unique_lock LC{C->initialization_lock};

    // 3. If the Class object for C indicates that initialization is in progress for C by the current thread,
    //    then this must be a recursive request for initialization. Release LC and complete normally.
    if (C->initializing_thread == &thread) {
        return ResultOk;
    }

    // 2. If the Class object for C indicates that initialization is in progress for C by some other thread,
    //    then release LC and block the current thread until informed that the in-progress initialization has completed,
    //    at which time repeat this procedure.
    //    Thread interrupt status is unaffected by execution of the initialization procedure.
    C->initialization_condition_variable.wait(LC, [C]() {
        return C->initializing_thread == nullptr;
    });

    // 4. If the Class object for C indicates that C has already been initialized, then no further action is required. Release LC and complete normally.
    if (C->is_initialized) {
        return ResultOk;
    }

    // 5. If the Class object for C is in an erroneous state, then initialization is not possible. Release LC and throw a NoClassDefFoundError.
    if (C->is_erroneous_state) {
        LC.unlock();
        abort(); // TODO throw
    }

    // 6. Otherwise, record the fact that initialization of the Class object for C is in progress by the current thread, and release LC.
    //    Then, initialize each final static field of C with the constant value in its ConstantValue attribute (§4.7.2),
    //    in the order the fields appear in the ClassFile structure.
    C->initializing_thread = &thread;
    LC.unlock();
    initialize_static_fields(C);

    // 7. Next, if C is a class rather than an interface, then let SC be its superclass and let SI1, ..., SIn be all
    //    superinterfaces of C (whether direct or indirect) that declare at least one non-abstract, non-static method.
    //    The order of superinterfaces is given by a recursive enumeration over the superinterface hierarchy of each
    //    interface directly implemented by C. For each interface I directly implemented by C (in the order of the
    //    interfaces array of C), the enumeration recurs on I's superinterfaces (in the order of the interfaces array
    //    of I) before returning I.
    //
    //    For each S in the list [ SC, SI1, ..., SIn ], if S has not yet been initialized, then recursively perform this
    //    entire procedure for S. If necessary, verify and prepare S first.
    //
    //    If the initialization of S completes abruptly because of a thrown exception, then acquire LC, label the Class
    //    object for C as erroneous, notify all waiting threads, release LC, and complete abruptly, throwing the same
    //    exception that resulted from initializing SC.
    if (!C->is_interface()) {
        auto fail7 = [&LC, C]() {
            LC.lock();
            C->is_erroneous_state = true;
            C->initializing_thread = nullptr;
            C->initialization_condition_variable.notify_all();
            LC.unlock();
            return Exception;
        };
        if (C->super_class) {
            if (initialize_class(C->super_class, thread)) {
                return fail7();
            }
        }
        if (recursively_initialize_interfaces(C, thread)) {
            return fail7();
        }
    }

    // 8. Next, determine whether assertions are enabled for C by querying its defining class loader.
    // TODO assertions

    // 9. Next, execute the class or interface initialization method of C.
    bool no_exception = true;
    if (C->clinit_index >= 0) {
        method_info *clinit = &C->methods[static_cast<unsigned long>(C->clinit_index)];

        interpret(thread, C, clinit);

        if (thread.current_exception != JAVA_NULL) {
            no_exception = false;
        }
    }

    // 10. If the execution of the class or interface initialization method completes normally,
    //     then acquire LC, label the Class object for C as fully initialized,
    //     notify all waiting threads, release LC, and complete this procedure normally.
    if (no_exception) {
        LC.lock();
        C->is_initialized = true;
        C->initializing_thread = nullptr;
        C->initialization_condition_variable.notify_all();
        return ResultOk;
    }

    // 11. Otherwise, the class or interface initialization method must have completed abruptly by throwing some
    //     exception E. If the class of E is not Error or one of its subclasses, then create a new instance of the class
    //     ExceptionInInitializerError with E as the argument, and use this object in place of E in the following step.
    //     If a new instance of ExceptionInInitializerError cannot be created because an OutOfMemoryError occurs, then
    //     use an OutOfMemoryError object in place of E in the following step.

    // TODO handle_throw(thread, frame, thread.current_exception);
    abort();

    // 12. Acquire LC, label the Class object for C as erroneous, notify all waiting threads, release LC,
    //     and complete this procedure abruptly with reason E or its replacement as determined in the previous step.
    LC.lock();
    C->is_erroneous_state = true;
    C->initializing_thread = nullptr;
    C->initialization_condition_variable.notify_all();
    LC.unlock();

    return Exception;
}

Result resolve_class(ClassFile *clazz) {
    if (clazz->resolved) {
        return ResultOk;
    }

    for (const auto &field : clazz->fields) {
        if (!field.is_static())
            ++clazz->declared_instance_field_count;
    }

    size_t parent_instance_field_count = 0;
    if (clazz->super_class == nullptr && clazz->super_class_ref != nullptr) {
        if (resolve_class(clazz->super_class_ref))
            return Exception;

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
        if (resolve_class(interface))
            return Exception;
    }

    clazz->resolved = true;
    clazz->this_class->clazz = clazz;
    return ResultOk;
}

Result resolve_class(CONSTANT_Class_info *class_info) {
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

        if (resolve_class(clazz) == Exception) {
            return Exception;
        }

        class_info->clazz = clazz;
    }
    return ResultOk;
}

Result resolve_field(ClassFile *clazz, CONSTANT_Fieldref_info *fieldref_info, Reference &exception) {
    assert(!fieldref_info->resolved);

    field_info *info = find_field(clazz, fieldref_info->name_and_type->name->value,
                                  fieldref_info->name_and_type->descriptor->value, exception);
    if (exception != JAVA_NULL) {
        return Exception;
    }

    fieldref_info->resolved = true;
    fieldref_info->is_boolean = info->descriptor_index->value == "Z";
    fieldref_info->is_static = info->is_static();
    fieldref_info->value_clazz = info->clazz->clazz;
    fieldref_info->index = info->index;
    fieldref_info->category = info->category;

    return ResultOk;
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


void Constants::resolve_and_initialize(Thread &thread) {
    if (thread.current_exception != JAVA_NULL) {
        throw std::runtime_error("Failed to resolve and initialize");
    }

    auto resolve_and_initialize = [&thread](ClassFile *clazz) {
        if (resolve_class(clazz->this_class)) {
            throw std::runtime_error("Failed to resolve");
        }
        if (thread.current_exception != JAVA_NULL) {
            throw std::runtime_error("Failed to resolve2");
        }
        if (initialize_class(clazz, thread)) {
            throw std::runtime_error("Failed to initialize");
        }
        if (thread.current_exception != JAVA_NULL) {
            throw std::runtime_error("Failed to initialize2");
        }
    };

    resolve_and_initialize(java_lang_Object);
    resolve_and_initialize(java_lang_Class);
    resolve_and_initialize(java_lang_String);
    resolve_and_initialize(java_io_Serializable);
    resolve_and_initialize(java_lang_Cloneable);
}
