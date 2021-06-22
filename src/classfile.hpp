#ifndef SCHOKOVM_CLASSFILE_HPP
#define SCHOKOVM_CLASSFILE_HPP

#include <bitset>
#include <condition_variable>
#include <vector>
#include <variant>
#include <cassert>
#include <string>

#include "memory.hpp"
#include "native.hpp"
#include "types.hpp"

// Initially generated like this:
// https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-4.html
// {
//     let es = document.querySelectorAll(':not(.informalexample) > pre.screen');
//     let code = '';
//     for (e of es) {
//         code += 'struct '
//         code += e.innerHTML.trim();
//         code += ';\n\n';
//     }
//     console.log(code);
// }


// forward declarations:
struct cp_info;
struct ConstantPool;
struct field_info;
struct method_info;
struct attribute_info;
struct stack_map_frame;
struct annotation;
struct type_annotation;
struct type_path;
struct record_component_info;
struct Code_attribute;
struct ClassFile;

enum CpTag : u1 {
    // Just a dummy value used in the constant pool for index 0 and after longs/doubles
    CONSTANT_Invalid = 0,
    CONSTANT_Utf8 = 1,
    CONSTANT_Integer = 3,
    CONSTANT_Float = 4,
    CONSTANT_Long = 5,
    CONSTANT_Double = 6,
    CONSTANT_Class = 7,
    CONSTANT_String = 8,
    CONSTANT_Fieldref = 9,
    CONSTANT_Methodref = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_NameAndType = 12,
    CONSTANT_MethodHandle = 15,
    CONSTANT_MethodType = 16,
    CONSTANT_Dynamic = 17,
    CONSTANT_InvokeDynamic = 18,
    CONSTANT_Module = 19,
    CONSTANT_Package = 20,
};

// TODO remove if this stays unused
inline bool is_loadable(CpTag tag) {
    switch (tag) {
        case CONSTANT_Integer:
        case CONSTANT_Float:
        case CONSTANT_Long:
        case CONSTANT_Double:
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_MethodHandle:
        case CONSTANT_MethodType:
        case CONSTANT_Dynamic:
            return true;
        default:
            return false;
    }
}

// cp_info:
struct CONSTANT_Utf8_info;
struct CONSTANT_NameAndType_info;

// not from the spec
struct CONSTANT_Invalid_info {
};

struct CONSTANT_Class_info {
    u2 name_index;
    CONSTANT_Utf8_info *name;

    ClassFile *clazz;
};

struct CONSTANT_Fieldref_info {
    u2 class_index;
    u2 name_and_type_index;
    CONSTANT_Class_info *class_;
    CONSTANT_NameAndType_info *name_and_type;

    bool resolved;
    bool is_boolean;
    bool is_static;
    ValueCategory category;
    // Might be a superclass of class_
    ClassFile *value_clazz;
    size_t index;
};


struct ClassInterface_Methodref {
    u2 class_index;
    u2 name_and_type_index;
    CONSTANT_Class_info *class_;
    CONSTANT_NameAndType_info *name_and_type;
    method_info *method;
};

struct CONSTANT_Methodref_info {
    ClassInterface_Methodref method;
};

struct CONSTANT_InterfaceMethodref_info {
    ClassInterface_Methodref method;
};

struct CONSTANT_String_info {
    u2 string_index;
    CONSTANT_Utf8_info *string;
};

struct CONSTANT_Integer_info {
    s4 value;
};

struct CONSTANT_Float_info {
    float value;
};

struct CONSTANT_Long_info {
    s8 value;
};

struct CONSTANT_Double_info {
    double value;
};

struct CONSTANT_NameAndType_info {
    u2 name_index;
    u2 descriptor_index;
    CONSTANT_Utf8_info *name;
    CONSTANT_Utf8_info *descriptor;
};

struct CONSTANT_Utf8_info {
    std::string value;
};

enum MethodHandleKind : u1 {
    REF_getField = 1,
    REF_getStatic = 2,
    REF_putField = 3,
    REF_putStatic = 4,
    REF_invokeVirtual = 5,
    REF_invokeStatic = 6,
    REF_invokeSpecial = 7,
    REF_newInvokeSpecial = 8,
    REF_invokeInterface = 9,
};

struct CONSTANT_MethodHandle_info {
    MethodHandleKind reference_kind;
    u2 reference_index;
};

struct CONSTANT_MethodType_info {
    u2 descriptor_index;
};

struct CONSTANT_Dynamic_info {
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
};

struct CONSTANT_InvokeDynamic_info {
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
};

struct CONSTANT_Module_info {
    u2 name_index;
};

struct CONSTANT_Package_info {
    u2 name_index;
};

struct cp_info {
    std::variant<
            CONSTANT_Invalid_info,
            CONSTANT_Class_info,
            CONSTANT_Fieldref_info,
            CONSTANT_Methodref_info,
            CONSTANT_InterfaceMethodref_info,
            CONSTANT_String_info,
            CONSTANT_Integer_info,
            CONSTANT_Float_info,
            CONSTANT_Long_info,
            CONSTANT_Double_info,
            CONSTANT_NameAndType_info,
            CONSTANT_Utf8_info,
            CONSTANT_MethodHandle_info,
            CONSTANT_MethodType_info,
            CONSTANT_Dynamic_info,
            CONSTANT_InvokeDynamic_info,
            CONSTANT_Module_info,
            CONSTANT_Package_info
    > variant;
};

enum class FieldInfoAccessFlags : u2 {
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_VOLATILE = 0x0040,
    ACC_TRANSIENT = 0x0080,
    ACC_SYNTHETIC = 0x1000,
    ACC_ENUM = 0x4000,
};

struct field_info {
    u2 access_flags;
    CONSTANT_Utf8_info *name_index;
    CONSTANT_Utf8_info *descriptor_index;
    std::vector<attribute_info> attributes;

    CONSTANT_Class_info* clazz;
    size_t index;
    ValueCategory category;

    [[nodiscard]] inline bool is_static() const {
        return (access_flags & static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC)) != 0;
    }
};

enum class MethodInfoAccessFlags : u2 {
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_SYNCHRONIZED = 0x0020,
    ACC_BRIDGE = 0x0040,
    ACC_VARARGS = 0x0080,
    ACC_NATIVE = 0x0100,
    ACC_ABSTRACT = 0x0400,
    ACC_STRICT = 0x0800,
    ACC_SYNTHETIC = 0x1000,
};

struct method_info {
    u2 access_flags;
    CONSTANT_Utf8_info *name_index;
    CONSTANT_Utf8_info *descriptor_index;
    std::vector<attribute_info> attributes;
    Code_attribute *code_attribute;

    // This is called nargs in the invoke* descriptions: https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-6.html#jvms-6.5.invokestatic
    u1 parameter_count;
    // Some parameters require 2 local variable slots
    u2 stack_slots_for_parameters;

    u1 return_category; // 0, 1, 2

    std::optional<NativeFunction> native_function;

    [[nodiscard]] inline bool is_static() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_STATIC)) != 0;
    }

    [[nodiscard]] inline bool is_private() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_PRIVATE)) != 0;
    }

    [[nodiscard]] inline bool is_abstract() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_ABSTRACT)) != 0;
    }

    [[nodiscard]] inline bool is_protected() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_PROTECTED)) != 0;
    }

    [[nodiscard]] inline bool is_public() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_PUBLIC)) != 0;
    }

    [[nodiscard]] inline bool is_native() const {
        return (access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_NATIVE)) != 0;
    }
};

// attribute_info...

struct ConstantValue_attribute {
    u2 constantvalue_index;
};

struct ExceptionTableEntry {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
};

struct Code_attribute {
    u2 max_stack;
    u2 max_locals;
    std::vector<u1> code;
    std::vector<ExceptionTableEntry> exception_table;
    std::vector<attribute_info> attributes;
};

#if 0
struct StackMapTable_attribute {
//    u2 number_of_entries;
//    stack_map_frame entries[number_of_entries];
    std::vector<stack_map_frame> entries;
};

union verification_type_info {
    Top_variable_info;
    Integer_variable_info;
    Float_variable_info;
    Long_variable_info;
    Double_variable_info;
    Null_variable_info;
    UninitializedThis_variable_info;
    Object_variable_info;
    Uninitialized_variable_info;
};

struct Top_variable_info {
    u1 tag = ITEM_Top; /* 0 */
};

struct Integer_variable_info {
    u1 tag = ITEM_Integer; /* 1 */
};

struct Float_variable_info {
    u1 tag = ITEM_Float; /* 2 */
};

struct Null_variable_info {
    u1 tag = ITEM_Null; /* 5 */
};

struct UninitializedThis_variable_info {
    u1 tag = ITEM_UninitializedThis; /* 6 */
};

struct Object_variable_info {
    u1 tag = ITEM_Object; /* 7 */
    u2 cpool_index;
};

struct Uninitialized_variable_info {
    u1 tag = ITEM_Uninitialized; /* 8 */
    u2 offset;
};

struct Long_variable_info {
    u1 tag = ITEM_Long; /* 4 */
};

struct Double_variable_info {
    u1 tag = ITEM_Double; /* 3 */
};

union stack_map_frame {
    same_frame;
    same_locals_1_stack_item_frame;
    same_locals_1_stack_item_frame_extended;
    chop_frame;
    same_frame_extended;
    append_frame;
    full_frame;
};

struct same_frame {
    u1 frame_type = SAME; /* 0-63 */
};

struct same_locals_1_stack_item_frame {
    u1 frame_type = SAME_LOCALS_1_STACK_ITEM; /* 64-127 */
//    verification_type_info stack[1];
};

struct same_locals_1_stack_item_frame_extended {
    u1 frame_type = SAME_LOCALS_1_STACK_ITEM_EXTENDED; /* 247 */
    u2 offset_delta;
//    verification_type_info stack[1];
};

struct chop_frame {
    u1 frame_type = CHOP; /* 248-250 */
    u2 offset_delta;
};

struct same_frame_extended {
    u1 frame_type = SAME_FRAME_EXTENDED; /* 251 */
    u2 offset_delta;
};

struct append_frame {
    u1 frame_type = APPEND; /* 252-254 */
    u2 offset_delta;
//    verification_type_info locals[frame_type - 251];
};

struct full_frame {
    u1 frame_type = FULL_FRAME; /* 255 */
    u2 offset_delta;
    u2 number_of_locals;
//    verification_type_info locals[number_of_locals];
    u2 number_of_stack_items;
//    verification_type_info stack[number_of_stack_items];
};
#endif

struct Exceptions_attribute {
    std::vector<CONSTANT_Class_info *> exception_index_table;
};

struct InnerClasses_attribute {
    u2 number_of_classes;
//    {
//        u2 inner_class_info_index;
//        u2 outer_class_info_index;
//        u2 inner_name_index;
//        u2 inner_class_access_flags;
//    } classes[number_of_classes];
};

struct EnclosingMethod_attribute {
    u2 class_index;
    u2 method_index;
};

struct Synthetic_attribute {
};

struct Signature_attribute {
    CONSTANT_Utf8_info *signature_index;
};

struct SourceFile_attribute {
    CONSTANT_Utf8_info *sourcefile_index;
};

struct SourceDebugExtension_attribute {
    std::string debug_extension;
};

struct LineNumberTableEntry {
    u2 start_pc;
    u2 line_number;
};

struct LineNumberTable_attribute {
    std::vector<LineNumberTableEntry> line_number_table;
};

struct LocalVariableTable_attribute {
    u2 local_variable_table_length;
//    {
//        u2 start_pc;
//        u2 length;
//        u2 name_index;
//        u2 descriptor_index;
//        u2 index;
//    } local_variable_table[local_variable_table_length];
};

struct LocalVariableTypeTable_attribute {
    u2 local_variable_type_table_length;
//    {
//        u2 start_pc;
//        u2 length;
//        u2 name_index;
//        u2 signature_index;
//        u2 index;
//    } local_variable_type_table[local_variable_type_table_length];
};

struct Deprecated_attribute {
};

struct RuntimeVisibleAnnotations_attribute {
    u2 num_annotations;
//    annotation annotations[num_annotations];
};

struct annotation {
    u2 type_index;
    u2 num_element_value_pairs;
//        {
//            u2 element_name_index;
//            element_value value;
//    } element_value_pairs[num_element_value_pairs];
};

struct element_value {
    u1 tag;
    union {
        u2 const_value_index;

        struct {
            u2 type_name_index;
            u2 const_name_index;
        } enum_const_value;

        u2 class_info_index;

        annotation annotation_value;

        struct {
            u2 num_values;
//            element_value values[num_values];
        } array_value;
    } value;
};

struct RuntimeInvisibleAnnotations_attribute {
    u2 num_annotations;
//    annotation annotations[num_annotations];
};

struct RuntimeVisibleParameterAnnotations_attribute {
    u1 num_parameters;
//        {
//            u2 num_annotations;
//        annotation annotations[num_annotations];
//    } parameter_annotations[num_parameters];
};

struct RuntimeInvisibleParameterAnnotations_attribute {
    u1 num_parameters;
//            {
//                u2 num_annotations;
//        annotation annotations[num_annotations];
//    } parameter_annotations[num_parameters];
};

struct RuntimeVisibleTypeAnnotations_attribute {
    u2 num_annotations;
//    type_annotation annotations[num_annotations];
};

#if 0
struct type_annotation {
    u1 target_type;
    union {
        type_parameter_target;
        supertype_target;
        type_parameter_bound_target;
        empty_target;
        formal_parameter_target;
        throws_target;
        localvar_target;
        catch_target;
        offset_target;
        type_argument_target;
    } target_info;
    type_path target_path;
    u2 type_index;
    u2 num_element_value_pairs;
//    {
//        u2 element_name_index;
//        element_value value;
//    } element_value_pairs[num_element_value_pairs];
};

struct type_parameter_target {
    u1 type_parameter_index;
};

struct supertype_target {
    u2 supertype_index;
};

struct type_parameter_bound_target {
    u1 type_parameter_index;
    u1 bound_index;
};

struct empty_target {
};

struct formal_parameter_target {
    u1 formal_parameter_index;
};

struct throws_target {
    u2 throws_type_index;
};

struct localvar_target {
    u2 table_length;
//    {
//        u2 start_pc;
//        u2 length;
//        u2 index;
//    } table[table_length];
};

struct catch_target {
    u2 exception_table_index;
};

struct offset_target {
    u2 offset;
};

struct type_argument_target {
    u2 offset;
    u1 type_argument_index;
};

struct type_path {
    u1 path_length;
//    {
//        u1 type_path_kind;
//        u1 type_argument_index;
//    } path[path_length];
};
#endif

struct RuntimeInvisibleTypeAnnotations_attribute {
    u2 num_annotations;
//    type_annotation annotations[num_annotations];
};

struct AnnotationDefault_attribute {
    element_value default_value;
};

struct BootstrapMethod {
    CONSTANT_MethodHandle_info *bootstrap_method_ref;
    std::vector<u2> bootstrap_arguments;
};

struct BootstrapMethods_attribute {
    std::vector<BootstrapMethod> bootstrap_methods;
};

struct MethodParameter {
    u2 name_index;
    u2 access_flags;
};

struct MethodParameters_attribute {
    std::vector<MethodParameter> parameters;
};

struct Module_attribute {

    u2 module_name_index;
    u2 module_flags;
    u2 module_version_index;

    u2 requires_count;
//    {
//        u2 requires_index;
//        u2 requires_flags;
//        u2 requires_version_index;
//    } requires[requires_count];

    u2 exports_count;
//    {
//        u2 exports_index;
//        u2 exports_flags;
//        u2 exports_to_count;
//        u2 exports_to_index[exports_to_count];
//    } exports[exports_count];

    u2 opens_count;
//    {
//        u2 opens_index;
//        u2 opens_flags;
//        u2 opens_to_count;
//        u2 opens_to_index[opens_to_count];
//    } opens[opens_count];

    u2 uses_count;
//    u2 uses_index[uses_count];

    u2 provides_count;
//    {
//        u2 provides_index;
//        u2 provides_with_count;
//        u2 provides_with_index[provides_with_count];
//    } provides[provides_count];
};

struct ModulePackages_attribute {
    u2 package_count;
//    u2 package_index[package_count];
};

struct ModuleMainClass_attribute {
    u2 main_class_index;
};

struct NestHost_attribute {
    CONSTANT_Class_info *host_class_index;
};

struct NestMembers_attribute {
    std::vector<CONSTANT_Class_info *> classes;
};

struct Record_attribute {
    u2 components_count;
//    record_component_info components[components_count];
};

struct record_component_info {
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
//    attribute_info attributes[attributes_count];
};

struct attribute_info {
    // TODO: Do we even need to store these two fields?
    CONSTANT_Utf8_info *attribute_name_index;
    u4 attribute_length;
    std::variant<
            ConstantValue_attribute,
            Code_attribute,
            Exceptions_attribute,
            Signature_attribute,
            SourceFile_attribute,
            SourceDebugExtension_attribute,
            LineNumberTable_attribute,
            Deprecated_attribute,
            BootstrapMethods_attribute,
            MethodParameters_attribute,
            ModuleMainClass_attribute,
            NestHost_attribute,
            NestMembers_attribute
    > variant;
};

struct ConstantPool {
    std::vector<cp_info> table;

    template<class T>
    inline T &get(u2 index) {
        return std::get<T>(table[index].variant);
    }
};

enum class ClassFileAccessFlags : u2 {
    ACC_PUBLIC = 0x0001,
    ACC_FINAL = 0x0010,
    ACC_SUPER = 0x0020,
    ACC_INTERFACE = 0x0200,
    ACC_ABSTRACT = 0x0400,
    ACC_SYNTHETIC = 0x1000,
    ACC_ANNOTATION = 0x2000,
    ACC_ENUM = 0x4000,
    ACC_MODULE = 0x8000,
};

struct ClassFile {
    Object header;
    Value padding_for_java_instance_fields_a[6]; // TODO
    Value field_component_type;
    Value padding_for_java_instance_fields_b[13]; // TODO

    u4 magic;
    u2 minor_version;
    u2 major_version;
    ConstantPool constant_pool;
    u2 access_flags;
    CONSTANT_Class_info *this_class;
    // nullptr for class Object
    CONSTANT_Class_info *super_class_ref;
    // nullptr for class Object and before resolution
    ClassFile *super_class;
    std::vector<CONSTANT_Class_info *> interfaces;
    std::vector<field_info> fields;
    std::vector<method_info> methods;
    std::vector<attribute_info> attributes;

    int clinit_index = -1;

    size_t declared_instance_field_count;
    size_t total_instance_field_count;
    std::vector<Value> static_field_values;

    ClassFile *array_element_type = nullptr; // set iff this is an array of references

    bool resolved = false;

    std::mutex initialization_lock{};
    std::condition_variable initialization_condition_variable{};
    bool is_initialized = false;
    struct Thread *initializing_thread = nullptr;
    bool is_erroneous_state = false;

    std::string_view package_name;

    // Computes whether `this` is a subclass of `other` (regarding both `extends` and `implements`).
    // Note that `x.is_subclass_of(x) == false`
    bool is_subclass_of(ClassFile *other) const {
        if (this->super_class == other) {
            return true;
        }
        if (other->is_interface()) {
            for (auto &i: interfaces) {
                if (i->clazz == other) {
                    return true;
                }
            }
        }
        if (this->super_class != nullptr && this->super_class->is_subclass_of(other)) {
            return true;
        }
        if (other->is_interface()) {
            for (auto &i: interfaces) {
                if (i->clazz->is_subclass_of(other)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool is_instance_of(ClassFile *parent) const {
        if (this == parent || is_subclass_of(parent)) {
            return true;
        }
        if (array_element_type == nullptr || parent->array_element_type == nullptr) {
            return false;
        }
        return array_element_type->is_instance_of(parent->array_element_type);
    }

    [[nodiscard]] inline bool is_interface() const {
        return (access_flags & static_cast<u2>(ClassFileAccessFlags::ACC_INTERFACE)) != 0;
    }

    [[nodiscard]] inline std::string const &name() const { return this_class->name->value; }

    [[nodiscard]] bool is_array() const {
        return !name().empty() && name()[0] == '[';
    }

    [[nodiscard]] std::string as_array_element() const {
        std::string const &n = name();
        if (!n.empty() && n[0] == '[') {
            return "[" + n;
        } else {
            return "[L" + n + ";";
        }
    }
};


#endif //SCHOKOVM_CLASSFILE_HPP

