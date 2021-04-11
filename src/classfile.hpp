#ifndef SCHOKOVM_CLASSFILE_HPP
#define SCHOKOVM_CLASSFILE_HPP

#include <vector>

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

struct ConstantPool {
    std::vector<cp_info> table;
    std::vector<std::string> utf8_strings;
};

struct ClassFile {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    ConstantPool constant_pool;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    std::vector<u2> interfaces;
    std::vector<field_info> fields;
    std::vector<method_info> methods;
    std::vector<attribute_info> attributes;
};

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

struct CONSTANT_Class_info {
    u2 name_index;
};

struct CONSTANT_Fieldref_info {
    u2 class_index;
    u2 name_and_type_index;
};

struct CONSTANT_Methodref_info {
    u2 class_index;
    u2 name_and_type_index;
};

struct CONSTANT_InterfaceMethodref_info {
    u2 class_index;
    u2 name_and_type_index;
};

struct CONSTANT_String_info {
    u2 string_index;
};

struct CONSTANT_Integer_info {
    u4 bytes;
};

struct CONSTANT_Float_info {
//    u4 bytes;
    float value;
};

struct CONSTANT_Long_info {
//    u4 high_bytes;
//    u4 low_bytes;
    u8 value;
};

struct CONSTANT_Double_info {
//    u4 high_bytes;
//    u4 low_bytes;
    double value;
};

struct CONSTANT_NameAndType_info {
    u2 name_index;
    u2 descriptor_index;
};

struct CONSTANT_Utf8_info {
    // This is an index into the utf8_strings vector of ConstantPool so that we don't have to store a vector
    // inside the cp_info union.
    size_t index;
//    u2 length;
//    u1 bytes[length];
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
    CpTag tag;
    union {
        CONSTANT_Class_info class_info;
        CONSTANT_Fieldref_info fieldref_info;
        CONSTANT_Methodref_info methodref_info;
        CONSTANT_InterfaceMethodref_info interface_methodref_info;
        CONSTANT_String_info string_info;
        CONSTANT_Integer_info integer_info;
        CONSTANT_Float_info float_info;
        CONSTANT_Long_info long_info;
        CONSTANT_Double_info double_info;
        CONSTANT_NameAndType_info name_and_type_info;
        CONSTANT_Utf8_info utf_8_info;
        CONSTANT_MethodHandle_info method_handle_info;
        CONSTANT_MethodType_info method_type_info;
        CONSTANT_Dynamic_info dynamic_info;
        CONSTANT_InvokeDynamic_info invoke_dynamic_info;
        CONSTANT_Module_info module_info;
        CONSTANT_Package_info package_info;
    } info;
};

enum class FieldInfoAccessFlags {
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
    u2 name_index;
    u2 descriptor_index;
    std::vector<attribute_info> attributes;
};

enum class MethodInfoAccessFlags {
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
    u2 name_index;
    u2 descriptor_index;
    std::vector<attribute_info> attributes;
};

struct attribute_info {
    u2 attribute_name_index;
    u4 attribute_length;
//    u1 info[attribute_length];
};

struct ConstantValue_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 constantvalue_index;
};

struct Code_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
//    u1 code[code_length];
    u2 exception_table_length;
//    {
//        u2 start_pc;
//        u2 end_pc;
//        u2 handler_pc;
//        u2 catch_type;
//    } exception_table[exception_table_length];
    u2 attributes_count;
//    attribute_info attributes[attributes_count];
};

struct StackMapTable_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 number_of_entries;
//    stack_map_frame entries[number_of_entries];
};

#if 0
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
#endif

#if 0
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
    u2 attribute_name_index;
    u4 attribute_length;
    u2 number_of_exceptions;
//    u2 exception_index_table[number_of_exceptions];
};

struct InnerClasses_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 number_of_classes;
//    {
//        u2 inner_class_info_index;
//        u2 outer_class_info_index;
//        u2 inner_name_index;
//        u2 inner_class_access_flags;
//    } classes[number_of_classes];
};

struct EnclosingMethod_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 class_index;
    u2 method_index;
};

struct Synthetic_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
};

struct Signature_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 signature_index;
};

struct SourceFile_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 sourcefile_index;
};

struct SourceDebugExtension_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
//    u1 debug_extension[attribute_length];
};

struct LineNumberTable_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 line_number_table_length;
//        {
//            u2 start_pc;
//            u2 line_number;
//    } line_number_table[line_number_table_length];
};

struct LocalVariableTable_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
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
    u2 attribute_name_index;
    u4 attribute_length;
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
    u2 attribute_name_index;
    u4 attribute_length;
};

struct RuntimeVisibleAnnotations_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
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
    u2 attribute_name_index;
    u4 attribute_length;
    u2 num_annotations;
//    annotation annotations[num_annotations];
};

struct RuntimeVisibleParameterAnnotations_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 num_parameters;
//        {
//            u2 num_annotations;
//        annotation annotations[num_annotations];
//    } parameter_annotations[num_parameters];
};

struct RuntimeInvisibleParameterAnnotations_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 num_parameters;
//            {
//                u2 num_annotations;
//        annotation annotations[num_annotations];
//    } parameter_annotations[num_parameters];
};

struct RuntimeVisibleTypeAnnotations_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
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
    u2 attribute_name_index;
    u4 attribute_length;
    u2 num_annotations;
//    type_annotation annotations[num_annotations];
};

struct AnnotationDefault_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    element_value default_value;
};

struct BootstrapMethods_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 num_bootstrap_methods;
//    {
//        u2 bootstrap_method_ref;
//        u2 num_bootstrap_arguments;
//        u2 bootstrap_arguments[num_bootstrap_arguments];
//    } bootstrap_methods[num_bootstrap_methods];
};

struct MethodParameters_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 parameters_count;
//    {
//        u2 name_index;
//        u2 access_flags;
//    } parameters[parameters_count];
};

struct Module_attribute {
    u2 attribute_name_index;
    u4 attribute_length;

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
    u2 attribute_name_index;
    u4 attribute_length;
    u2 package_count;
//    u2 package_index[package_count];
};

struct ModuleMainClass_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 main_class_index;
};

struct NestHost_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 host_class_index;
};

struct NestMembers_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 number_of_classes;
//    u2 classes[number_of_classes];
};

struct Record_attribute {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 components_count;
//    record_component_info components[components_count];
};

struct record_component_info {
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
//    attribute_info attributes[attributes_count];
};


#endif //SCHOKOVM_CLASSFILE_HPP

